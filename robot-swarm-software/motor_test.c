/*
 * File:   motor_test.c
 * Author: Lim
 *
 * Created on May 8, 2019, 8:45 PM
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
#include <p24FJ32GA002.h>
#include "motor_control.h"

#define PWMPERIOD 333

volatile int tick = 0;
volatile int state = 0;
volatile int xor = 0;

void __attribute__((__interrupt__, __shadow__)) _T1Interrupt(void)
{
    _T1IF = 0;
    xor^= 1;
    
    if(xor) forwardMotor(80);
    else
    {
        OC1RS = PWMPERIOD + 1;
        OC2RS = PWMPERIOD + 1;
    }
//    tick++;
//    if(tick == 5) 
//    {
//        tick = 0;
//        state = (state + 1) % 3;
//    } 
}

int main(void) {
    
    setupMotors();
    T1CON = 0x00;
    TMR1 = 0x00;
    T1CONbits.TCKPS = 3;            //Prescale of 256
    PR1 = 15625;                    //1s timer period
    _T1IF = 0;
    _T1IE = 1;
    T1CONbits.TON = 1;
    while(1);
//    {
//        if(state == 0) forwardMotor(80);
//        else if(state == 1) turnLeft(80);
//        else if (state == 2) turnRight(80);
//    }
    return 0;
}
