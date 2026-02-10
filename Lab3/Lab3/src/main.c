#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "BOARD.h"
#include "I2c.h"
#include "BNO055.h"

// #define Accelerometer
#define Magnetometer
// #define Gyroscope

#ifdef Accelerometer
    static int xAccel;
    static int yAccel;  
    static int zAccel;
#endif

#ifdef Magnetometer
    static int xMag;
    static int yMag;
    static int zMag;
#endif

#ifdef Gyroscope
    static int xGyro;
    static int yGyro;
    static int zGyro;
#endif

int main(void)
{
    BOARD_Init();
    I2C_Init();
    BNO055_Init();
    
    while (1)
    {
        #ifdef Accelerometer
        xAccel =  BNO055_ReadAccelX();
        yAccel = BNO055_ReadAccelY();
        zAccel = BNO055_ReadAccelZ();
        printf("%d, %d, %d\r\n", xAccel, yAccel, zAccel);
        HAL_Delay(50);
        #endif

        #ifdef Magnetometer
        xMag = BNO055_ReadMagX();
        yMag = BNO055_ReadMagY();
        zMag = BNO055_ReadMagZ();
        printf("%d, %d, %d\r\n", xMag, yMag, zMag);
        HAL_Delay(100);
        #endif

        #ifdef Gyroscope
        xGyro = BNO055_ReadGyroX();
        yGyro = BNO055_ReadGyroY();
        zGyro = BNO055_ReadGyroZ();
        printf("%d, %d, %d\r\n", xGyro, yGyro, zGyro);
        HAL_Delay(100);
        #endif
    }
}