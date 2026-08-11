#ifndef GESTURE_CONTROL_H_
#define GESTURE_CONTROL_H_

#include <stdint.h>

/*Gesture Samples Frame Size*/
#define GESTURE_FRAME_SIZE	20

/*Gesture Hold Time in milliseconds*/
#define GESTURE_HOLD_MS		400

/*Gesture Sample Period in milliseconds*/
#define GESTURE_SAMPLE_PERIOD_MS	10

/*The Moving Average of the Gesture Samples*/
#define MOV_AVERAGE			3

/*Gesture Debugging Information Enable/Disable*/
#define STD_DEBUG_EN		0
#define LFT_RHT_DEBUG_EN	0
#define LFT_UP_DEBUG_EN		0
#define RHT_UP_DEBUG_EN		0

/*Standard Deviation & Signal Delay Thresholds*/
#define STD_THRESHOLD		50
#define LFT_RHT_DELAY_TH	0
#define LFT_UP_DELAY_TH		0
#define RHT_UP_DELAY_TH		0

typedef enum
{
	GESTURE_NONE = 0,
	GESTURE_RIGHT,
	GESTURE_LEFT,
	GESTURE_UP,
	GESTURE_DOWN,
	GESTURE_INIT = 0xFF,

} gesture_t;

void gesture_control_reset(void);

uint8_t gesture_control_process_sample(
		uint16_t ps1,
		uint16_t ps2,
		uint16_t ps3);

uint8_t gesture_control_get_last_gesture(void);

const char* gesture_control_get_gesture_name(uint8_t gesture);

#endif /* GESTURE_CONTROL_H_ */