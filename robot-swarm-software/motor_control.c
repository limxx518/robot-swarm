/*
 * File:   motor_control.c
 * Author: Lim
 *
 * Created on May 5, 2019, 1:12 PM
 */


#include "xc.h"
#include <p24Fxxxx.h>
#include <p24FJ32GA002.h>

#define PWMPERIOD 133

void setupMotors(void)
{
    AD1PCFG = 0x9fff;
    CLKDIVbits.RCDIV = 0;
    T2CON = 0;                  //Configure Timer 2 to be the timer source for both OC1 and OC2
    TMR2 = 0;
    PR2 = PWMPERIOD;                  //Set frequency to be at 12kHz(might need to adjust)   
    _T2IF = 0;
    TRISBbits.TRISB15 = 0;      //Configure PWM 1 pin as output
    TRISBbits.TRISB13 = 0;  
    
    //PPS Unlock sequence
    __builtin_write_OSCCONL(OSCCON & 0xbf);
    _RP15R = 18;                //PWM 1 set to OC1
    _RP13R = 19;                //PWM 2 set to OC2
    __builtin_write_OSCCONL(OSCCON | 0x40);
    
    //OC1 setup
    OC1CON = 0;
    OC1CONbits.OCTSEL = 0;          //Timer 2 as source
    OC1CONbits.OCM = 0b110;         //PWM mode with no fault feature
    OC1R = 116;                     //Set soft start of 70% duty cycle, ~2.3V
    
    //OC2 setup
    OC2CON = 0;
    OC2CONbits.OCTSEL = 0;          //Timer 2 as source
    OC2CONbits.OCM = 0b110;         //PWM mode with no fault feature
    OC2R = 116;
}

void forwardMotor(int dutyCycle)
{
    if(T2CONbits.TON);
    else T2CONbits.TON = 1;                  //Enable PWM signaling
    OC1RS = (unsigned int)((100-dutyCycle) * PWMPERIOD)/100;
    OC2RS = (unsigned int)((100-dutyCycle) * PWMPERIOD)/100;
}

void turnLeft(int dutyCycle)
{
    //OC1RS = (20 * PWMPERIOD)/100;
    if(T2CONbits.TON);
    else T2CONbits.TON = 1;
    OC2RS = PWMPERIOD + 1;
    OC1RS = (unsigned int)((100-dutyCycle) * PWMPERIOD)/100;
}

void turnRight(int dutyCycle)
{
    //OC2RS = (20 * PWMPERIOD)/100;
    if(T2CONbits.TON);
    else T2CONbits.TON = 1;
    OC1RS = PWMPERIOD + 1;
    OC2RS = (unsigned int)((100-dutyCycle) * PWMPERIOD)/100;
}

