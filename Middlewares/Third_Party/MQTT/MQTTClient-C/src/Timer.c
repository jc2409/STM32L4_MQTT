#include "Timer.h"
#include "stm32l4xx_hal.h"  // Ensure HAL_GetTick() is declared

/**
 * @brief Initializes the timer structure.
 * @param timer Pointer to the Timer structure.
 */
void TimerInit(Timer* timer) {
    if (timer != NULL) {
        timer->start_tick = HAL_GetTick();
        timer->timeout_tick = 0; // 0 means no timeout has been set yet.
    }
}

/**
 * @brief Starts a countdown in milliseconds.
 * @param timer Pointer to the Timer structure.
 * @param ms Countdown duration in milliseconds.
 */
void TimerCountdownMS(Timer* timer, unsigned int ms) {
    if (timer != NULL) {
        timer->start_tick = HAL_GetTick();
        timer->timeout_tick = timer->start_tick + ms;
        /* Note: For long durations, you might consider wrap-around handling */
    }
}

/**
 * @brief Starts a countdown in seconds.
 * @param timer Pointer to the Timer structure.
 * @param seconds Countdown duration in seconds.
 */
void TimerCountdown(Timer* timer, unsigned int seconds) {
    TimerCountdownMS(timer, seconds * 1000);
}

/**
 * @brief Checks if the timer has expired.
 * @param timer Pointer to the Timer structure.
 * @return 1 if expired or timer is NULL; 0 if not expired.
 */
char TimerIsExpired(Timer* timer) {
    if (timer != NULL) {
        return (HAL_GetTick() >= timer->timeout_tick) ? 1 : 0;
    }
    return 1; // If no timer provided, treat as expired.
}

/**
 * @brief Returns the remaining time in milliseconds.
 * @param timer Pointer to the Timer structure.
 * @return Remaining milliseconds before expiration, or 0 if expired or timer is NULL.
 */
int TimerLeftMS(Timer* timer) {
    if (timer != NULL) {
        uint32_t now = HAL_GetTick();
        if (now >= timer->timeout_tick)
            return 0;
        else
            return (int)(timer->timeout_tick - now);
    }
    return 0;
}

