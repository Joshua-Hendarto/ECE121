#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "BOARD.h"
#include "QEI.h"
#include "PWM.h"


#define QEI_TEST
// #define QEI_TEST_ROTATE
#define QEI_LEDS

static int32_t turn_count = 0;

#ifdef QEI_TEST

//set default color
static int angle;
    #ifdef QEI_LEDS
        static uint8_t LED_R = 255;
        static uint8_t LED_G = 255;
        static uint8_t LED_B = 0;
        #define OFFSET 60
    #endif
#endif

typedef enum {
    QEI_OFF,      // Both A and B are off
    QEI_A_B,      // A is before B
    QEI_B_A,      // B is before A
} QEI_State;

static QEI_State currentState = QEI_OFF; //Set initial state

/**
 * @function QEI_Init(void)
 * @param none
 * @brief  Enables the external GPIO interrupt, anything
 *         else that needs to be done to initialize the module. 
 * @return none
*/
void QEI_Init(void){
    //setting pin on GPIO for A and B
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5;          // Both pins
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;     // Interrupt on both edges
    GPIO_InitStruct.Pull = GPIO_NOPULL;                     // No pull-up/down
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);                 // Port B
    
    // Enable EXTI interrupts for the pins
    HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);    // PB4 interrupt priority
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);             // Enable PB4 interrupt
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);  // PB5 interrupt priority
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);           // Enable PB5 interrupt
    
    // Initialize the state
    uint8_t pinA = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_SET) ? 1 : 0; 
    uint8_t pinB = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_SET) ? 1 : 0;
    currentState = (pinB << 1) | pinA;  // State = [B A]
}

void QEI_IRQ(void){
  // State = [B A]
    // Read current pin states
    uint8_t pinA = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_SET) ? 1 : 0;
    uint8_t pinB = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_SET) ? 1 : 0;
    QEI_State newState = (pinB << 1) | pinA;
    
    // Determine direction based on state transition
    switch (currentState) {
        case QEI_OFF:  // Both A and B are off
            break;
        case QEI_A_B:  // A is before B
            if (newState == QEI_OFF) {
                // Counter-clockwise count
                turn_count--;
            }
            break;
        case QEI_B_A:  // B is before A
            if (newState == QEI_OFF) { 
                turn_count++;// Clockwise count
            }
            break;
        default:
            break;
    }
    currentState = newState;
}

/**
 * @function QEI_GetPosition(void) 
 * @param none
 * @brief This function returns the current position of the Quadrature Encoder in degrees.      
*/
int QEI_GetPosition(void){
    // Calculate angle from turn_count without modifying turn_count
    int angle = (turn_count * 360) / 24; // 24 pulses per revolution
    #ifdef QEI_LEDS
    angle = (turn_count * 360) / 98; //doubling the pulses
    #endif

    #ifdef QEI_LEDS
    if (angle >= 360) {
        angle = angle % 360; //Wrap around for angles >= 360
    } else if (angle < 0) {
        angle = ((angle % 360) + 360) % 360; //Remove negative angles
    }
    #endif
    return angle;
}

/**
 * @Function QEI_ResetPosition(void) 
 * @param  none
 * @return none
 * @brief  Resets the encoder such that it starts counting from 0.
*/
void QEI_ResetPosition(){
    turn_count = 0;
    currentState = QEI_OFF;
}


//Interupt for Channel A
void EXTI4_IRQHandler(void){
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_4) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);
        // Handle interrupt for GPIO_PIN_4 (Channel A)
    }
}



//Interrupt for Channel B
void EXTI9_5_IRQHandler(void){
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_5) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5);
        // Handle interrupt for GPIO_PIN_5 (Channel B)
    }
}

//Testing for QEI

#ifdef QEI_TEST
int main(void){
    BOARD_Init();
    QEI_Init();
    PWM_Init();
    while(1){
        #ifdef QEI_TEST_ROTATE
            QEI_IRQ();
            angle = QEI_GetPosition(); // position in degrees
            printf("Encoder Position: %d degrees\n", angle);
        #endif
        #ifdef QEI_LEDS //seting LED colors based on position
            QEI_IRQ();
            angle = QEI_GetPosition(); 

            if (angle < 0) {
                angle = ((angle % 360) + 360) % 360; // removing negative angles
            }
            int preset = (angle + OFFSET) % 360; // setting initial as yellow 
            if (preset >= 0 && preset < 60) { // Red to yellow
                LED_R = 255;
                LED_G = preset * 255 / 60;
                LED_B = 0;
            } else if (preset >= 60 && preset < 120) { // Yellow to Green
                LED_R = 255 - ((preset - 60) * 255 / 60);
                LED_G = 255;
                LED_B = 0;
            } else if (preset >= 120 && preset < 180) { // Green to Cyan
                LED_R = 0;
                LED_G = 255;
                LED_B = (preset - 120) * 255 / 60;
            } else if (preset >= 180 && preset < 240) { // Cyan to Blue
                LED_R = 0;
                LED_G = 255 - ((preset - 180) * 255 / 60);
                LED_B = 255;
            } else if (preset >= 240 && preset < 300) { // Blue to Magenta
                LED_R = (preset - 240) * 255 / 60;
                LED_G = 0;
                LED_B = 255;
            } else if (preset >= 300 && preset < 360) { // Magenta to Red
                LED_R = 255;
                LED_G = 0;
                LED_B = 255 - ((preset - 300) * 255 / 60);
            } else { // Yellow when reset
                LED_R = 255;
                LED_G = 255;
                LED_B = 0;
            }

            //changing Duty cycle to set the color strength for each LED
            PWM_SetDutyCycle(PWM_1, 100 - (LED_R * 100 / 255)); // Red LED
            PWM_SetDutyCycle(PWM_2, 100 - (LED_G * 100 / 255)); // Green LED
            PWM_SetDutyCycle(PWM_3, 100 - (LED_B * 100 / 255)); // Blue LED
            PWM_Start(PWM_1);
            PWM_Start(PWM_2);
            PWM_Start(PWM_3);
            printf("LED Colors - R: %d, G: %d, B: %d, Angle: %d\n", LED_R, LED_G, LED_B, angle);
        #endif
    }
    return 0;
    
}
#endif
