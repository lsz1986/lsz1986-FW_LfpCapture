#ifndef __TIMER_SAMPLE_DATA_H__
#define __TIMER_SAMPLE_DATA_H__

#include <stdint.h>

#define RAW_ADC_BUFF_SIZE				6

typedef enum{
	SAMPLE_CHANNEL_HEART_RATE,
	SAMPLE_CHANNEL_BATTERY_CAPACITY,
	SAMPLE_CHANNEL_SIZE
}SAMPLE_CHANNEL_IDX;

#define SAMPLE_SUM_PER_FRAME (RAW_ADC_BUFF_SIZE/SAMPLE_CHANNEL_SIZE)

extern volatile uint16_t g_vBatteryCapacity[SAMPLE_SUM_PER_FRAME];
extern volatile uint16_t g_vHeartRate[SAMPLE_SUM_PER_FRAME];

void task_sample_enable(void);




#endif
