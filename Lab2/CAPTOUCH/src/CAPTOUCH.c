#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "BOARD.h"
#include "CAPTOUCH.h"
#include "timers.h"

#define CAP_TOUCH_TEST
#define N 8

TIM_HandleTypeDef htim2;
static volatile uint32_t previous_reads[N] = {0}; //save previous readings
static volatile uint32_t current_time = 0;
static volatile uint32_t last_time = 0;
static volatile uint32_t average = 0;

/** CAPTOUCH_Init()
 *
 * This function initializes the module for use. Initialization is done by
 * opening and configuring timer 2, opening and configuring the GPIO pin and
 * setting up the interrupt.
 */
void CAPTOUCH_Init(void) {
    //Configure GPIO pin PB5 
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // EXTI interrupt init
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

    // the rest of the function goes here
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 83; // 1 MHz timer frequency
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 0xFFFFFFFF; // max period
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim2);
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);
    HAL_TIM_Base_Start(&htim2);
}

/** CAPTOUCH_IsTouched(void)
 *
 * Returns TRUE if finger is detected. Averaging of previous measurements may
 * occur within this function, however you are NOT allowed to do any I/O inside
 * this function.
 *
 * @return  (char)    [TRUE, FALSE]
 */
char CAPTOUCH_IsTouched(void){
    if (average > 150){ // threshold value for touch detection
        return TRUE;
    } else {
        return FALSE;
    }
}


void TIM2_IRQHandler(void) {
    if (__HAL_TIM_GET_IT_SOURCE(&htim2, TIM_IT_UPDATE) != RESET) {
        __HAL_TIM_CLEAR_IT(&htim2, TIM_IT_UPDATE); 
    }
}

void EXTI9_5_IRQHandler(void) {
    // EXTI line interrupt detected 
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_5) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5); 
        GPIO_PinState Pin_State = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5); //check pin state
        uint32_t current_time = __HAL_TIM_GET_COUNTER(&htim2); //get current timer value
        average = current_time - last_time; 
        if (Pin_State == GPIO_PIN_SET) {
            current_time = __HAL_TIM_GET_COUNTER(&htim2);
            for (int i = N - 1; i > 0; i--) { // shift previous reads to add the new one
                previous_reads[i] = previous_reads[i - 1];
                average += previous_reads[i];
            }
            previous_reads[0] = current_time - last_time;// store new reading
            average /= N; // compute the average frequency
            last_time = current_time; //set the old time to the current time
        }
    }
}



#ifdef CAP_TOUCH_TEST
int main(void){
    BOARD_Init();
    CAPTOUCH_Init();
    while(1){
        char touch;
        touch = CAPTOUCH_IsTouched();
        if (touch){
            printf("Touched: TRUE average =%lu\n", average);
        }else{
            printf("Touched: FALSE average =%lu\n", average);
        }
    }
}
#endif
