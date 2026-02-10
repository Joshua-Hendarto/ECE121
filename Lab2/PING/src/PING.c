#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "PING.h"
#include "BOARD.h"
#include "timers.h"
#include "Pwm.h"

#define PING_TEST
#define THEREMIN_TEST

// PING sensor state machine states
typedef enum {
    PING_Sent,      // Wait for echo and measure
    PING_Waited_60ms      // Reset and Send 10µs trigger pulse
} PING_State;

// Global variables
static PING_State ping_state = PING_Sent; // Initial state
static volatile unsigned int echo_start = 0;  // Rising edge time (us)
static volatile unsigned int echo_end = 0;    // Falling edge time (us)
static volatile unsigned int echo_time = 0;    // Pulse width (us)
static volatile unsigned int distance = 0;         // Calculated distance (mm)

TIM_HandleTypeDef htim3;
/**
 * @function    PING_Init(void)
 * @brief       Sets up both the timer extrenal interrupt peripherals along with their
 *              Also configures PWM_5 as a GPIO output for the trigger pin. 
 * @return      SUCCESS or ERROR
 */
char PING_Init(void) {
    // init other libraries
    BOARD_Init();
    Timers_Init();

    // this block initializes the GPIO output pin (PB8, PWM_5 on shield)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // this block inits the timer generated interrupt
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 83; // divide by 1 prescaler (84-1) = 1 Mhz tick
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 999;  // initial period of 1ms
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
    {
        return ERROR;
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
    {
        return ERROR;
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
    {
        return ERROR;
    }
    HAL_TIM_Base_Start_IT(&htim3); // start interrupt

    // this block inits the external pin-generted interrupt on any change of pin PB5 (ENC_B)
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

    return SUCCESS;
}

/**
 * @function    PING_GetDistance(void)
 * @brief       Returns the calculated distance in mm using the sensor model determined
 *              experimentally. 
 *              No I/O should be done in this function
 * @return      distance in mm
 */
unsigned int PING_GetDistance(void){
    return distance;
}

/**
 * @function    PING_GetTimeofFlight(void)
 * @brief       Returns the raw microsecond duration of the echo from the sensor.
 *              NO I/O should be done in this function.
 * @return      time of flight in uSec
 */
unsigned int PING_GetTimeofFlight(void){
    return echo_time;
}


// TIM3 ISR
void TIM3_IRQHandler(void) {
    if (__HAL_TIM_GET_IT_SOURCE(&htim3, TIM_IT_UPDATE) != RESET) {
        __HAL_TIM_CLEAR_IT(&htim3, TIM_IT_UPDATE); // clear interrupt flag
        
        switch (ping_state) {
        case PING_Sent:
            // End the 10µs trigger pulse, switch to waiting state
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
            htim3.Instance->ARR = 59999; // 60ms delay
            htim3.Instance->CNT = 0;
            ping_state = PING_Waited_60ms;
            break;
        case PING_Waited_60ms:
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);    
            htim3.Instance->ARR = 9; // 10us to send trigger pulse
            htim3.Instance->CNT = 0;
            ping_state = PING_Sent;
            break;
        }
    }
}


 void EXTI9_5_IRQHandler(void) {
    // EXTI line interrupt detected 
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_5) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5); 

        // anything that needs to happen when PB5 (ENC_B) changes state
        GPIO_PinState Echo_Pin = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5);
        if (Echo_Pin == GPIO_PIN_SET) {
            // Rising edge detected - start timing
            echo_start = Timers_GetMicroSeconds();
        } else {
            // Falling edge detected - stop timing
            echo_end = Timers_GetMicroSeconds();
                // Calculate pulse width
                echo_time = (uint32_t)(echo_end - echo_start);
                // Calculate distance in mm using Least Squares fit from matlab
                distance =  (24 * echo_time - 2589) / 100; 
        }
    }
}

#ifdef PING_TEST
int main(void)
{
    PING_Init();
    PWM_Init();
    while (1)
    {
        unsigned int dist = PING_GetDistance();
        unsigned int tof = PING_GetTimeofFlight();
        printf("Distance: %u mm, ToF: %u us\r\n", dist, tof);
        #ifdef THEREMIN_TEST
            static float Distance_adj = 0.0f;
            float alpha = 0.05;
            Distance_adj = (alpha * dist) + ((1 - alpha) * Distance_adj); //IIR filter
            if (Distance_adj < 8) Distance_adj = 8.0f;
            if (Distance_adj > 1000) Distance_adj = 1000.0f;
            // Map distance to frequency 
            unsigned int frequency = 200 + ((Distance_adj - 8) * (2000 - 200)) / (1000 - 8);
            PWM_SetFrequency(frequency);
            PWM_SetDutyCycle(PWM_0, 50); // 50% duty cycle
            PWM_Start(PWM_0);
        #endif
    }
    
    return 0;
}
#endif
