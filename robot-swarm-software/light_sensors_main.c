/*
 * File:   light_sensors_main.c
 * Author: Lim
 *
 * Created on April 20, 2019, 11:34 AM
 */


#include "xc.h"
#include <p24Fxxxx.h>
#include <p24FJ32GA002.h>
#include "light_sensors.h"
//#include "delay.h"

#define BUS_SWITCH_WRITE 0xE0
#define BUS_SWITCH_READ 0xE1
#define SENSOR_READ 0x73
#define SENSOR_WRITE 0x72
#define RED_DATA_LSB 0x50
#define RED_DATA_MSB 0x51
#define GREEN_DATA_LSB 0x52
#define GREEN_DATA_MSB 0x53
#define BLUE_DATA_LSB 0x54
#define BLUE_DATA_MSB 0x55

#define BLUE    0
#define GREEN   1

volatile int count = 0;
volatile int doneReset = 0;
volatile unsigned int lightMode;
volatile unsigned int mode;

volatile int toggleOn = 0;                                      //Set toggleOn = 0 to Off, 1 to On
volatile int lightOn = 0;                                       //Set lightOn = 0 to Off, 1 to On

void release_I2Cbus(void)
{
    I2C2CONbits.I2CEN = 0;
    //float SDA pin
    TRISBbits.TRISB2 = 1;
    LATBbits.LATB3 = 0;
    //Set SCL pin as output to toggle it
    TRISBbits.TRISB3 = 0;
    TRISBbits.TRISB8 = 0;
    
    //Setup timer 3 to toggle SCL pin 9 times at 100 kHz
    TMR3 = 0;
    T3CON = 0;
    _T3IF = 0;    
    PR3 = 7999;
    _T3IE = 1;
    T3CONbits.TON = 1;
    while(!doneReset);
    doneReset = 0;
}


void setup(void)
{
    mode = INIT;
    AD1PCFG = 0x9fff;               //All digital
    //CLKDIVbits.RCDIV = 0;           //16MHz Fcy
    INTCON2bits.ALTIVT = 0;
    I2C2CON = 0; 
    release_I2Cbus();
    I2C2STAT = 0;
    I2C2BRG = 0x4E;                 //100kHz, Fscl I2C
    IFS3bits.MI2C2IF = 0;           //Clear I2C flag
    I2C2CONbits.IPMIEN = 0; 
    I2C2CONbits.I2CEN = 1;
    _IC2IE = 0;
    
    TRISBbits.TRISB14 = 0;      //LED output
    
    //Interrupt from sleep setup routine ------ Note: Using Change Notification(CN) feature
    TRISAbits.TRISA0 = 1;
    TRISAbits.TRISA4 = 1;
    TRISBbits.TRISB7 = 1;
    _CNIF = 0;
    _CNIP = 7;
    _CN0IE = 1;
    _CN1IE = 1;
    _CN23IE = 1;
    _CNIE = 1;
    
    lightMode = BLUE;
}

void __attribute__((__interrupt__, __shadow__)) _CNInterrupt(void)
{
    int delay = 100;
    while((PORTAbits.RA0== 0) | (PORTAbits.RA4 == 0) | (PORTBbits.RB7 == 0));
    while(delay--);
    _CNIF = 0;          
    if(!toggleOn) toggleOn ^= 1;
    else
    {
        latch = 6;
        LATBbits.LATB14 = 0;
        toggleOn ^= 1;
        Sleep();
    }
}
    


void __attribute__((__interrupt__, __shadow__)) _T3Interrupt(void)
{
    _T3IF = 0;
    count++;
    TRISBbits.TRISB3 ^= 1;
    TRISBbits.TRISB8 ^= 1;            //just to test 
    if(count==18) 
    {
        T3CONbits.TON = 0;
        TMR3 = 0;
        _T3IE = 0;
        TRISBbits.TRISB2 = 0;
        PR3 = 39;
        _T3IE = 1;
        T3CONbits.TON = 1;
    }
    else if(count == 19) LATBbits.LATB2 = 0;
    else if(count == 20) LATBbits.LATB3 = 0;
    else if(count == 21) LATBbits.LATB3 = 1;
    else if(count == 22)
    {
        LATBbits.LATB2 = 1;
        T3CONbits.TON = 0;
        TMR3 = 0;
        _T3IE = 0;
        doneReset = 1;
        count = 0;
    }    
}


void I2C_route(int channel)
{
    I2C2CONbits.SEN = 1;
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0;
    while(I2C2STATbits.IWCOL);
    I2C2STATbits.IWCOL = 0;
    I2C2TRN = BUS_SWITCH_WRITE;
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0;
    while(I2C2STATbits.IWCOL);
    I2C2STATbits.IWCOL = 0;
    I2C2TRN = (0xF1 << channel);
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0;
    I2C2CONbits.PEN = 1;
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0;   
}

char I2C_bus_read(void)
{
    char status;
    I2C2CONbits.SEN = 1;
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0;
    while(I2C2STATbits.IWCOL);
    I2C2STATbits.IWCOL = 0;
    I2C2TRN = BUS_SWITCH_WRITE;
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0;
    I2C2CONbits.RSEN = 1;
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0;
    while(I2C2STATbits.IWCOL);
    I2C2STATbits.IWCOL = 0;
    I2C2TRN = BUS_SWITCH_READ;
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF= 0;
    I2C2CONbits.RCEN = 1;
    while(!I2C2STATbits.RBF);
    IFS3bits.MI2C2IF = 0;
    status = I2C2RCV;
    I2C2CONbits.ACKDT = 1;
    I2C2CONbits.ACKEN = 1;
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0;
    I2C2CONbits.PEN = 1;
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0; 
    return status;
}

void I2C_write(char address, char package)
{
    I2C2CONbits.SEN = 1;
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0;
    I2C2TRN = SENSOR_WRITE;
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0;
    I2C2TRN = address;
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0;
    I2C2TRN = package;
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0;
    I2C2CONbits.PEN = 1;
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0;
}

void setup_LightSensor(int channel)
{
    I2C_route(channel);
    I2C_write(0x60,0x08);               //Just to clear any previous latching interrupt value
    I2C_write(0x40,0x0b);               //Default settings, INT pin not initialized
    I2C_write(0x41, 0x00);              //RGBC measurement time set at update rate of 160mscec(MODE_CONTROL1)
    I2C_write(0x42, 0x12);              //Enable RGBC measurement, with ADC gain setting of 16(MODE_CONTROL2)
    I2C_write(0x44, 0x02);              //MODE_CONTROL3
    I2C_write(0x62, 0x00);              //High threshold LSB, threshold set to  0x00x3500
    I2C_write(0x63, 0x35);              //High threshold MSB
    I2C_write(0x61, 0x02);              //Persistence set to 4 consecutive measurements above threshold
    //I2C_write(0x63, 0x01);              //Persistence set to a single measurement above threshold
    I2C_write(0x60,0x19);               //Interrupt register: Enabled, latched, blue as source of interrupt  
}

char I2C_read(int channel,char reg)
{
    char result;
    
    I2C_route(channel);
    I2C2CONbits.SEN = 1;
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0;
    I2C2TRN = SENSOR_WRITE;
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0;
    I2C2TRN = reg;                  
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0;
    I2C2CONbits.RSEN = 1;
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0;
    I2C2TRN = SENSOR_READ;
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0;
    I2C2CONbits.RCEN = 1;
    while(!I2C2STATbits.RBF);
    IFS3bits.MI2C2IF = 0;
    result = I2C2RCV;
    I2C2CONbits.ACKDT = 1;               //Send a NACK
    I2C2CONbits.ACKEN = 1;
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0;
    I2C2CONbits.PEN = 1;    
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0;  
    return result;
}



//void delay(int window)
//{
//    while(window--) wait_1ms();
//}


