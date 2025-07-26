/*
 * File name            : i2c.c
 * Compiler             : MPLAB XC8/ MPLAB C18 compiler
 * IDE                  : Microchip  MPLAB X IDE v5.25
 * Processor            : PIC18F66K40
 * Author               : Bhoomi Jalasandharan
 * Created on           : July 15, 2020, 05:23 PM
 * Description          : Main source file
 */


#include "congfigBits.h"
#include "variableDefinitions.h"
#include "controllerActions.h"
#ifdef DEBUG_MODE_ON_H
#include "serialMonitor.h"
#endif

#define I2C_WRITE   0b11111110
#define I2C_READ    0b00000001

/****************RTC_I2C-Library*******************/

void rtc_i2cStart(void) {
    //timer3Count = 1;  // 3 sec window
	SSP2CON2bits.SEN = 1;
    //lcdCmdExecuted = false;
    //T3CONbits.TMR3ON = ON; // Start timer thread to unlock system if GSM fails to respond within 15 sec
	while (SSP2CON2bits.SEN == SET)
        ;// && !lcdCmdExecuted);
    //lcdCmdExecuted = true;
    //if (T3CONbits.TMR3ON) {
      //  PIR5bits.TMR3IF = SET; //Stop timer thread
    //}
	//SEN =1 initiate the Start Condition on SDA and SCL Pins
	//Automatically Cleared by Hardware
	// 0 for Idle State
}

void rtc_i2cRestart(void) {
	SSP2CON2bits.RSEN = 1;
    //timer3Count = 1;  // 3 sec window
    //lcdCmdExecuted = false;
    //T3CONbits.TMR3ON = ON; // Start timer thread to unlock system if GSM fails to respond within 15 sec
	while (SSP2CON2bits.RSEN == SET )
        ;//&& !lcdCmdExecuted);
    //lcdCmdExecuted = true;
    //if (T3CONbits.TMR3ON) {
    //    PIR5bits.TMR3IF = SET; //Stop timer thread
    //}
	//RSEN = 1 initiate the Restart Condition
	//Automatically Cleared by Hardware
}

void rtc_i2cStop(void) {
	SSP2CON2bits.PEN = 1;
    //timer3Count = 1;  // 3 sec window
    //lcdCmdExecuted = false;
    //T3CONbits.TMR3ON = ON; // Start timer thread to unlock system if GSM fails to respond within 15 sec
	while (SSP2CON2bits.PEN == SET )
        ;//&& !lcdCmdExecuted);
    //lcdCmdExecuted = true;
    //if (T3CONbits.TMR3ON) {
    //    PIR5bits.TMR3IF = SET; //Stop timer thread
    //}
}

void rtc_i2cWait(void) {
    //timer3Count = 1;  // 3 sec window
    //lcdCmdExecuted = false;
    //T3CONbits.TMR3ON = ON; // Start timer thread to unlock system if GSM fails to respond within 15 sec
    while ((SSP2CON2 & 0x1F) | (SSP2STATbits.R_NOT_W))
        ;// && !lcdCmdExecuted);
    //lcdCmdExecuted = true;
    //if (T3CONbits.TMR3ON) {
    //    PIR5bits.TMR3IF = SET; //Stop timer thread
    //}
    // Wait condition until I2C bus is Idle.
}

void rtc_i2cWrite(unsigned char data) {
	SSP2BUF = data;    /* Move data to SSPBUF */
    //timer3Count = 1;  // 3 sec window
    //lcdCmdExecuted = false;
    //T3CONbits.TMR3ON = ON; // Start timer thread to unlock system if GSM fails to respond within 15 sec
    while (SSP2STATbits.BF)
        ;// && !lcdCmdExecuted);       /* wait till complete data is sent from buffer */
    //lcdCmdExecuted = true;
    //if (T3CONbits.TMR3ON) {
    //    PIR5bits.TMR3IF = SET; //Stop timer thread
    //}
    rtc_i2cWait();       /* wait for any pending transfer */
}

unsigned char rtc_i2cRead(_Bool ACK) {
	unsigned char temp;
    SSP2CON2bits.RCEN = 1;
    //timer3Count = 1;  // 3 sec window
    //lcdCmdExecuted = false;
    //T3CONbits.TMR3ON = ON; // Start timer thread to unlock system if GSM fails to respond within 15 sec
    /* Enable data reception */
    while (SSP2STATbits.BF == CLEAR)
        ;// && !lcdCmdExecuted);      /* wait for buffer full */
    //lcdCmdExecuted = true;
    //if (T3CONbits.TMR3ON) {
    //    PIR5bits.TMR3IF = SET; //Stop timer thread
    //}
    temp = SSP2BUF;   /* Read serial buffer and store in temp register */
    rtc_i2cWait();       /* wait to check any pending transfer */
    if (ACK)
        SSP2CON2bits.ACKDT=0;               //send acknowledge
    else
        SSP2CON2bits.ACKDT=1;				//Do not  acknowledge
	SSP2CON2bits.ACKEN=1;
    //timer3Count = 1;  // 3 sec window
    //lcdCmdExecuted = false;
    //T3CONbits.TMR3ON = ON; // Start timer thread to unlock system if GSM fails to respond within 15 sec
	while (SSP2CON2bits.ACKEN == SET)
        ;// && !lcdCmdExecuted);
    //lcdCmdExecuted = true;
    //if (T3CONbits.TMR3ON) {
    //    PIR5bits.TMR3IF = SET; //Stop timer thread
    //}
    return temp;     /* Return the read data from bus */
}
/**********************************************/

/****************LCD_I2C-Library*******************/

void lcd_i2cWait(void) {
    //timer3Count = 1;  // 3 sec window
    //lcdCmdExecuted = false;
    //T3CONbits.TMR3ON = ON; // Start timer thread to unlock system if GSM fails to respond within 15 sec
    while ((SSP1CON2 & 0x1F) | (SSP1STATbits.R_NOT_W))
        ;// && !lcdCmdExecuted);
    //lcdCmdExecuted = true;
    //if (T3CONbits.TMR3ON) {
    //    PIR5bits.TMR3IF = SET; //Stop timer thread
    //}
    // Wait condition until I2C bus is Idle.
}

void lcd_i2cStart(void) {
	SSP1CON2bits.SEN = 1;
    //timer3Count = 1;  // 3 sec window
    //lcdCmdExecuted = false;
    //T3CONbits.TMR3ON = ON; // Start timer thread to unlock system if GSM fails to respond within 15 sec
	while (SSP1CON2bits.SEN == SET)
        ;// && !lcdCmdExecuted);
    //lcdCmdExecuted = true;
    //if (T3CONbits.TMR3ON) {
    //    PIR5bits.TMR3IF = SET; //Stop timer thread
    //}
	//SEN =1 initiate the Start Condition on SDA and SCL Pins
	//Automatically Cleared by Hardware
	// 0 for Idle State
}

void lcd_i2cRestart(void) {
	SSP1CON2bits.RSEN = 1;
    //timer3Count = 1;  // 3 sec window
    //lcdCmdExecuted = false;
    //T3CONbits.TMR3ON = ON; // Start timer thread to unlock system if GSM fails to respond within 15 sec
	while (SSP1CON2bits.RSEN == SET)
        ;// && !lcdCmdExecuted);
    //lcdCmdExecuted = true;
    //if (T3CONbits.TMR3ON) {
    //    PIR5bits.TMR3IF = SET; //Stop timer thread
    //}
	//RSEN = 1 initiate the Restart Condition
	//Automatically Cleared by Hardware
}

void lcd_i2cStop(void) {
	SSP1CON2bits.PEN = 1;
    //timer3Count = 1;  // 3 sec window
    //lcdCmdExecuted = false;
    //T3CONbits.TMR3ON = ON; // Start timer thread to unlock system if GSM fails to respond within 15 sec
	while (SSP1CON2bits.PEN == SET)
        ;// && !lcdCmdExecuted);
    //lcdCmdExecuted = true;
    //if (T3CONbits.TMR3ON) {
    //    PIR5bits.TMR3IF = SET; //Stop timer thread
    //}
}

void lcd_i2cWrite(unsigned char data) {
	SSP1BUF = data;                 /* Move data to SSPBUF */
    //timer3Count = 1;  // 3 sec window
    //lcdCmdExecuted = false;
    //T3CONbits.TMR3ON = ON; // Start timer thread to unlock system if GSM fails to respond within 15 sec
    while (SSP1STATbits.BF)
        ;// && !lcdCmdExecuted);        /* wait till complete data is sent from buffer */
    //lcdCmdExecuted = true;
    //if (T3CONbits.TMR3ON) {
    //    PIR5bits.TMR3IF = SET; //Stop timer thread
    //}
    lcd_i2cWait();                  /* wait for any pending transfer */
}

void lcd_i2cWriteByteSingleReg(unsigned char device, unsigned char info)
{
    __delay_us(100);
    lcd_i2cWait();
    __delay_us(100);
    lcd_i2cStart();
    __delay_us(100);
    lcd_i2cWrite(device & I2C_WRITE);
    __delay_us(100);
    lcd_i2cWait();
    __delay_us(100);
    lcd_i2cWrite(info);
    __delay_us(100);
    lcd_i2cStop();
    __delay_us(100);
}
/**********************************************/