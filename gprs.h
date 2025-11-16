/*
 * File name            : gsm.h
 * Compiler             : MPLAB XC8/ MPLAB C18 compiler
 * IDE                  : Microchip  MPLAB X IDE v5.25
 * Processor            : PIC18F66K40
 * Author               : Bhoomi Jalasandharan
 * Created on           : July 15, 2020, 05:23 PM
 * Description          : UART functions header file
 */

#ifndef UART3_H
#define	UART3_H

/***************************** Serial communication functions declarations#start ****/
unsigned char rxByte(void); // To receive a byte from GSM
void txByte(unsigned char); // To transmit a byte to GSM
void transmitStringToGSM(const char*); // To transmit string of bytes to GSM
void transmitNumberToGSM(char*, size_t); // To transmit array of bytes to GSM
void setGsmToLocalTime(void); // To set GSM at local time standard across the globe
void checkGsmConnection(void); // To check GSM connection
void configureGPRS(void);
void configureMQTT(void); // To enable reception
void deleteMsgFromSIMStorage(void); // To delete sms from sim memory
_Bool checkSignalStrength(void); // To check GSM signal strength
void publishResponse(const char*, _Bool); // To publish Response
void publishNotification(const char*, const char*, _Bool);
void checkResponse(unsigned char); // Check gsm response upon complete reception
void checkPingResponse(void); //transmit the AT command "AT+CMQTTPING=0" via UART and monitor the response buffer
/***************************** Serial communication functions declarations#end ******/

#endif
/* UART3_H */