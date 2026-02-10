#include <stdio.h>
#include <stdlib.h>
#include <Board.h>
#include <ADC.h>
#include <pwm.h>
#include <buttons.h>

//define parts to be run for each section:
// #define Part_2
// #define Part_3_1
// #define Part_3_2
// #define Part_3_3
#define Part_3_4

//defines for adjustable values:
uint32_t PWM_FREQ = 1000; //Frequency that can be changed (hz)
//higher frequency = higher pitch
uint8_t PWM_DUTY_CYCLE = 50; //Duty cycle that can be changed
//higher duty cycle = louder sound
int btn;

int main(void) {
    BOARD_Init();
    ADC_Init();
    PWM_Init();
    BUTTONS_Init();

    #ifdef Part_2
    while (TRUE) {
        printf("Hello World\r\n");
    }
    #endif

    while (TRUE) {


#ifdef Part_3_1
        ADC_Start();
        uint32_t adcValue = ADC_Read(POT); //Reads the potentiometer value from POT channel
        printf("A/D: %u\r\n", adcValue); //Prints the value of the potentiometer
#endif 

#ifdef Part_3_2
        PWM_SetFrequency(PWM_FREQ); //Setting the frequency to speaker
        PWM_SetDutyCycle(PWM_0, PWM_DUTY_CYCLE); //Setting the initial duty cycle
        PWM_Start(PWM_0); //Starting the PWM to make sound
#endif 

#ifdef Part_3_3
        ADC_Start();
        uint32_t adcValue = ADC_Read(POT); //Reads the potentiometer value from POT channel
        static float f_adc = 0;
        float alpha = 0.05; //smoothing value
        f_adc = (alpha * adcValue) + ((1 - alpha) * f_adc); //IIR filter
        PWM_FREQ = ((uint32_t)f_adc * 20000) / 4095 + 100; //match the potentiometer value to the value of the speaker frequency
        PWM_SetFrequency(PWM_FREQ); //Setting the frequency to speaker
        PWM_SetDutyCycle(PWM_0, PWM_DUTY_CYCLE); //Setting the initial duty cycle
        PWM_Start(PWM_0); //Starting the PWM to make sound
#endif

#ifdef Part_3_4
        ADC_Start();
        uint32_t adcValue = ADC_Read(POT); //Reads the potentiometer value from POT channel
        static float f_adc = 0;
        float alpha = 0.05; //smoothing value
            f_adc = (alpha * adcValue) + ((1 - alpha) * f_adc); //IIR filter
            uint32_t additional = ((uint32_t)f_adc * 20000) / 4095 + 100; //match the potentiometer value to the value of the speaker frequency
        btn = buttons_state();
        //Checking which button is pressed then change the frequency:
        if ((btn & 0x1) == 0) {
            PWM_FREQ = 100;
            PWM_FREQ = (PWM_FREQ + additional) / 2; //average the set value with the potentiometer value
            PWM_SetFrequency(PWM_FREQ);
            PWM_SetDutyCycle(PWM_0, PWM_DUTY_CYCLE); 
            PWM_Start(PWM_0);
            printf("Button 0 pressed\r\n");
        }else if ((btn & 0x2) == 0) {
            PWM_FREQ = 1000;
            PWM_FREQ = (PWM_FREQ + additional) / 2;
            PWM_SetFrequency(PWM_FREQ);
            PWM_SetDutyCycle(PWM_0, PWM_DUTY_CYCLE); 
            PWM_Start(PWM_0);
            printf("Button 1 pressed\r\n");
        }else if ((btn & 0x4) == 0) {
            PWM_FREQ = 2000;
            PWM_FREQ = (PWM_FREQ + additional) / 2;
            PWM_SetFrequency(PWM_FREQ);
            PWM_SetDutyCycle(PWM_0, PWM_DUTY_CYCLE); 
            PWM_Start(PWM_0);
            printf("Button 2 pressed\r\n");   
        }else if ((btn & 0x8) == 0) {
            PWM_FREQ = 3000;
            PWM_FREQ = (PWM_FREQ + additional) / 2;
            PWM_SetFrequency(PWM_FREQ);
            PWM_SetDutyCycle(PWM_0, PWM_DUTY_CYCLE); 
            PWM_Start(PWM_0);
            printf("Button 3 pressed\r\n");
        }else{
            PWM_SetDutyCycle(PWM_0, 0); 
        }

#endif
    }
}
