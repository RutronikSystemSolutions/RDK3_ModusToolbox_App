/*
 * um980_app.c
 *
 *  Created on: 19 Oct 2023
 *      Author: jorda
 */


#include "um980_app.h"

#include <stddef.h>
#include <stdbool.h>

#include "nmea_packet.h"
#include "gga_packet.h"
#include "packet_printer.h"
#include "command_ack_packet.h"
#include <string.h>
#include <stdio.h>

static um980_app_uart_readable_func_t uart_readable_func = NULL;
static um980_app_uart_read_func_t uart_read_func = NULL;
static um980_app_uart_write_func_t uart_write_func = NULL;
static um980_app_get_uticks get_uticks_func = NULL;
static um980_app_on_nmea_packet nmea_listener = NULL;

#define PACKET_BUFFER_SIZE 512
static uint8_t packet_buffer[PACKET_BUFFER_SIZE] = {0};

#define MAX_CMD_LEN 128

static const uint32_t timeout_us = 100000; // 100ms

/**
 * @brief Flush the RX receiving buffer
 */
static void read_all()
{
	for(;;)
	{
		uint32_t readable = uart_readable_func();
		if (readable == 0)
		{
			break;
		}

		uint16_t toread = (readable > PACKET_BUFFER_SIZE) ? PACKET_BUFFER_SIZE : (uint16_t)readable;
		uart_read_func(packet_buffer, toread);
	}
}

void um980_app_init_hal(um980_app_uart_readable_func_t uart_readable,
		um980_app_uart_read_func_t uart_read,
		um980_app_uart_write_func_t uart_write,
		um980_app_get_uticks get_uticks)
{
	uart_readable_func = uart_readable;
	uart_read_func = uart_read;
	uart_write_func = uart_write;
	get_uticks_func = get_uticks;

	packet_handler_init(uart_readable, uart_read, uart_write);

	read_all();
}

void um980_app_set_nmea_listener(um980_app_on_nmea_packet listener)
{
	nmea_listener = listener;
}

void um980_app_reset()
{
	packet_handler_reset();
}

/**
 * @brief Send a command and wait until feedback (or timeout)
 *
 * @retval 0 Success
 * @retval != 0 Error
 */
static int send_command_and_wait(char* cmd)
{
	char buffer[MAX_CMD_LEN];
	sprintf(buffer, "%s\r\n", cmd);

	uint16_t msg_size = strlen(buffer);
	if (uart_write_func((uint8_t*)buffer, msg_size) != msg_size)
	{
		return -1;
	}

	uint32_t start_time = get_uticks_func();
	for(;;)
	{
		int retval = packet_handler_read_packet(packet_buffer, PACKET_BUFFER_SIZE);
		if (retval < 0) return -2;

		// Packet is available, retval contains its length
		if (retval > 0)
		{
			if (packet_buffer[0] == '$')
			{
				nmea_packet_type_t packet_type = nmea_packet_get_type(packet_buffer, (uint16_t)retval);
				if (packet_type == PACKET_TYPE_COMMAND_ACK)
				{
					command_ack_packet_t packet;
					command_ack_packet_extract_data(packet_buffer, (uint16_t)retval, &packet);

					retval = command_ack_packet_check_command_status(cmd, &packet);
					if (retval == 0) return 0;
				}
			}
		}

		// Check for timeout
		uint32_t current_time = get_uticks_func();
		if (current_time < start_time) return -3;
		if ((current_time - start_time) > timeout_us) return -4;
	}

	return -5;
}

static void wait_us(uint32_t us)
{
	uint32_t start_timestamp = get_uticks_func();
	for(;;)
	{
		uint32_t timestamp = get_uticks_func();
		if (timestamp < start_timestamp) return;

		uint32_t diff = timestamp - start_timestamp;
		if (diff > us) return;
	}
}

int um980_app_init()
{
	int counter = 0;

	// Stop message generation (correction and position)
	// Wait until successful - First call might fail because of strange startup behavior
	for(;;)
	{
		int retval = um980_app_unlog();
		if (retval != 0)
		{
			wait_us(500000); // wait for 0.5 seconds
			um980_app_reset();
		}
		else return 0;

		counter++;
		if (counter > 1) return -1;
	}

	return -2;
}

int um980_app_unlog()
{
	return send_command_and_wait("unlog");
}

int um980_app_set_mode_base()
{
	return send_command_and_wait("mode base time 60");
}

int um980_app_set_mode_rover()
{
	return send_command_and_wait("mode rover");
}

int um980_app_start_correction_generation(uint16_t rtcm_number, uint16_t period)
{
	char cmd[32] = {0};
	sprintf(cmd, "RTCM%d %d\r\n", rtcm_number, period);

	return send_command_and_wait(cmd);
}

int um980_app_start_gga_generation(um980_frequency_hz_t frequency)
{
	switch(frequency)
	{
		case FREQUENCY_1HZ: return send_command_and_wait("gpgga 1");
		case FREQUENCY_2HZ: return send_command_and_wait("gpgga 0.5");
		case FREQUENCY_5HZ: return send_command_and_wait("gpgga 0.2");
		case FREQUENCY_10HZ: return send_command_and_wait("gpgga 0.1");
	}
	return -1;
}

int um980_app_do()
{
	int retval = packet_handler_read_packet(packet_buffer, PACKET_BUFFER_SIZE);
	if (retval < 0)
	{
		return -1;
	}

	if (retval == 0) return 0;

	uint16_t packet_type = packet_handler_get_packet_type(packet_buffer);
	switch(packet_type)
	{
		case PACKET_HANDLER_NMEA_PACKET:
		{
			if (nmea_listener != NULL)
			{
				nmea_listener(packet_buffer, (uint16_t) retval);
			}
			break;
		}
		case PACKET_HANDLER_RTCM_PACKET:
		{
			packet_printer_print_rtcm(packet_buffer, (uint16_t) retval);
			break;
		}
		case PACKET_HANDLER_UNKNOWN_PACKET:
		{
			return -2;
		}
	}

	return 0;
}
