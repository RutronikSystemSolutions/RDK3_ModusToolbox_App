#include "gesture_control.h"

#include "pca9554_app.h"
#include "arm_math.h"
#include "cyhal.h"

#include <string.h>
#include <stdio.h>

/*Moving Average Function Prototype*/
static float MovingAvg(int *ptrArrNumbers, long *ptrSum, int pos, int len, int nextNum);

/*Moving average global variables*/
static int sens_buff_1[MOV_AVERAGE] = {0};
static int buff_pos_1 = 0;
static long sum_1 = 0;

static int sens_buff_2[MOV_AVERAGE] = {0};
static int buff_pos_2 = 0;
static long sum_2 = 0;

static int sens_buff_3[MOV_AVERAGE] = {0};
static int buff_pos_3 = 0;
static long sum_3 = 0;

/*Gesture Frames Global Storage*/
static float sensor1[GESTURE_FRAME_SIZE] = {0};
static float sensor2[GESTURE_FRAME_SIZE] = {0};
static float sensor3[GESTURE_FRAME_SIZE] = {0};

static float crosscorr[GESTURE_FRAME_SIZE * 2 - 1] = {0};

static int buff_pos = 0;

static float std_left = 0;
static float std_up = 0;
static float std_right = 0;

static gesture_t current_gesture = GESTURE_NONE;

static uint32_t gesture_hold_time_ms = 0;

/*PCA9554 availability*/
static uint8_t pca9554_available = 0;


void gesture_control_reset(void)
{
	cy_rslt_t result;

	/*Configure the PCA9554 GPIO Expander*/
	result = pca9554_configure();
	if (result == CY_RSLT_SUCCESS)
	{
		pca9554_available = 1;

		/*Initial LED Direction*/
		pca9554_dir_set(PCA9554_DIR_LOWER);
	}
	else
	{
		pca9554_available = 0;
		printf("PCA9554 not available. Gesture LEDs disabled.\r\n");
	}

	memset(sens_buff_1, 0, sizeof(sens_buff_1));
	memset(sens_buff_2, 0, sizeof(sens_buff_2));
	memset(sens_buff_3, 0, sizeof(sens_buff_3));

	memset(sensor1, 0, sizeof(sensor1));
	memset(sensor2, 0, sizeof(sensor2));
	memset(sensor3, 0, sizeof(sensor3));

	memset(crosscorr, 0, sizeof(crosscorr));

	buff_pos_1 = 0;
	buff_pos_2 = 0;
	buff_pos_3 = 0;

	sum_1 = 0;
	sum_2 = 0;
	sum_3 = 0;

	buff_pos = 0;

	std_left = 0;
	std_up = 0;
	std_right = 0;

	current_gesture = GESTURE_NONE;
	gesture_hold_time_ms = 0;

	/*Indicate the progress*/
	if (pca9554_available)
	{
		pca9554_dir_set(PCA9554_DIR_UPPER);
	}

	/*Initial gesture state*/
	current_gesture = GESTURE_NONE;

	if (pca9554_available)
	{
		pca9554_dir_set(PCA9554_DIR_NONE);
	}
}


uint8_t gesture_control_process_sample(
		uint16_t ps1,
		uint16_t ps2,
		uint16_t ps3)
{
	float sens_av_1 = 0;
	float sens_av_2 = 0;
	float sens_av_3 = 0;

	float max_value = 0;
	uint32_t value_index = 0;

	int left_right_delay = 0;
	int left_up_delay = 0;
	int right_up_delay = 0;

	uint8_t detected_gesture = GESTURE_NONE;

	/* Check if the Gesture is still valid */
	if (gesture_hold_time_ms > 0)
	{
		if (gesture_hold_time_ms > GESTURE_SAMPLE_PERIOD_MS)
		{
			gesture_hold_time_ms -= GESTURE_SAMPLE_PERIOD_MS;
		}
		else
		{
			gesture_hold_time_ms = 0;
			current_gesture = GESTURE_NONE;

			if (pca9554_available)
			{
				pca9554_dir_set(PCA9554_DIR_NONE);
			}
		}
	}

	/*Do the moving average math for the sensor 1*/
	sens_av_1 = MovingAvg(
			sens_buff_1,
			&sum_1,
			buff_pos_1,
			MOV_AVERAGE,
			(int)ps1);

	buff_pos_1++;

	if(buff_pos_1 >= MOV_AVERAGE)
	{
		buff_pos_1 = 0;
	}

	/*Do the moving average math for the sensor 2*/
	sens_av_2 = MovingAvg(
			sens_buff_2,
			&sum_2,
			buff_pos_2,
			MOV_AVERAGE,
			(int)ps2);

	buff_pos_2++;

	if(buff_pos_2 >= MOV_AVERAGE)
	{
		buff_pos_2 = 0;
	}

	/*Do the moving average math for the sensor 3*/
	sens_av_3 = MovingAvg(
			sens_buff_3,
			&sum_3,
			buff_pos_3,
			MOV_AVERAGE,
			(int)ps3);

	buff_pos_3++;

	if(buff_pos_3 >= MOV_AVERAGE)
	{
		buff_pos_3 = 0;
	}

	/*Put the data into the gesture frame storage*/
	if(buff_pos < GESTURE_FRAME_SIZE)
	{
		sensor1[buff_pos] = sens_av_1;
		sensor2[buff_pos] = sens_av_2;
		sensor3[buff_pos] = sens_av_3;

		buff_pos++;
	}

	/* Frame buffer is full, do the data processing. */
	if(buff_pos == GESTURE_FRAME_SIZE)
	{
		buff_pos = 0;

		/*Calculate the standard deviation of all sensor frames*/
		arm_std_f32(
				sensor1,
				GESTURE_FRAME_SIZE,
				&std_left);

		arm_std_f32(
				sensor2,
				GESTURE_FRAME_SIZE,
				&std_right);

		arm_std_f32(
				sensor3,
				GESTURE_FRAME_SIZE,
				&std_up);

		#if STD_DEBUG_EN

		/*Print standard deviation data*/
		printf("Left: %.2f\r\n ", std_left);
		printf("Up: %.2f\r\n ", std_up);
		printf("Right: %.2f\r\n\r\n", std_right);

		#endif

		/*If motion detected, analyze the frame*/
		if((std_left >= STD_THRESHOLD) ||
		   (std_right >= STD_THRESHOLD) ||
		   (std_up >= STD_THRESHOLD))
		{
			/*Left Right Cross-Correlation*/
			arm_correlate_f32(
					sensor1,
					GESTURE_FRAME_SIZE,
					sensor2,
					GESTURE_FRAME_SIZE,
					crosscorr);

			arm_max_f32(
					crosscorr,
					GESTURE_FRAME_SIZE * 2 - 1,
					&max_value,
					&value_index);

			left_right_delay =
					value_index -
					(GESTURE_FRAME_SIZE - 1);

			if(left_right_delay < -LFT_RHT_DELAY_TH)
			{
				current_gesture = GESTURE_RIGHT;
				detected_gesture = GESTURE_RIGHT;
			
				gesture_hold_time_ms = GESTURE_HOLD_MS;
			
				if (pca9554_available)
				{
					pca9554_dir_set(
							PCA9554_DIR_RIGHT);
				}
			
				#if LFT_RHT_DEBUG_EN
			
				printf(
						"Right %d \r\n",
						left_right_delay);
			
				#endif
			}
			else if(left_right_delay > LFT_RHT_DELAY_TH)
			{
				current_gesture = GESTURE_LEFT;
				detected_gesture = GESTURE_LEFT;
			
				gesture_hold_time_ms = GESTURE_HOLD_MS;
			
				if (pca9554_available)
				{
					pca9554_dir_set(
							PCA9554_DIR_LEFT);
				}
			
				#if LFT_RHT_DEBUG_EN
			
				printf(
						"Left %d \r\n",
						left_right_delay);
			
				#endif
			}
			
			/*Left Up Cross-Correlation*/
			else
			{
				arm_correlate_f32(
						sensor1,
						GESTURE_FRAME_SIZE,
						sensor3,
						GESTURE_FRAME_SIZE,
						crosscorr);

				arm_max_f32(
						crosscorr,
						GESTURE_FRAME_SIZE * 2 - 1,
						&max_value,
						&value_index);

				left_up_delay =
						value_index -
						(GESTURE_FRAME_SIZE - 1);

				if(left_up_delay > LFT_UP_DELAY_TH)
				{
					current_gesture = GESTURE_DOWN;
					detected_gesture = GESTURE_DOWN;

					gesture_hold_time_ms = GESTURE_HOLD_MS;

					if (pca9554_available)
					{
						pca9554_dir_set(
								PCA9554_DIR_DOWN);
					}

					#if LFT_UP_DEBUG_EN

					printf(
							"Down %d \r\n",
							left_up_delay);

					#endif
				}
				else if(left_up_delay < LFT_UP_DELAY_TH)
				{
					current_gesture = GESTURE_UP;
					detected_gesture = GESTURE_UP;

					gesture_hold_time_ms = GESTURE_HOLD_MS;

					if (pca9554_available)
					{
						pca9554_dir_set(
								PCA9554_DIR_UP);
					}

					#if LFT_UP_DEBUG_EN

					printf(
							"Up %d \r\n",
							left_up_delay);

					#endif
				}

				/*Right Up Cross-Correlation*/
				else
				{
					arm_correlate_f32(
							sensor2,
							GESTURE_FRAME_SIZE,
							sensor3,
							GESTURE_FRAME_SIZE,
							crosscorr);

					arm_max_f32(
							crosscorr,
							GESTURE_FRAME_SIZE * 2 - 1,
							&max_value,
							&value_index);

					right_up_delay =
							value_index -
							(GESTURE_FRAME_SIZE - 1);

					if(right_up_delay > RHT_UP_DELAY_TH)
					{
						current_gesture = GESTURE_DOWN;
						detected_gesture = GESTURE_DOWN;

						gesture_hold_time_ms =
								GESTURE_HOLD_MS;

						if (pca9554_available)
						{
							pca9554_dir_set(
									PCA9554_DIR_DOWN);
						}

						#if RHT_UP_DEBUG_EN

						printf(
								"Down %d \r\n",
								right_up_delay);

						#endif
					}
					else if(right_up_delay < RHT_UP_DELAY_TH)
					{
						current_gesture = GESTURE_UP;
						detected_gesture = GESTURE_UP;

						gesture_hold_time_ms =
								GESTURE_HOLD_MS;

						if (pca9554_available)
						{
							pca9554_dir_set(
									PCA9554_DIR_UP);
						}

						#if RHT_UP_DEBUG_EN

						printf(
								"Up %d \r\n",
								right_up_delay);

						#endif
					}
					else
					{
						current_gesture = GESTURE_NONE;

						if (pca9554_available)
						{
							pca9554_dir_set(
									PCA9554_DIR_NONE);
						}
					}
				}
			}
		}
	}

	/*
	 * Only return a newly detected gesture.
	 * This keeps the behavior expected by rutronik_application.c.
	 */
	return detected_gesture;
}


uint8_t gesture_control_get_last_gesture(void)
{
	return (uint8_t)current_gesture;
}


const char* gesture_control_get_gesture_name(uint8_t gesture)
{
	switch(gesture)
	{
		case GESTURE_RIGHT:
			return "RIGHT";

		case GESTURE_LEFT:
			return "LEFT";

		case GESTURE_UP:
			return "UP";

		case GESTURE_DOWN:
			return "DOWN";

		case GESTURE_NONE:
		default:
			return "NONE";
	}
}


static float MovingAvg(
		int *ptrArrNumbers,
		long *ptrSum,
		int pos,
		int len,
		int nextNum)
{
	/*Subtract the oldest number from the previous sum, add the new number*/
	*ptrSum =
			*ptrSum -
			ptrArrNumbers[pos] +
			nextNum;

	/*Assign the nextNum to the position in the array*/
	ptrArrNumbers[pos] = nextNum;

	/*Return the average*/
	return (float)(*ptrSum) / (float)len;
}