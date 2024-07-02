#ifndef __TIMER_SAMPLE_DATA_H__
#define __TIMER_SAMPLE_DATA_H__

#include <stdint.h>

void task_sample_enable(void);

extern uint16_t g_vBatteryCapacity;
extern uint16_t g_vHeartRate;

#endif
