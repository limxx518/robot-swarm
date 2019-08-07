/*
 * File:   swarm_bots_main_ver4.c
 * Author: Lim
 *
 * Created on July 7, 2019, 5:24 PM
 */


// PIC24FJ32GA002 Configuration Bit Settings

// 'C' source line config statements

// CONFIG2
//#pragma config POSCMOD = NONE           // Primary Oscillator Select 
#pragma config I2C1SEL = PRI            // I2C1 Pin Location Select (Use default SCL1/SDA1 pins)
#pragma config IOL1WAY = OFF            // IOLOCK Protection (IOLOCK may be changed via unlocking seq)
#pragma config OSCIOFNC = ON            // Primary Oscillator Output Function (OSC2/CLKO/RC15 functions as port I/O (RC15))
#pragma config FCKSM = CSECME           // Clock Switching and Monitor (Clock switching is enabled, Fail-Safe Clock Monitor is enabled)
//#pragma config FNOSC = FRCPLL           // Oscillator Select (Fast RC Oscillator with PLL module (FRCPLL))
#pragma config SOSCSEL = SOSC           // Sec Oscillator Select (Default Secondary Oscillator (SOSC))
#pragma config WUTSEL = LEG             // Wake-up timer Select (Legacy Wake-up Timer)
#pragma config IESO = ON                // Internal External Switch Over Mode (IESO mode (Two-Speed Start-up) enabled)

// CONFIG1
#pragma config WDTPS = PS32768          // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (Prescaler ratio of 1:128)
#pragma config WINDIS = ON              // Watchdog Timer Window (Standard Watchdog Timer enabled,(Windowed-mode is disabled))
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config ICS = PGx1               // Comm Channel Select (Emulator EMUC1/EMUD1 pins are shared with PGC3/PGD3)
#pragma config GWRP = OFF               // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF                // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG port is disabled)


#include <xc.h>
#include <p24Fxxxx.h>
#include <stdlib.h>
#include "light_sensors.h"
#include "motor_control.h"
//#include "delay.h"

#define DELAY_1_MS 400

//State machine definitions
#define DEFAULT_STATE 0
#define ADJUST_RG     1
#define ADJUST_LG     2
#define ADJUST_LR     3
#define ADJUST_RR     4
#define SENSITIVITY   5
#define ERR_RANGE     500
#define THRESHOLD     0x3200

volatile unsigned int currentState;
volatile unsigned int lastF = 0;

volatile unsigned int latch = 0;

volatile unsigned int redValue;
volatile unsigned int greenValue;
volatile unsigned int blueValue;

unsigned long rgb[3][4] = {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}};      //2D array of the 3 sensors with corresponding RGB and sum value
unsigned long maxRed[2] = {0,0};                                  //A 2-element array that denotes the sensor with max value and raw value
unsigned long maxGreen[2] = {0,0};
unsigned long maxBlue [2] = {0,0};

//T1Interrupt only for testing purposes
void __attribute__((__interrupt__, __shadow__)) _T1Interrupt(void)
{
    unsigned char temp;
    _T1IF = 0;
    temp = I2C_read(0,0x50);            //Red LSB, channel 0
    redValue = (I2C_read(0,0x51) << 8) | temp;
    temp = I2C_read(0,0x52);            //Green LSB, channel 0
    greenValue = (I2C_read(0,0x53) << 8) | temp;
    temp = I2C_read(0, 0x54);           //Blue LSB, channel 0
    blueValue = (I2C_read(0,0x55) << 8) | temp;
}

void motorControl(void)
{
    if(currentState == ADJUST_RG || currentState == ADJUST_RR) turnRight(80);
    else if(currentState == ADJUST_LG || currentState == ADJUST_LR) turnLeft(80);
    else forwardMotor(80);
}

void readValues(void)
{
    int i = 0, j = 0, delay;
    unsigned char temp1, temp2;
    delay = DELAY_1_MS;
    
    if(latch) latch--;
    
    //Initialization block
    while(j < 3)
    {
        rgb[j][3] = 0;
        j++;
    }
    
    while(i < 3)
    {
        while(delay--);
        temp1 = I2C_read(i,0x50);
        temp2 = I2C_read(i, 0x51);
        rgb[i][0] = (temp2 << 8) | temp1;
        rgb[i][3] += rgb[i][0];                 //Red value
        if(i == 0)
        {
            maxRed[0] = i;
            maxRed[1] = rgb[i][0];
        }
        else
        {
            if(rgb[i][0] > maxRed[1] )
            {
                maxRed[0] = i;
                maxRed[1] = rgb[i][0];
            }
        }
        temp1 = I2C_read(i, 0x52);
        temp2 = I2C_read(i, 0x53);
        rgb[i][1] = (temp2 << 8) | temp1;
        rgb[i][3] += rgb[i][1];
        if( i == 0 )
        {
            maxGreen[0] = i;
            maxGreen[1] = rgb[i][1];
        }
        else
        {
            if(rgb[i][1] > maxGreen[1])
            {
                maxGreen[0] = i;
                maxGreen[1] = rgb[i][1];
            }
        }
        temp1 = I2C_read(i, 0x54);
        temp2 = I2C_read(i, 0x55);
        rgb[i][2] = (temp2 << 4) | temp1;
        rgb[i][3] += rgb[i][2];
        if(i == 0)
        {
            maxBlue[0] = i;
            maxBlue[1] = rgb[i][2];
        }
        else
        {
            if(rgb[i][2] > maxBlue[1])
            {
                maxBlue[0] = i;
                maxBlue[1] = rgb[i][2];
            }
        }
        i++;
    }
    if(latch == 0 && rgb[0][1] >= THRESHOLD && rgb[1][1] >= THRESHOLD && rgb[2][1] >= THRESHOLD) 
    {
        LATBbits.LATB14 ^= 1;
        latch = 6;          // 3 sec latching debounce;
    }
}

void poll_1s()
{
    int placeholder;
    if(_T1IF)
    {
        _T1IF = 0;
        readValues();
        placeholder = (maxGreen[1]*10 / rgb[maxRed[0]][3]);
        switch(currentState){
            case DEFAULT_STATE: 
                if((maxGreen[1] >= maxRed[1]) && ((maxGreen[1]*10 / rgb[maxGreen[0]][3]) >= SENSITIVITY))
                {
                    if(maxGreen[0] == 2) currentState = ADJUST_LG;
                    else if(maxGreen[0] == 0) currentState = ADJUST_RG;
                    //if(maxGreen[0] != 1) lastF = maxGreen[1];
                }
                else if((maxRed[1] > maxGreen[1]) && ((maxRed[1]*10 / rgb[maxRed[0]][3]) >= SENSITIVITY))
                {
                    if(maxRed[0] == 2) currentState = ADJUST_LR;
                    else if(maxRed[0] == 0) currentState = ADJUST_RR;
                    //if(maxRed[0] != 1) lastF = maxRed[1];
     
                }
                else currentState = DEFAULT_STATE;
                //lastF = 0;
                break;
            case ADJUST_RG:
                if((maxGreen[1] >= maxRed[1]))
                {
                    if(maxGreen[0] == 2 && (maxGreen[1]*10 / rgb[maxGreen[0]][3]) >= SENSITIVITY) 
                    {
                        currentState = ADJUST_LG;
                        lastF = 0;
                    }
                    else if(maxGreen[0] == 1)       //might consider adding threshold check to ensure correct operation when no light is shined
                    {
                        if(maxGreen[1] <= lastF)
                        {
                            currentState = DEFAULT_STATE;
                            lastF = 0;
                            break;
                        }
                        lastF = maxGreen[1];
                    }
                    else currentState = ADJUST_RG;
                    //lastF = maxGreen[1];
                }
                else if((maxRed[1] > maxGreen[1]) && ((maxRed[1]*10 / rgb[maxRed[0]][3]) >= SENSITIVITY))
                {
                    if(maxRed[0] == 2) currentState = ADJUST_LR;
                    else if(maxRed[0] == 0) currentState = ADJUST_RR;
                    else currentState = DEFAULT_STATE;                    
                    lastF = 0;
                }
                else 
                {
                    currentState = DEFAULT_STATE;          //In the case where all light sources are off, assume default state
                    lastF = 0;
                }
                break;
            case ADJUST_LG:
                if((maxGreen[1] >= maxRed[1]))
                {
                    if(maxGreen[0] == 0 && (maxGreen[1]*10 / rgb[maxGreen[0]][3]) >= SENSITIVITY) 
                    {
                        currentState = ADJUST_RG;
                        lastF = 0;
                    }
                    else if(maxGreen[0] == 1) 
                    {                       
                        if(maxGreen[1] <= lastF)
                        {
                            currentState = DEFAULT_STATE;
                            lastF = 0;
                            break;
                        }
                        lastF = maxGreen[1];
                    }
                    else currentState = ADJUST_LG;
                    //lastF = maxGreen[1];
                }
                else if((maxRed[1] > maxGreen[1]) && ((maxRed[1]*10 / rgb[maxRed[0]][3]) >= SENSITIVITY))
                {
                    if(maxRed[0] == 2) currentState = ADJUST_LR;
                    else if(maxRed[0] == 0) currentState = ADJUST_RR;
                    else currentState = DEFAULT_STATE;
                    lastF = 0;                  
                }
                else 
                {
                    currentState = DEFAULT_STATE;
                    lastF = 0;
                }
                break;
            case ADJUST_RR:
                if((maxRed[1] >= maxGreen[1]))
                {
                    if(maxRed[0] == 2 && (maxRed[1]*10 / rgb[maxRed[0]][3]) >= SENSITIVITY) 
                    {
                        currentState = ADJUST_LR;
                        lastF = 0;
                    }
                    else if(maxRed[0] == 1) 
                    {                      
                        if(maxRed[1] <= lastF)
                        {
                            currentState = DEFAULT_STATE;
                            lastF = 0;
                            break;
                        }
                        lastF = maxRed[1];
                    }
                    else currentState = ADJUST_RR;
                    //lastF = maxRed[1];
                }
                else if((maxGreen[1] > maxRed[1]) && ((maxGreen[1]*10 / rgb[maxGreen[0]][3]) >= SENSITIVITY))
                {
                    if(maxGreen[0] == 2) currentState = ADJUST_LG;
                    else if(maxGreen[0] == 0) currentState = ADJUST_RG;
                    else currentState = DEFAULT_STATE;                   
                    lastF = 0;
                }
                else 
                {
                    currentState = DEFAULT_STATE;
                    lastF = 0;
                }
                break;
            case ADJUST_LR:
                if((maxRed[1] >= maxGreen[1]))
                {
                    if(maxRed[0] == 0 && (maxRed[1]*10 / rgb[maxRed[0]][3]) >= SENSITIVITY) 
                    {
                        currentState = ADJUST_RR;
                        lastF = 0;
                    }
                    else if(maxRed[0] == 1) 
                    {                       
                        if(maxRed[1] <= lastF)
                        {
                            currentState = DEFAULT_STATE;
                            lastF = 0;
                            break;
                        }
                        lastF = maxRed[1];
                    }
                    else currentState = ADJUST_LR;
                    //lastF = maxRed[1];
                }
                else if((maxGreen[1] > maxRed[1]) && ((maxGreen[1]*10 / rgb[maxGreen[0]][3]) >= SENSITIVITY))
                {
                    if(maxGreen[0] == 2) currentState = ADJUST_LG;
                    else if(maxGreen[0] == 0) currentState = ADJUST_LG;
                    else currentState = DEFAULT_STATE;                    
                    lastF = 0;
                }
                else 
                {
                    currentState = DEFAULT_STATE;
                    lastF = 0;
                }
                break;
        }
    }
}

int main(void) {
    
    char busStatus;
    char test;
    int delay = DELAY_1_MS;
    
    while(delay--);
    
    setup();
    currentState = DEFAULT_STATE;
    TRISBbits.TRISB14 = 0;
    setup_LightSensor(0);
    while(delay--);
    delay = DELAY_1_MS;
    setup_LightSensor(1);
    while(delay--);
    setup_LightSensor(2);
    Sleep();
    asm("NOP");     //TO DO: Set _CNIE = 0 when I2C read is running in polling loop
    setupMotors();
    //I2C_route(1);
    //delay(1);
    //busStatus = I2C_bus_read();
    //busStatus = I2C_bus_read();
    //test = I2C_read(2, 0x92);
    
    //The block of code below is just for testing purposes, not to be integrated in final revision
    T1CON = 0x00;
    TMR1 = 0x00;
    T1CONbits.TCKPS = 2;            //Prescale of 64
    PR1 = 12500;                    //200ms timer period
    _T1IF = 0;
    _T1IE = 0;
    T1CONbits.TON = 1;
    while(1)
    {
        poll_1s();
        motorControl();
    }  
    return 0;
}

