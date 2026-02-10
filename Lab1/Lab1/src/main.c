#include <stdio.h>
#include <stdlib.h>
#include <Board.h>
#include <ADC.h>
#include <pwm.h>

//Max and min for flex sensor 
#define ADC_min 1100
#define ADC_max 2500

// angle limits
#define ANGLE_MIN 0
#define ANGLE_MAX 180

//Defines for each part for testing and running 
#define Flex_sensor
// #define Flex_sensor_test
// #define Piezo_sensor_test
#define Piezo_sensor


int main(void) {
    BOARD_Init();
    ADC_Init();
    PWM_Init();
    while (1) {

    #ifdef Flex_sensor
        // Flex sensor reading from ADC_0 channel
        uint32_t Flex_sen = ADC_Read(ADC_0);
        

        // Convert ADC value to angle (0-180 degrees)
        static float Flex_adj = 0;
        float alpha = 0.05; //smoothing value
        Flex_adj = (alpha * Flex_sen) + ((1 - alpha) * Flex_adj); //IIR filter
        float angle = (float)(Flex_adj - ADC_min) / (ADC_max - ADC_min) * (ANGLE_MAX - ANGLE_MIN); //finding the angle from the ADC
        
        unsigned int Flex_freq = (unsigned int)(7.875 * angle + 1146.392857); //linear regression to map the angle


    #endif
        
    #ifdef Flex_sensor_test

        printf("Flex Sensor Value: %lu, Angle: %.2f°, Frequency: %lu Hz \n", Flex_sen, angle, Flex_freq); //Test output of ADC
        PWM_SetFrequency(Flex_freq); //setting frequency 
        PWM_SetDutyCycle(PWM_0, 50);
        PWM_Start(PWM_0); //play the tone
        
    #endif

    #ifdef Piezo_sensor_test
        uint32_t Piezo_sen = ADC_Read(ADC_1); //reading from piezo sensor
        if (Piezo_sen > 30){ //threshold to detect tap from sensor
            PWM_SetFrequency(1000); //setting a audible frequency
            PWM_SetDutyCycle(PWM_0, 50);
            PWM_Start(PWM_0);
        }else{
            PWM_SetDutyCycle(PWM_0, 0); 
        }
        printf("Piezo Sensor Value: %lu \n", Piezo_sen);
    #endif

    #ifdef Piezo_sensor
        uint32_t Piezo_sen = ADC_Read(ADC_1); //reading from piezo sensor
        if (Piezo_sen == 40){ //threshold to detect tap from sensor
            PWM_SetFrequency(Flex_freq); //setting frequency based on flex sensor
            PWM_SetDutyCycle(PWM_0, 50);
            PWM_Start(PWM_0);
            HAL_Delay(200); //delay for sound to play longer
        }else{
            PWM_SetDutyCycle(PWM_0, 0); //stop sound when no tap detected
        }
    #endif
    }
    return 0;
}