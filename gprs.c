/*
 * File name            : gsm.c
 * Compiler             : MPLAB XC8/ MPLAB C18 compiler
 * IDE                  : Microchip  MPLAB X IDE v5.25
 * Processor            : PIC18F66K40
 * Author               : Bhoomi Jalasandharan
 * Created on           : July 15, 2020, 05:23 PM
 * Description          : uart functions definitions source file
 */

#include "congfigBits.h"
#include "variableDefinitions.h"
#include "controllerActions.h"
#include "gprs.h"
#include "i2c_LCD_PCF8574.h"
#include "i2c.h"


//*****************Serial communication function_Start****************//

/*************************************************************************************************************************

This function is called to receive Byte data from GSM
The purpose of this function is to return Data loaded into Reception buffer (RCREG) until Receive flag (RCIF) is pulled down

 **************************************************************************************************************************/
unsigned char rxByte(void) {
    while (PIR4bits.RC3IF == CLEAR); // Wait until RCIF gets low
    // ADD indication if infinite
    return RC3REG; // Return data stored in the Reception register
}

/*************************************************************************************************************************

This function is called to transmit Byte data to GSM
The purpose of this function is to transmit Data loaded into Transmit buffer (TXREG) until Transmit flag (TXIF) is pulled down

 **************************************************************************************************************************/
// Transmit data through TX pin
void txByte(unsigned char serialData) {
    TX3REG = serialData; // Load Transmit Register 
    while (PIR4bits.TX3IF == CLEAR); // Wait until TXIF gets low
    // ADD indication if infinite
}

/*************************************************************************************************************************

This function is called to transmit data to GSM in string format
The purpose of this function is to call transmit Byte data (txByte) Method until the string register reaches null.

 **************************************************************************************************************************/
void transmitStringToGSM(const char *string) {
    // Until it reaches null
    while (*string) {
        txByte(*string++); // Transmit Byte Data
        __delay_ms(10);
    }
}

/*************************************************************************************************************************

This function is called to transmit data to GSM in Number format
The purpose of this function is to call transmit Byte data (txByte) Method until mentioned index.

 **************************************************************************************************************************/
void transmitNumberToGSM(char *number, size_t index) {
    unsigned char j = CLEAR;
    // Until it reaches index no.
    while (j < index) {
        txByte(*number++); // Transmit Byte Data
        j++;
        __delay_ms(10);
    }
}

/*************************************************************************************************************************

This function is called to publish notification.

***************************************************************************************************************************/
void publishNotification(const char *alert, const char *message, _Bool isnum) {
    unsigned int stringLength;
    //publish operations:
    //Set topic
 
    transmitStringToGSM("\r\nAT+CMQTTTOPIC=0,");
    stringLength = strlen((const char *)deviceId) + 13;
    clearTempBytesString();
    sprintf(temporaryBytesArray, "%d", stringLength);
    temp = strlen((const char *)temporaryBytesArray);
    transmitNumberToGSM(temporaryBytesArray,temp);
    transmitStringToGSM("\r");
    __delay_ms(500);

    //Publish

    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    isERROR = false;
    do {
        transmitStringToGSM(deviceId);
        transmitStringToGSM("/notification");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In GPRS ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From GPRS", 3);
#endif
        __delay_ms(2000);
    }
    
    //Set payload

    transmitStringToGSM("\r\nAT+CMQTTPAYLOAD=0,");
    //transmitStringToGSM(number);
    if (strncmp(alert,"null",4) == 0) {
        stringLength = strlen((const char *)message);
    } else {
        stringLength = 24 + strlen((const char *)message) + strlen((const char *)alert);
        if (isnum) {
            stringLength = stringLength + strlen((const char *)fieldByte);
        }
    }
    clearTempBytesString();
    sprintf(temporaryBytesArray, "%d", stringLength);
    temp = strlen((const char *)temporaryBytesArray);
    transmitNumberToGSM(temporaryBytesArray,temp);
    transmitStringToGSM("\r");
    __delay_ms(500);
        
    //Publish message
 
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    isERROR = false;
    do {
        if (strncmp(alert,"null",4) == 0) {
            transmitStringToGSM(message);
        } else {
            transmitStringToGSM("{\"title\": \"");
            transmitStringToGSM(alert);
            transmitStringToGSM("\",\"body\": \""); 
            transmitStringToGSM(message);
            if (isnum) {
                temp = strlen((const char *)fieldByte);
                transmitNumberToGSM(fieldByte,temp);
            }
            transmitStringToGSM("\"}");
        }
        //transmitStringToGSM("\r\n");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In GPRS ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From GPRS", 3);
#endif
        __delay_ms(2000);
    }
 
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    isERROR = false;
    do {
        transmitStringToGSM("\r\nAT+CMQTTPUB=0,1,60\r\n");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In GPRS ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From GPRS", 3);
#endif
        __delay_ms(2000);
    }    
}

/*************************************************************************************************************************

This function is called to publish response to subscribed command.

***************************************************************************************************************************/
void publishResponse(const char *message, _Bool isnum) {
    unsigned int stringLength = 0;
    char tempBytesArray[26] = ""; 
    strcpyCustom(tempBytesArray, message);
    //publish operations:
    //Set topic
    transmitStringToGSM("\r\nAT+CMQTTTOPIC=0,");
    stringLength = strlen((const char *)deviceId) + 9;
    clearTempBytesString();
    sprintf(temporaryBytesArray, "%d", stringLength);
    temp = strlen((const char *)temporaryBytesArray);
    transmitNumberToGSM(temporaryBytesArray,temp);
    transmitStringToGSM("\r");
    __delay_ms(500);

    //Publish

    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    isERROR = false;
    do {
        transmitStringToGSM(deviceId);
        transmitStringToGSM("/response");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In GPRS ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From GPRS", 3);
#endif
        __delay_ms(2000);
    }
    
    //Set payload
 
    transmitStringToGSM("\r\nAT+CMQTTPAYLOAD=0,");
    stringLength = strlen(tempBytesArray);
    clearTempBytesString();
    sprintf(temporaryBytesArray, "%d", stringLength);
    temp = strlen((const char *)temporaryBytesArray);
    transmitNumberToGSM(temporaryBytesArray,temp);
    transmitStringToGSM("\r");
    __delay_ms(500);
    
    //Publish message
 
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    isERROR = false;
    do {
        if (isnum) {
            transmitStringToGSM(tempBytesArray);
            temp = strlen((const char *)fieldByte);
            transmitNumberToGSM(fieldByte,temp);
        } else {
            transmitStringToGSM(tempBytesArray);
        }
        //transmitStringToGSM("\r\n");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In GPRS ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From GPRS", 3);
#endif
        __delay_ms(2000);
    }
    
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    isERROR = false;
    do {
        transmitStringToGSM("\r\nAT+CMQTTPUB=0,1,60\r\n");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In GPRS ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From GPRS", 3);
#endif
        __delay_ms(2000);
    }
}

/*************************************************************************************************************************

This function is called to enable receive mode of GSM module.
The purpose of this function is to transmit AT commands which enables Receive mode of GSM module in Text mode

**************************************************************************************************************************/
void configureGPRS(void) {
    unsigned int stringLength = 0;
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdClearLine(4);
    lcdWriteStringAtCenter("Connecting to ", 2);
    lcdWriteStringAtCenter("GPRS Network", 3);
#endif    
    __delay_ms(3000);
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdWriteStringAtCenter("SENDING ", 2);
    lcdWriteStringAtCenter("AT", 3);
#endif
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    //isERROR = false;
    do {
        transmitStringToGSM("ATE0\r\n"); // Echo off command
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In GPRS ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From GPRS", 3);
#endif
        __delay_ms(2000);
    }

#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdWriteStringAtCenter("SENDING ", 2);
    lcdWriteStringAtCenter("AT+CSQ", 3);
#endif
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    //isERROR = false;
    do {
        transmitStringToGSM("AT+CSQ\r\n");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In GPRS ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From GPRS", 3);
#endif
        __delay_ms(2000);
    }

#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdWriteStringAtCenter("SENDING ", 2);
    lcdWriteStringAtCenter("AT+CREG?", 3);
#endif
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    //isERROR = false;
    do {
        transmitStringToGSM("AT+CREG?\r\n");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In GPRS ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From GPRS", 3);
#endif
        __delay_ms(2000);
    }

#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdWriteStringAtCenter("SENDING ", 2);
    lcdWriteStringAtCenter("AT+CGREG?", 3);
#endif
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    //isERROR = false;
    do {
        transmitStringToGSM("AT+CGREG?\r\n");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In GPRS ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From GPRS", 3);
#endif
        __delay_ms(2000);
    }
    
 // Specify the parameter value of the PDP context corresponding to cid
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdWriteStringAtCenter("SENDING ", 2);
    lcdWriteStringAtCenter("AT+CGSOCKCONT=1...", 3);
#endif
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    //isERROR = false;
    do {
        transmitStringToGSM("AT+CGSOCKCONT=1,\"IP\",\"CMNET\"\r\n");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In GPRS ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From GPRS", 3);
#endif
        __delay_ms(2000);
    }
//ensure the first PDP context get a IP address
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdWriteStringAtCenter("SENDING ", 2);
    lcdWriteStringAtCenter("AT+CGPADDR", 3);
#endif
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    //isERROR = false;
    do {
        transmitStringToGSM("AT+CGPADDR\r\n");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In GPRS ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From GPRS", 3);
#endif
        __delay_ms(2000);
    }
    
    newSMSRcvd = false;
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdWriteStringAtCenter("GPRS Enabled", 2);   
#endif
}


/*************************************************************************************************************************

This function is called to enable receive mode of GSM module.
The purpose of this function is to transmit AT commands which enables Receive mode of GSM module in Text mode

**************************************************************************************************************************/
void configureMQTT(void) {
    unsigned int stringLength = 0;
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdClearLine(4);
    lcdWriteStringAtCenter("Enabling", 2);
    lcdWriteStringAtCenter("MQTT Service", 3);
#endif    
    __delay_ms(3000);
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdWriteStringAtCenter("SENDING ", 2);
    lcdWriteStringAtCenter("AT", 3);
#endif
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    //isERROR = false;
    do {
        transmitStringToGSM("ATE0\r\n"); // Echo off command
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In MQTT ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From MQTT", 3);
#endif
        __delay_ms(2000);
    }
    
    //LTE Mode
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdWriteStringAtCenter("SENDING ", 2);
    lcdWriteStringAtCenter("AT+CNMP=38", 3);
#endif
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    //isERROR = false;
    do {
        transmitStringToGSM("AT+CNMP=38\r\n");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In MQTT ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From MQTT", 3);
#endif
        __delay_ms(2000);
    }

    //Start MQTT service
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdWriteStringAtCenter("SENDING ", 2);
    lcdWriteStringAtCenter("AT+CMQTTSTART", 3);
#endif
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    //isERROR = false;
    do {
        transmitStringToGSM("AT+CMQTTSTART\r\n");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In MQTT ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From MQTT", 3);
#endif
        __delay_ms(2000);
    }
    
    //Acquire client
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdWriteStringAtCenter("SENDING ", 2);
    lcdWriteStringAtCenter("AT+CMQTTACCQ...", 3);
#endif
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    //isERROR = false;
    do {
        transmitStringToGSM("AT+CMQTTACCQ=0,\"ESP32\"\r\n");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In MQTT ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From MQTT", 3);
#endif
        __delay_ms(2000);
    }
    
    //Enable SSL/TLS Support
    /*
    #ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdWriteStringAtCenter("SENDING ", 2);
    lcdWriteStringAtCenter("AT+CMQTTSSLCFG=0,1", 3);
#endif
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    //isERROR = false;
    do {
        transmitStringToGSM("AT+CMQTTSSLCFG=0,1\r\n");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In MQTT ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From MQTT", 3);
#endif
        __delay_ms(2000);
    }
    //Initialize SSL Parameters
    //This tells the A7670SA to use TLS 1.2 (sslversion 3) with strong encryption (0XFFFF).
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    //isERROR = false;
    do {
        transmitStringToGSM("AT+CSSLCFG=\"sslversion\",0,3\r\n");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In MQTT ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From MQTT", 3);
#endif
        __delay_ms(2000);
    }
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    //isERROR = false;
    do {
        transmitStringToGSM("AT+CSSLCFG=\"ciphersuite\",0,\"0XFFFF\"\r\n");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In MQTT ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From MQTT", 3);
#endif
        __delay_ms(2000);
    }
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    //isERROR = false;
    do {
        transmitStringToGSM("AT+CSSLCFG=\"seclevel\",0,2\r\n");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In MQTT ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From MQTT", 3);
#endif
        __delay_ms(2000);
    }
    //Upload the CA Certificate
    
    //Option A: Using AT+FSWRITE
    //The file mqtt_ca.pem must be placed in the module's filesystem using UART.
    // Then send the actual PEM data byte by byte or in chunks.
    transmitStringToGSM("AT+FSWRITE=\"mqtt_ca.pem\",0,1436,10");
    transmitStringToGSM(mqtt_ca);

    //Bind the Certificate to SSL Context
    transmitStringToGSM("AT+CSSLCFG=\"cacert\",0,\"mqtt_ca.pem\"");

    */
    //Connect to broker
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdWriteStringAtCenter("SENDING ", 2);
    lcdWriteStringAtCenter("AT+CMQTTCONNECT=0...", 3);
#endif
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    //isERROR = false;
    do {
        transmitStringToGSM("AT+CMQTTCONNECT=0,\"tcp://mqtt.bhoomijalasandharan.com:1883\",64800,1,\"bjuUser\",\"Boomi1moob\"\r\n");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In MQTT ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From MQTT", 3);
#endif
        __delay_ms(2000);
    }
    
    //subscribe operations:
    //Set topic
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdWriteStringAtCenter("SENDING ", 2);
    lcdWriteStringAtCenter("AT+CMQTTSUB...", 3);
    transmitStringToGSM("AT+CMQTTSUB=0,20,1\r"); 
#endif
    __delay_ms(500);
    //Subscribe
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdWriteStringAtCenter("SENDING ", 2);
    lcdWriteStringAtCenter("SUBSCRIBE", 3);
#endif
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    //isERROR = false;
    do { 
        transmitStringToGSM(deviceId);
        transmitStringToGSM("/command");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In MQTT ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From MQTT", 3);
#endif
        __delay_ms(2000);
    }
    transmitStringToGSM("\r\n");
    newSMSRcvd = false;
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdWriteStringAtCenter("Successfully", 2);
    lcdWriteStringAtCenter("Subscribed", 3);    
#endif
}

/*************************************************************************************************************************

This function is called to check connection between C and GSM  
The purpose of this function is to reset GSM until GSM responds OK to AT command.

 **************************************************************************************************************************/
/*
void checkGsmConnection(void) {
    timer3Count = 15;  // 15 sec window
    //setBCDdigit(0x0A,0);  // (c.) BCD indication for checkGsmConnection
    controllerCommandExecuted = false;
    msgIndex = 1;
    T3CONbits.TMR3ON = ON; // Start timer thread to unlock system if GSM fails to respond within 15 sec
    while (!controllerCommandExecuted) {
        transmitStringToGSM("AT\r\n"); // Echo ON command
        __delay_ms(500);
    }
    if (T3CONbits.TMR3ON) {
        PIR5bits.TMR3IF = SET; //Stop timer thread
    }
    //setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
}
*/
//************Set GSM at local time standard across the globe***********//

/*************************************************************************************************************************

This function is called to set GSm at Local time zone 
The purpose of this function is to send AT commands to GSM in order to set it at Local time zone.

 **************************************************************************************************************************/

void setGsmToLocalTime(void) {
    gsmSetToLocalTime = false;
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdClearLine(4);
    lcdWriteStringAtCenter("Setting System", 2);
    lcdWriteStringAtCenter("To Local Time", 3);
#endif
    __delay_ms(2000);
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdWriteStringAtCenter("SENDING ", 2);
    lcdWriteStringAtCenter("AT", 3);
#endif
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    isERROR = false;
    do {
        transmitStringToGSM("ATE0\r\n"); // Echo off command
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In GPRS ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From GPRS", 3);
#endif
        __delay_ms(2000);
    }
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdWriteStringAtCenter("SENDING ", 2);
    lcdWriteStringAtCenter("AT+COPS=2", 3);
#endif    
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    //isERROR = false;
    do {
        transmitStringToGSM("AT+COPS=2\r\n");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In GPRS ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From GPRS", 3);
#endif
        __delay_ms(2000);
    }
    __delay_ms(3000);
    __delay_ms(3000);
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdWriteStringAtCenter("SENDING ", 2);
    lcdWriteStringAtCenter("AT+CTZU=1", 3);
#endif
    msgIndex = 1;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    //isERROR = false;
    do {
        transmitStringToGSM("AT+CTZU=1\r\n");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In GPRS ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From GPRS", 3);
#endif
        __delay_ms(2000);
    }
    __delay_ms(3000);
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdWriteStringAtCenter("SENDING ", 2);
    lcdWriteStringAtCenter("AT+COPS=0", 3);
#endif
    msgIndex = 1;
    controllerCommandExecuted = false; 
    loraAttempt = 0;
    isOK = false;
    //isERROR = false;
    do {
        transmitStringToGSM("AT+COPS=0\r\n");
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 3 && !isOK);
    controllerCommandExecuted = true;
    if (isOK) {
    } else if (isERROR) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("ERROR In GPRS ", 2);
        lcdWriteStringAtCenter("Connection", 3);
#endif
        __delay_ms(2000);
    } else {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdWriteStringAtCenter("No Response ", 2);
        lcdWriteStringAtCenter("From GPRS", 3);
#endif
        __delay_ms(2000);
    }
    __delay_ms(3000);
    __delay_ms(3000);
    gsmSetToLocalTime = true;
}

/*************************************************************************************************************************

This function is called to delete messages from SIM storage
The purpose of this function is to send AT commands to GSM in order delete messages from SIM storage for receiving new messages in future

 **************************************************************************************************************************/
void deleteMsgFromSIMStorage(void) {
    timer3Count = 30;  // 30 sec window
    //setBCDdigit(0x09,1);  // (9) BCD indication Delete SMS action
    //controllerCommandExecuted = false;
    msgIndex = 1;
    T3CONbits.TMR3ON = ON; // Start timer thread to unlock system if GSM fails to respond within 15 sec
    while (!controllerCommandExecuted) {        
        transmitStringToGSM("AT+CMGD=1,4\r\n"); // delete message from ALL location
        __delay_ms(500);
    }
    if (T3CONbits.TMR3ON) {
        PIR5bits.TMR3IF = SET; //Stop timer thread
    }
    //setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    // ADD indication if infinite
}


/*************************************************************************************************************************

This function is called to reset GSM Module
The purpose of this function is to reset GSM module on ERROR.

 **************************************************************************************************************************/
/*
void resetGSM(void)
{
    gsmReboot = LOW;
    __delay_ms(1000);
    gsmReboot = HIGH;  
}
*/

/*************************************************************************************************************************

This function is called to Check GSM signal strength 
The purpose of this function is to send AT commands to GSM in order to get signal strength.

**************************************************************************************************************************/

void checkSignalStrength(void) {
	unsigned char digit = 0;
    while (1) {
        //setBCDdigit(0x0F,1); // BCD Indication for Flash
        __delay_ms(1000);
        digit = 0;
        timer3Count = 30;  // 30 sec window
        //setBCDdigit(0x0A,1);  // (c) BCD indication for checkSignalStrength Action
        //controllerCommandExecuted = false;
        msgIndex = CLEAR;
        transmitStringToGSM("AT+CSQ\r\n"); // To get signal strength
        T3CONbits.TMR3ON = ON; // Start timer thread to unlock system if GSM fails to respond within 15 sec
        while (!controllerCommandExecuted);
        if (T3CONbits.TMR3ON) {
            PIR5bits.TMR3IF = SET; //Stop timer thread
        }
        for(msgIndex = 6;  gsmResponse[msgIndex] != ',' ; msgIndex++)  
        {
            if(isNumber(gsmResponse[msgIndex])) //is number
            {	
                if(gsmResponse[msgIndex+1] == ',')
                {	
                    gsmResponse[msgIndex] = gsmResponse[msgIndex]-48;
                    digit = digit+gsmResponse[msgIndex];
                }
                else
                {
                    gsmResponse[msgIndex] = gsmResponse[msgIndex]-48;
                    gsmResponse[msgIndex] = gsmResponse[msgIndex]*10;
                    digit = digit*10;
                    digit = digit+gsmResponse[msgIndex];
                    }       
            }
        }  
        __delay_ms(1000);
        sprintf(temporaryBytesArray,"%d",digit);
        if(digit <= 5) //Poor Signal
        {   
#ifdef LCD_DISPLAY_ON_H
            lcdClear();
            lcdWriteStringAtCenter("Poor",2);
            lcdWriteStringAtCenter("GSM Signal",3);
            lcdWriteStringAtCenter((const char *)temporaryBytesArray,4);
#endif
            __delay_ms(3000);
            __delay_ms(3000);
            __delay_ms(3000);
        }
        else if(digit >= 6 && digit <= 9) //Very Low Signal
        { 
#ifdef LCD_DISPLAY_ON_H
            lcdClear();
            lcdWriteStringAtCenter("Very Low",2);
            lcdWriteStringAtCenter("GSM Signal",3);
            lcdWriteStringAtCenter((const char *)temporaryBytesArray,4);
#endif
            __delay_ms(3000);
            __delay_ms(3000);
            __delay_ms(3000);
        }
        else if(digit >= 10&&digit <= 13) //Low Signal
        { 
#ifdef LCD_DISPLAY_ON_H
            lcdClear();
            lcdWriteStringAtCenter("LOW",2);
            lcdWriteStringAtCenter("GSM Signal",3);
            lcdWriteStringAtCenter((const char *)temporaryBytesArray,4);
#endif
            __delay_ms(3000);
            __delay_ms(3000);
            __delay_ms(3000);
        }
        else if(digit >= 14&&digit <= 17) //Moderate Signal
        { 
#ifdef LCD_DISPLAY_ON_H
            lcdClear();
            lcdWriteStringAtCenter("Moderate",2);
            lcdWriteStringAtCenter("GSM Signal",3);
            lcdWriteStringAtCenter((const char *)temporaryBytesArray,4);
#endif
            __delay_ms(3000);
            __delay_ms(3000);
            __delay_ms(3000);
        }
        else if(digit >= 18 && digit <= 21) //Good Signal
        { 
#ifdef LCD_DISPLAY_ON_H
            lcdClear();
            lcdWriteStringAtCenter("Good",2);
            lcdWriteStringAtCenter("GSM Signal",3);
            lcdWriteStringAtCenter((const char *)temporaryBytesArray,4);
#endif
            __delay_ms(3000);
            __delay_ms(3000);
            __delay_ms(3000);
        }
        else if(digit >= 22&& digit <= 25) //very good Signal
        { 
#ifdef LCD_DISPLAY_ON_H
            lcdClear();
            lcdWriteStringAtCenter("Very Good",2);
            lcdWriteStringAtCenter("GSM Signal",3);
            lcdWriteStringAtCenter((const char *)temporaryBytesArray,4);
#endif
            __delay_ms(3000);
            __delay_ms(3000);
            __delay_ms(3000);
        }
        else if(digit >= 26 && digit <= 31) //Excellent Signal
        { 
#ifdef LCD_DISPLAY_ON_H
            lcdClear();
            lcdWriteStringAtCenter("Excellent",2);
            lcdWriteStringAtCenter("GSM Signal",3);
            lcdWriteStringAtCenter((const char *)temporaryBytesArray,4);
#endif
            __delay_ms(3000);
            __delay_ms(3000);
            __delay_ms(3000);
        }
        else if(digit == 99) // not known or not detectable
        { 
#ifdef LCD_DISPLAY_ON_H
            lcdClear();
            lcdWriteStringAtCenter("NO",2);
            lcdWriteStringAtCenter("GSM Signal",3);
            lcdWriteStringAtCenter((const char *)temporaryBytesArray,4);
#endif
            __delay_ms(3000);
            __delay_ms(3000);
            __delay_ms(3000);
        }
        else {
#ifdef LCD_DISPLAY_ON_H
            lcdClear();
            lcdWriteStringAtCenter("Error In",2);
            lcdWriteStringAtCenter("GSM Signal",3);
            lcdWriteStringAtCenter((const char *)temporaryBytesArray,4);
#endif
            __delay_ms(3000);
            __delay_ms(3000);
            __delay_ms(3000);
        }
    }															 
}
//*****************Serial communication functions_End****************//
