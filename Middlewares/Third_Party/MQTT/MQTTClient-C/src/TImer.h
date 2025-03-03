#ifndef __TIMER_H__
#define __TIMER_H__

#include "stm32l4xx_hal.h"  // for HAL_GetTick()

/* Timer structure definition */
typedef struct Timer {
    uint32_t start_tick;
    uint32_t timeout_tick;
} Timer;

/* Function prototypes expected by the MQTT client */
void TimerInit(Timer* timer);
char TimerIsExpired(Timer* timer);
void TimerCountdownMS(Timer* timer, unsigned int ms);
void TimerCountdown(Timer* timer, unsigned int seconds);
int TimerLeftMS(Timer* timer);

#endif // __TIMER_H__
