/*
 * File name            : controllerActions.c
 * Compiler             : MPLAB XC8/ MPLAB C18 compiler
 * IDE                  : Microchip  MPLAB X IDE v5.25
 * Processor            : PIC18F66K40
 * Author               : Bhoomi Jalasandharan
 * Created on           : July 15, 2020, 05:23 PM
 * Description          : Controller general functions
 */

#include "congfigBits.h"
#include "variableDefinitions.h"
#include "ADC.h"
#include "controllerActions.h"
#include "eeprom.h"
#include "gprs.h"
#include "lora.h"
#include "i2c.h"
#include "i2c_RTC_DS1307.h"
#include "i2c_LCD_PCF8574.h"
#ifdef Encryption_ON_H
#include "dataEncryption.h"
#endif
#ifdef DEBUG_MODE_ON_H
#include "serialMonitor.h"
#endif
/************************general purpose functions_start*******************************/

/*************************************************************************************************************************

This function is called to copy string from source to destination
The purpose of this function is to copy string till it finds first \n character in source.

 **************************************************************************************************************************/

char *strcpyCustom(char *restrict dest, const char *restrict src) {
    const char *s = src;
    char *d = dest;
    while ((*d++ = *s++))
        if (*s == '\n' || *s == '\r' || *s == '\0')
            break;
    return dest;
}

/*************************************************************************************************************************

This function is called to check if character is Numeric.
The purpose of this function is to check if the given character is in range of numeric ascii code

 **************************************************************************************************************************/
_Bool isNumber(unsigned char character) {
    if (character >= 48 && character <= 57) {
        return true;
    } else
        return false;
}

#ifdef Encryption_ON_H

/*************************************************************************************************************************

This function is called to check if string is Base64 encoded
The purpose of this function is to check if string has space or = or multiple of 4.

 **************************************************************************************************************************/
_Bool isBase64String(unsigned char * string) {
    //unsigned int stringLength;
    unsigned char * s = string;
    while (*s++ != '\0') {
        if (*s == space) {
            return false;
        }
    }
    return true;
}
#endif

/************************Get Current Clock Time From GSM#Start************************************/

/*************************************************************************************************************************

This function is called to get current local time
The purpose of this function is to receive local time stamp from GSM module

 **************************************************************************************************************************/
void getDateFromGSM(void) {   
    unsigned char index = 0;
    //timer3Count = 30; // 30 sec window
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("getDateFromGSM_IN\r\n");
    //********Debug log#end**************//
#endif
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdWriteStringAtCenter("Fetching ", 2);
    lcdWriteStringAtCenter("Server Time ", 3);
#endif
    msgIndex = CLEAR;
    controllerCommandExecuted = false;
    loraAttempt = 0;
    isOK = false;
    isERROR = false;
    do {
        transmitStringToGSM("AT+CCLK?\r\n"); // To get local time stamp  +CCLK: "18/05/26,12:00:06+22"   ok
        __delay_ms(1000);
        loraAttempt++;
    } while (loraAttempt < 5 && !isOK);
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
    // ADD indication if infinite
    tensDigit = CLEAR;
    unitsDigit = CLEAR;
    currentYY = CLEAR;
    currentMM = CLEAR;
    currentDD = CLEAR;
    currentHour = CLEAR;
    currentMinutes = CLEAR;
    currentSeconds = CLEAR;
///*
    lcdSetCursor(4,1);
    lcdWriteStringIndex(gsmResponse+8,17);
     __delay_ms(2500);
//*/
    // To check no garbage value received for date time command
    for (index = 8; index < 24; index += 3) {
        if (isNumber(gsmResponse[index]) && isNumber(gsmResponse[index + 1])) {
            controllerCommandExecuted = true;
        } else {
            controllerCommandExecuted = false;
            break;
        }
    }

    if (!controllerCommandExecuted) {
        controllerCommandExecuted = true;
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);
        lcdWriteStringAtCenter("Incorrect Date ", 2);
        __delay_ms(2500);
        __delay_ms(2500);
        /***************************/
        publishNotification("Time Alert",NotRTC6_36,false); // Acknowledge user about successful Authentication
        /***************************/
        //goto reexecute;
        // set indication of reset due to incorrect time stamp
    }
    else {

        temporaryBytesArray[5] = ':';
        tensDigit = gsmResponse[8] - 48;
        temporaryBytesArray[6] = gsmResponse[8];
        tensDigit = tensDigit * 10;
        unitsDigit = gsmResponse[9] - 48;
        temporaryBytesArray[7] = gsmResponse[9];
        currentYY = tensDigit + unitsDigit; // Store year in decimal
        
        temporaryBytesArray[2] = ':';
        tensDigit = gsmResponse[11] - 48;
        temporaryBytesArray[3] = gsmResponse[11];
        tensDigit = tensDigit * 10;
        unitsDigit = gsmResponse[12] - 48;
        temporaryBytesArray[4] = gsmResponse[12];
        currentMM = tensDigit + unitsDigit; // Store month in decimal

        tensDigit = gsmResponse[14] - 48;
        temporaryBytesArray[0] = gsmResponse[14];
        tensDigit = tensDigit * 10;
        unitsDigit = gsmResponse[15] - 48;
        temporaryBytesArray[1] = gsmResponse[15];
        currentDD = tensDigit + unitsDigit; // Store day in decimal

        temporaryBytesArray[8] = ' ';
        tensDigit = gsmResponse[17] - 48;
        temporaryBytesArray[9] = gsmResponse[17];
        tensDigit = tensDigit * 10;
        unitsDigit = gsmResponse[18] - 48;
        temporaryBytesArray[10] = gsmResponse[18];
        currentHour = tensDigit + unitsDigit; // Store hour in decimal

        temporaryBytesArray[11] = ':';
        tensDigit = gsmResponse[20] - 48;
        temporaryBytesArray[12] = gsmResponse[20];
        tensDigit = tensDigit * 10;
        unitsDigit = gsmResponse[21] - 48;
        temporaryBytesArray[13] = gsmResponse[21];
        currentMinutes = tensDigit + unitsDigit; // Store minutes in decimal

        temporaryBytesArray[14] = ':';
        tensDigit = gsmResponse[23] - 48;
        temporaryBytesArray[15] = gsmResponse[23];
        tensDigit = tensDigit * 10;
        unitsDigit = gsmResponse[24] - 48;
        temporaryBytesArray[16] = gsmResponse[24];
        currentSeconds = tensDigit + unitsDigit; // Store minutes in decimal
        
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);
        lcdWriteStringAtCenter("Current ", 2);
        lcdWriteStringAtCenter("Server Time ", 3);
        lcdSetCursor(4,2);
        lcdWriteStringIndex(temporaryBytesArray,17);
        __delay_ms(2500);
        __delay_ms(2500);
        __delay_ms(2500);
        __delay_ms(2500);
    }
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("getDateFromGSM_OUT\r\n");
    //********Debug log#end**************//
#endif
}
/************************Get Current Clock Time From GSM#End************************************/


/************************Calculate Next Due Dates for Valve Action#Start************************************/

/*************************************************************************************************************************

This function is called to get due date
The purpose of this function is to calculate the future date with given days from current date

 **************************************************************************************************************************/
void getDueDate(unsigned char days) {
    unsigned int remDays = CLEAR, offset = CLEAR, leapYearDays = 366, yearDays = 365;
    unsigned char firstMonth = 1, lastMonth = 12, month[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("getDueDate_IN\r\n");
    //********Debug log#end**************//
#endif
    dueDD = CLEAR, dueMM = CLEAR, dueYY = CLEAR;
    __delay_ms(100);
    fetchTimefromRTC();
    __delay_ms(100);
    dueDD = currentDD; // get todays day and set as temporary  dueDD
    switch (currentMM - 1) {
        case 11:
            dueDD += 30;
        case 10:
            dueDD += 31;
        case 9:
            dueDD += 30;
        case 8:
            dueDD += 31;
        case 7:
            dueDD += 31;
        case 6:
            dueDD += 30;
        case 5:
            dueDD += 31;
        case 4:
            dueDD += 30;
        case 3:
            dueDD += 31;
        case 2:
            dueDD += 28;
        case 1:
            dueDD += 31;
    }
    // leap year and greater than February
    if ((((2000 + (unsigned int) currentYY) % 100 != 0 && currentYY % 4 == 0) || (2000 + (unsigned int) currentYY) % 400 == 0) && currentMM > 2) {
        dueDD += 1;
    }
    //leap year
    if (((2000 + (unsigned int) currentYY) % 100 != 0 && currentYY % 4 == 0) || (2000 + (unsigned int) currentYY) % 400 == 0) {
        remDays = leapYearDays - dueDD;
    } else {
        remDays = yearDays - dueDD;
    }
    if (days <= remDays) {
        dueYY = currentYY;
        dueDD += days;
    } else {
        days -= remDays;
        dueYY = currentYY + 1;
        //leap year
        if (((2000 + (unsigned int) dueYY) % 100 != 0 && dueYY % 4 == 0) || (2000 + (unsigned int) dueYY) % 400 == 0) {
            offset = leapYearDays;
        } else {
            offset = yearDays;
        }
        while (days >= offset) {
            days -= offset;
            dueYY++;
            //leap year
            if (((2000 + (unsigned int) dueYY) % 100 != 0 && dueYY % 4 == 0) || (2000 + (unsigned int) dueYY) % 400 == 0) {
                offset = leapYearDays;
            } else {
                offset = yearDays;
            }
        }
        dueDD = days;
    }
    //leap year
    if (((2000 + (unsigned int) currentYY) % 100 != 0 && currentYY % 4 == 0) || (2000 + (unsigned int) currentYY) % 400 == 0) {
        month[2] = 29;
    }
    for (dueMM = firstMonth; dueMM <= lastMonth; dueMM++) {
        if (dueDD <= month[dueMM])
            break;
        dueDD = dueDD - month[dueMM];
    }
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("getDueDate_OUT\r\n");
    //********Debug log#end**************//
#endif
}
/************************Calculate Next Due Dates for Valve Action#End************************************/


/************************SleepCount for Next Valve Action#Start************************************/

/*************************************************************************************************************************

This function is called to get sleep count
The purpose of this function is to check if configured valve is due then calculate sleep count for on period else calculate sleep count upto the nearest configured valve setting.

 **************************************************************************************************************************/
void scanValveScheduleAndGetSleepCount(void) {
    unsigned long newCount = CLEAR; // Used to save temporary calculated sleep count
    unsigned int leapYearDays = 366, yearDays = 365;
    unsigned char iLocal = CLEAR;
    _Bool fieldCylceChecked = false;
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("scanValveScheduleAndGetSleepCount_IN\r\n");
    //********Debug log#end**************//
#endif
    sleepCount = 65500; // Set Sleep count to default value until it is calculated
    currentDateCalled = false;
    if (startFieldNo > fieldCount - 1) { // reset starting point after iterating through all valves
        startFieldNo = 0;
    }
nxtCycle:
    for (iterator_field = startFieldNo; iterator_field < fieldCount; iterator_field++) {
        // do if Configured valve is not in action
        if (fieldValve[iterator_field].isConfigured && fieldValve[iterator_field].status != ON) {
            //get current date only for one iteration
            if (!currentDateCalled) {
                __delay_ms(100);
                fetchTimefromRTC(); // Get today's date
                __delay_ms(100);
                currentDateCalled = true; // Today's date is known
                sleepCount = 65500; // Set Sleep count to default value until it is calculated
            }
            /*** Due date is over passed without taking action on valves ***/
            // if year over passes || if month  over passes || if day over passes || if hour over passes ||if minute over passes
            if ((currentYY > fieldValve[iterator_field].nextDueYY) || 
                    (currentMM > fieldValve[iterator_field].nextDueMM && 
                    currentYY == fieldValve[iterator_field].nextDueYY) || 
                    (currentDD > fieldValve[iterator_field].nextDueDD && 
                    currentMM == fieldValve[iterator_field].nextDueMM && 
                    currentYY == fieldValve[iterator_field].nextDueYY) || 
                    (currentHour > fieldValve[iterator_field].motorOnTimeHour && 
                    currentDD == fieldValve[iterator_field].nextDueDD && 
                    currentMM == fieldValve[iterator_field].nextDueMM && 
                    currentYY == fieldValve[iterator_field].nextDueYY) || 
                    (currentMinutes >= fieldValve[iterator_field].motorOnTimeMinute && 
                    currentHour == fieldValve[iterator_field].motorOnTimeHour && 
                    currentDD == fieldValve[iterator_field].nextDueDD && 
                    currentMM == fieldValve[iterator_field].nextDueMM && 
                    currentYY == fieldValve[iterator_field].nextDueYY)) {
                valveDue = true; // Set Valve Due
                break;
            } else if (fieldValve[iterator_field].cyclesExecuted < fieldValve[iterator_field].cycles) {
                valveDue = true; // Set Valve Due
                break;
            }// Due Date is yet to come find the sleep count to reach the Due date	
            else {
                valveDue = false; // All due valves are operated
                newCount = CLEAR; // clear initial temporary calculated sleep count

                /*** temporary sleep count between today's date and valve's next due date ***/

                for (iLocal = currentYY; iLocal < fieldValve[iterator_field].nextDueYY; iLocal++) {
                    if ((2000 + (unsigned int) iLocal) % 100 != 0 && iLocal % 4 == 0 && (2000 + (unsigned int) iLocal) % 400 == 0)
                        newCount += leapYearDays;
                    else
                        newCount += yearDays;
                }
                newCount += days(fieldValve[iterator_field].nextDueMM, fieldValve[iterator_field].nextDueYY);
                newCount += fieldValve[iterator_field].nextDueDD;
                newCount -= days(currentMM, currentYY);
                newCount -= currentDD;
                newCount *= 24; // converting into no. of hours
                // Consider current hour in calculated sleep count
                if (fieldValve[iterator_field].motorOnTimeHour >= currentHour) {
                    newCount += (fieldValve[iterator_field].motorOnTimeHour - currentHour);
                    /****converting in minutes****/
                    newCount *= 60;
                    if (currentMinutes >= fieldValve[iterator_field].motorOnTimeMinute) {
                        newCount -= (currentMinutes - fieldValve[iterator_field].motorOnTimeMinute);
                    } else {
                        newCount += (fieldValve[iterator_field].motorOnTimeMinute - currentMinutes);
                    }
                }// Subtract current hour from calculated sleep count
                else if (fieldValve[iterator_field].motorOnTimeHour < currentHour) {
                    newCount -= (currentHour - fieldValve[iterator_field].motorOnTimeHour);
                    /****converting in minutes****/
                    newCount *= 60;
                    if (currentMinutes >= fieldValve[iterator_field].motorOnTimeMinute) {
                        newCount -= (currentMinutes - fieldValve[iterator_field].motorOnTimeMinute);
                    } else {
                        newCount += (fieldValve[iterator_field].motorOnTimeMinute - currentMinutes);
                    }
                }
                // Valve is due in a minute
                if (newCount == 0 || newCount == 1) {
                    sleepCount = 1; // calculate sleep count for upcoming due valve
                }// Save sleep count for nearest next valve action  
                else if (newCount < sleepCount) {
                    sleepCount = (unsigned int)newCount;                      // calculate sleep count for upcoming due valve
                    /*****To display next due date in lcd******/
                    dueDate[0] = (fieldValve[iterator_field].nextDueDD / 10) + 48;
                    dueDate[1] = (fieldValve[iterator_field].nextDueDD  % 10) + 48;
                    dueDate[2] = '/';
                    dueDate[3] = (fieldValve[iterator_field].nextDueMM  / 10) + 48;
                    dueDate[4] = (fieldValve[iterator_field].nextDueMM  % 10) + 48;
                    dueDate[5] = '/';
                    dueDate[6] = (fieldValve[iterator_field].nextDueYY  / 10) + 48;
                    dueDate[7] = (fieldValve[iterator_field].nextDueYY  % 10) + 48;
                    dueDate[8] = ' ';
                    dueDate[9] = (fieldValve[iterator_field].motorOnTimeHour  / 10) + 48;
                    dueDate[10] = (fieldValve[iterator_field].motorOnTimeHour  % 10) + 48;
                    dueDate[11] = ':';
                    dueDate[12] = (fieldValve[iterator_field].motorOnTimeMinute  / 10) + 48;
                    dueDate[13] = (fieldValve[iterator_field].motorOnTimeMinute  % 10) + 48;
                    dueDate[14] = '\0';
                }
            }
        }
    }
    if (!valveDue && !fieldCylceChecked) {
        fieldCylceChecked = true;
        startFieldNo = 0; // Reset start field no after scanning all irrigation valves from start field no.
        goto nxtCycle;
    }
    if (valveDue) {
        /* check Fertigation status and set sleep count to fertigation wet period*/
        if (fieldValve[iterator_field].isFertigationEnabled && fieldValve[iterator_field].fertigationInstance != 0) {
            sleepCount = fieldValve[iterator_field].fertigationDelay; // calculate sleep count for fertigation delay 
            fieldValve[iterator_field].fertigationStage = wetPeriod;
            saveFertigationValveStatusIntoEeprom(eepromAddress[iterator_field], &fieldValve[iterator_field]);
#ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("scanValveScheduleAndGetSleepCount_ValveDueWithFertigation_OUT\r\n");
            //********Debug log#end**************//
#endif
        } else { /*Only Irrigation valve*/
            sleepCount = fieldValve[iterator_field].onPeriod; // calculate sleep count for Valve on period 
#ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("scanValveScheduleAndGetSleepCount_ValveDueW/OFertigation_OUT\r\n");
            //********Debug log#end**************//
#endif   
        }
    }
    else {
        if (sleepCount > 1 && sleepCount < 4369) {
            sleepCount = sleepCount * 15;
            sleepCount = (sleepCount / 17);
        }
        else if (sleepCount >= 4369) {
            sleepCount = 4095;
        }
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("scanValveScheduleAndGetSleepCount_W/OValveDue_OUT\r\n");
        //********Debug log#end**************//
#endif
    }
}
/************************SleepCount for Next Valve Action#End************************************/


/************************Days Between Two Dates#Start************************************/

/*************************************************************************************************************************

This function is called to get no. of days.
The purpose of this function is to calculate no. of days left in the calender year from given month and year
It counts no.of days from 1st January of calender to 1st day of given month
 **************************************************************************************************************************/
unsigned int days(unsigned char mm, unsigned char yy) {
    unsigned char itr = CLEAR, month[12] = {31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    unsigned int days = CLEAR;
    for (itr = 0; itr < mm - 1; itr++) {
        if (itr == 1) {
            if ((2000 + (unsigned int) yy) % 100 != 0 && yy % 4 == 0 && (2000 + (unsigned int) yy) % 400 == 0)
                days += 29;
            else
                days += 28;
        } else
            days += month[itr];
    }
    return (days);
}
/************************Days Between Two Dates#End************************************/


/************************Extract Digits no. in msg#Start************************************/

/*************************************************************************************************************************

This function is called to fetch digit no mentioned in sms.
The purpose of this function is to navigate through received sms and fetch two digit no from given position 
It fetches two digit from given position
 **************************************************************************************************************************/
unsigned char fetchDigits(unsigned char index) {
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("fetchDigits_IN: ");
    //********Debug log#end**************//
#endif
    if (decodedString[index] == '0') {
        if (decodedString[index + 1] > '0' && decodedString[index + 1] <= '9' ) {
#ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("\r\nfetchDigits_OUT\r\n");
            //********Debug log#end**************//
#endif         
        }
        return (decodedString[index + 1] - 49);
    } else if (decodedString[index] == '1') {
        if (decodedString[index + 1] >= '0' && decodedString[index + 1] <= '9' ) {
#ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("\r\nfetchDigits_OUT\r\n");
            //********Debug log#end**************//
#endif         
        }
        return (decodedString[index + 1] - 39);
    }
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("\r\nfetchDigits_OUT\r\n");
        //********Debug log#end**************//
#endif
        return 255;
}
/************************Extract Digit no. in msg#End************************************/


/************************Actions on Received SMS#Start************************************/

/*************************************************************************************************************************

This function is called to extract received message after new message notification is received.
The purpose of this function is to read sms received at 1st SIM memory location.
The Action is decided upon Type of message received.

 **************************************************************************************************************************/
void extractReceivedSms(void) {
    unsigned char count = CLEAR, onHour = CLEAR, onMinute = CLEAR;
    unsigned int digit = CLEAR;
    timer3Count = 30; // 30 sec window
#ifdef LCD_DISPLAY_ON_H
    removeIcon(sms_icon);
#endif
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("extractReceivedSms_IN\r\n");
    //********Debug log#end**************//
#endif
  
    clearStringToDecode();
    /*Decode received  Base64 format message*/
#ifdef Encryption_ON_H
    //strcpyCustom((char *) stringToDecode, (const char *) gsmResponse + 1);
    strcpyCustom((char *) decodedString, (const char *) gsmResponse + 1);
    
#ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug((const char *) gsmResponse + 63);
            transmitStringToDebug("\r\n");
            transmitStringToDebug((char *) stringToDecode);
            transmitStringToDebug("\r\n");
            //********Debug log#end**************//
#endif
    clearGsmResponse();           
#endif
    //#-1>..............Device ID set up .................//
    if (!deviceIdSet) {
        if (strncmp(decodedString, admin, 2) == 0) {
            msgIndex = CLEAR;
            strncpy(deviceId, decodedString + 3, 12);
            saveDeviceIDIntoEeprom(); //store device id into eeprom
            deviceIdSet = true;
            __delay_ms(50);
            saveDeviceIdStatus();
    #ifdef LCD_DISPLAY_ON_H
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);
            lcdWriteStringAtCenter("Device ID", 2);
            lcdWriteStringAtCenter("Set", 3);
            lcdWriteStringAtCenter((const char *)deviceId, 4);
            __delay_ms(3000);
    #endif
    #ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("extractReceivedSms_DeviceID Set_OUT\r\n");
            //********Debug log#end**************// 
    #endif
            return;
        } else {
#ifdef LCD_DISPLAY_ON_H
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);
            lcdWriteStringAtCenter("Device ID", 2);
            lcdWriteStringAtCenter("Setting Failed", 3);
            __delay_ms(3000);
    #endif										
            msgIndex = CLEAR;
    #ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("extractReceivedSms_Device ID not Set_OUT\r\n");
            //********Debug log#end**************//
    #endif
            return;
        }
    } 
    //#0>..............Device Online Connection .................//
    if (strncmp(decodedString, admin, 2) == 0 || !systemAuthenticated) {
        msgIndex = CLEAR;
        if (strncmp(decodedString + 3, factryPswrd, 6) == 0) {
            systemAuthenticated = true;
            __delay_ms(50);
            saveAuthenticationStatus();
#ifdef LCD_DISPLAY_ON_H
            lcdClearLine(1);
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);
            lcdWriteStringAtCenter("System", 2);
            lcdWriteStringAtCenter("Authenticated", 3);
#endif		
            /***************************/
            publishResponse(cmd1_6,false); // Acknowledge user about successful Authentication
            /***************************/
            /***************************/                  
            //sendSms(SmsPwd1, userMobileNo, noInfo);
#ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            //setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            //setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
#endif
            /***************************/
        #ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("extractReceivedSms_Password updated_OUT\r\n");
            //********Debug log#end**************// 
        #endif
            return;
        } else {
#ifdef LCD_DISPLAY_ON_H
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);
            lcdWriteStringAtCenter("System", 2);
            lcdWriteStringAtCenter("Not Authenticated", 3);
#endif										
            msgIndex = CLEAR;
            /***************************/
            publishResponse(cmd2_9,false); // Acknowledge user about unsuccessful Authentication
            /***************************/
            /***************************/
            //sendSms(SmsPwd3, userMobileNo, noInfo);
#ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            //setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            //setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
#endif
            /***************************/ 
#ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("extractReceivedSms_Password not changed_OUT\r\n");
            //********Debug log#end**************//
#endif
            return;
        }
    }
    //#1>..............Device Online Connection .................//
    if (strncmp(decodedString, alive1, 5) == 0) {
        msgIndex = CLEAR;
        /***************************/
        publishResponse(cmd3_9,false); // Acknowledge user about successful Irrigation configuration
        /***************************/
#ifdef LCD_DISPLAY_ON_H        
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);
        lcdWriteStringAtCenter("ALIVE", 2);
#endif
        /***************************/
#ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        //setBCDdigit(0x05, 0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
        /***************************/
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_Configure_OUT\r\n");
        //********Debug log#end**************//
#endif
        return;
    }//#2>..............Save New Configured values of Field Valve X .................//
    else if (strncmp(decodedString, set, 3) == 0) {
        iterator_field = fetchDigits(3); // fetch field no. and save irrigation by field sequence                        
        digit = CLEAR;
        count = CLEAR; //count to extract onperiod, offperiod,motorOnHour,motorOnMinute,DryValue, wetValue; i.e total 9 attributes

        for (count = 1, msgIndex = 6; count <= 9; msgIndex++) {
            //is number
            if (isNumber(decodedString[msgIndex])) {
                if (decodedString[msgIndex + 1] == space || decodedString[msgIndex + 1] == '#' ) {
                    decodedString[msgIndex] = decodedString[msgIndex] - 48;
                    digit = digit + decodedString[msgIndex];
                } else {
                    decodedString[msgIndex] = decodedString[msgIndex] - 48;
                    decodedString[msgIndex] = decodedString[msgIndex] * 10;
                    digit = digit * 10;
                    digit = digit + decodedString[msgIndex];
                }
            } else {
                switch (count) {
                    case 1: // code to extract on period;
                        fieldValve[iterator_field].onPeriod = digit;
                        digit = CLEAR;
                        break;
                    case 2: // code to extract off period;
                        fieldValve[iterator_field].offPeriod = (unsigned char) digit;
                        digit = CLEAR;
                        break;
                    case 3: // code to extract motorOnTimeHour;
                        fieldValve[iterator_field].motorOnTimeHour = (unsigned char) digit;
                        digit = CLEAR;
                        break;
                    case 4: // code to extract motorOnTimeMinute;
                        fieldValve[iterator_field].motorOnTimeMinute = (unsigned char) digit;
                        digit = CLEAR;
                        break;
                    case 5: // code to extract dryValue
                        fieldValve[iterator_field].dryValue = digit;
                        digit = CLEAR;
                        break;
                    case 6: // code to extract wetValue
                        fieldValve[iterator_field].wetValue = digit;
                        digit = CLEAR;
                        break;
                    case 7: // code to extract priority
                        fieldValve[iterator_field].priority = (unsigned char) digit;
                        digit = CLEAR;
                        break;
                    case 8: // code to extract cycles
                        fieldValve[iterator_field].cycles = (unsigned char) digit;
                        fieldValve[iterator_field].cyclesExecuted = (unsigned char) digit;
                        digit = CLEAR;
                        break;
                    case 9: // code to extract day count;
                        getDueDate((unsigned char) digit); // Get due dates w.r.t triggered from date
                        digit = CLEAR;
                        fieldValve[iterator_field].nextDueDD = (unsigned char) dueDD;
                        fieldValve[iterator_field].nextDueMM = dueMM;
                        fieldValve[iterator_field].nextDueYY = dueYY;
                        fieldValve[iterator_field].status = OFF;
                        fieldValve[iterator_field].isConfigured = true;
                        fieldValve[iterator_field].fertigationDelay = 0;
                        fieldValve[iterator_field].fertigationONperiod = 0;
                        fieldValve[iterator_field].fertigationInstance = 0;
                        fieldValve[iterator_field].isFertigationEnabled = false;
                        fieldValve[iterator_field].fertigationStage = OFF;
                        fieldValve[iterator_field].fertigationValveInterrupted = false;
                        break;
                }
                count++;
            }
        }
        __delay_ms(100);
        saveIrrigationValveValuesIntoEeprom(eepromAddress[iterator_field], &fieldValve[iterator_field]);
        __delay_ms(100);
        saveIrrigationValveDueTimeIntoEeprom(eepromAddress[iterator_field], &fieldValve[iterator_field]);
        __delay_ms(100);
        saveIrrigationValveOnOffStatusIntoEeprom(eepromAddress[iterator_field], &fieldValve[iterator_field]);
        __delay_ms(100);
        saveIrrigationValveCycleStatusIntoEeprom(eepromAddress[iterator_field], &fieldValve[iterator_field]);
        __delay_ms(100);
        saveIrrigationValveConfigurationStatusIntoEeprom(eepromAddress[iterator_field], &fieldValve[iterator_field]);
        __delay_ms(100);
        saveFertigationValveValuesIntoEeprom(eepromAddress[iterator_field], &fieldValve[iterator_field]);
        __delay_ms(100);
        msgIndex = CLEAR;
#ifdef LCD_DISPLAY_ON_H        
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);
        lcdWriteStringAtCenter("Irri.Data Configured", 2);
        lcdWriteStringAtCenter("For Priority :", 3);
        lcdSetCursor(3,17);
        clearFieldByte();
        sprintf(fieldByte,"%d",iterator_field+1);
        temp = strlen((const char *)fieldByte);
        lcdWriteStringIndex(fieldByte,temp);
#endif
        /***************************/
        publishResponse(cmd4_7,false); // Acknowledge user about successful Irrigation configuration
#ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        //setBCDdigit(0x05, 0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
        /***************************/
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_Configure_OUT\r\n");
        //********Debug log#end**************//
#endif
        return;
    }//#3>..............Hold Field x Irrigation.................//
    //Hold<x>
    else if (strncmp(decodedString, hold, 4) == 0) {
        iterator_field = fetchDigits(4); // fetch priority
        if (fieldValve[iterator_field].status == ON) { // hold active valve
            onHold = true; // execute onhold condition of sleepcount finish
        }
        fieldValve[iterator_field].isConfigured = false; // configuration hold
        if (fieldValve[iterator_field].isFertigationEnabled == true) {
            fieldValve[iterator_field].isFertigationEnabled = false; // configuration hold
            __delay_ms(100);
            saveFertigationValveValuesIntoEeprom(eepromAddress[iterator_field], &fieldValve[iterator_field]);
            __delay_ms(100);
        }
        saveIrrigationValveConfigurationStatusIntoEeprom(eepromAddress[iterator_field], &fieldValve[iterator_field]);
        __delay_ms(100);
        msgIndex = CLEAR;
#ifdef LCD_DISPLAY_ON_H        
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);
        lcdWriteStringAtCenter("Irri. Data Disabled", 2);
        lcdWriteStringAtCenter("For Priority :", 3);
        lcdSetCursor(3,17);
        clearFieldByte();
        sprintf(fieldByte,"%d",iterator_field+1);
        temp = strlen((const char *)fieldByte);
        lcdWriteStringIndex(fieldByte,temp);
#endif
        /***************************/
        publishResponse(cmd5_8,false); // Acknowledge user about successful Irrigation configuration disable action
#ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        //setBCDdigit(0x05, 0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
        /***************************/
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_Hold_OUT\r\n");
        //********Debug log#end**************//
#endif
        return;
    }//#4>.............. Enable Fertigation Valve for field.................//
    //Msg Format---------***Enable<FieldNo.><Space><Delay><Space><ONPeriod><Space><Instance><Space>***------------//
    //SMS Location---------63--------<69>------<71>-----------<73/74>---------<78/79/80>-------------//
    else if (strncmp(decodedString, enable, 6) == 0) {
        digit = CLEAR;
        count = CLEAR; //count to extract delayStart, onPeriod, no. of times
        iterator_field = fetchDigits(6);
        if (fieldValve[iterator_field].isConfigured == DISABLED) {
            msgIndex = CLEAR;
#ifdef LCD_DISPLAY_ON_H        
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);
            lcdWriteStringAtCenter("Irri. Not Configured", 2);
            lcdWriteStringAtCenter("Fertigation Disabled:", 3);
            lcdWriteStringAtCenter("For Priority No:", 4);
            lcdSetCursor(4,17);
            clearFieldByte();
            sprintf(fieldByte,"%d",iterator_field+1);
            temp = strlen((const char *)fieldByte);
            lcdWriteStringIndex(fieldByte,temp);
#endif                        
            /***************************/
            publishResponse(cmd8_13,false); // Acknowledge user about Fertigation not configured due to disabled irrigation
#ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            //setBCDdigit(0x05, 0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
            /***************************/
        } else {
            for (msgIndex = 9; count < 15; msgIndex++) {
                if (isNumber(decodedString[msgIndex])) {
                    if (decodedString[msgIndex + 1] == space || decodedString[msgIndex + 1] == '#') {
                        decodedString[msgIndex] = decodedString[msgIndex] - 48;
                        digit = digit + decodedString[msgIndex];
                    } else {
                        decodedString[msgIndex] = decodedString[msgIndex] - 48;
                        decodedString[msgIndex] = decodedString[msgIndex] * 10;
                        digit = digit * 10;
                        digit = digit + decodedString[msgIndex];
                    }
                } else {
                    count++;
                    switch (count) {
                        case 1: // code to extract fertigationDelay;
                            fieldValve[iterator_field].fertigationDelay = digit;
                            digit = CLEAR;
                            break;
                        case 2: // code to extract fertigationONperiod;
                            fieldValve[iterator_field].fertigationONperiod = digit;
                            digit = CLEAR;
                            break;
                        case 3: // code to extract fertigationInstance;
                            fieldValve[iterator_field].fertigationInstance = (unsigned char) digit;
                            digit = CLEAR;
                            break;
                        case 4: // code to extract injector1OnPeriod;
                            fieldValve[iterator_field].injector1OnPeriod = digit;
                            digit = CLEAR;
                            break;
                        case 5: // code to extract injector1OffPeriod;
                            fieldValve[iterator_field].injector1OffPeriod = digit;
                            digit = CLEAR;
                            break;
                        case 6: // code to extract injector1Cycle;
                            fieldValve[iterator_field].injector1Cycle = (unsigned char) digit;
                            digit = CLEAR;
                            break;
                        case 7: // code to extract injector2OnPeriod;
                            fieldValve[iterator_field].injector2OnPeriod = digit;
                            digit = CLEAR;
                            break;
                        case 8: // code to extract injector2OffPeriod;
                            fieldValve[iterator_field].injector2OffPeriod = digit;
                            digit = CLEAR;
                            break;
                        case 9: // code to extract injector2Cycle;
                            fieldValve[iterator_field].injector2Cycle = (unsigned char) digit;
                            digit = CLEAR;
                            break;
                        case 10: // code to extract injector3OnPeriod;
                            fieldValve[iterator_field].injector3OnPeriod = digit;
                            digit = CLEAR;
                            break;
                        case 11: // code to extract injector3OffPeriod;
                            fieldValve[iterator_field].injector3OffPeriod = digit;
                            digit = CLEAR;
                            break;
                        case 12: // code to extract injector3Cycle;
                            fieldValve[iterator_field].injector3Cycle = (unsigned char) digit;
                            digit = CLEAR;
                            break;
                        case 13: // code to extract injector4OnPeriod;
                            fieldValve[iterator_field].injector4OnPeriod = digit;
                            digit = CLEAR;
                            break;
                        case 14: // code to extract injector4OffPeriod;
                            fieldValve[iterator_field].injector4OffPeriod = digit;
                            digit = CLEAR;
                            break;
                        case 15: // code to extract injector1Cycle;
                            fieldValve[iterator_field].injector4Cycle = (unsigned char) digit;
                            fieldValve[iterator_field].fertigationStage = OFF;
                            fieldValve[iterator_field].fertigationValveInterrupted = false;
                            digit = CLEAR;
                            if ((fieldValve[iterator_field].fertigationDelay + 
                                    fieldValve[iterator_field].fertigationONperiod) 
                                    >= fieldValve[iterator_field].onPeriod) {
                                fieldValve[iterator_field].isFertigationEnabled = false;
                                msgIndex = CLEAR;
#ifdef LCD_DISPLAY_ON_H        
                                lcdClearLine(2);
                                lcdClearLine(3);
                                lcdClearLine(4);
                                lcdWriteStringAtCenter("Incorrect Data", 2);
                                lcdWriteStringAtCenter("Fertigation Disabled:", 3);
                                lcdWriteStringAtCenter("For Priority No:", 4);
                                lcdSetCursor(4,17);
                                clearFieldByte();
                                sprintf(fieldByte,"%d",iterator_field+1);
                                temp = strlen((const char *)fieldByte);
                                lcdWriteStringIndex(fieldByte,temp);
#endif
                                /***************************/
                                publishResponse(cmd7_17,false); // Acknowledge user about Fertigation not configured due to incorrect values
#ifdef SMS_DELIVERY_REPORT_ON_H
                                sleepCount = 2; // Load sleep count for SMS transmission action
                                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                                //setBCDdigit(0x05, 0);
                                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                                //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
                                /***************************/
                            } else {
                                fieldValve[iterator_field].isFertigationEnabled = true;
                            }
                            break;
                    }
                }
            }
            if (fieldValve[iterator_field].isFertigationEnabled) {
                __delay_ms(100);
                saveFertigationValveValuesIntoEeprom(eepromAddress[iterator_field], &fieldValve[iterator_field]);
                __delay_ms(100);
                msgIndex = CLEAR;
#ifdef LCD_DISPLAY_ON_H        
                lcdClearLine(2);
                lcdClearLine(3);
                lcdClearLine(4);
                lcdWriteStringAtCenter("Fert.Data Configured", 2);
                lcdWriteStringAtCenter("For Priority No:", 3);
                lcdSetCursor(3,17);
                clearFieldByte();
                sprintf(fieldByte,"%d",iterator_field+1);
                temp = strlen((const char *)fieldByte);
                lcdWriteStringIndex(fieldByte,temp);
#endif
                /***************************/
                publishResponse(cmd6_10,false); // Acknowledge user about successful Fertigation enabled action
#ifdef SMS_DELIVERY_REPORT_ON_H
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                //setBCDdigit(0x05, 0);
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
                /***************************/
            }
        }
#ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("extractReceivedSms_EnableFertigation_OUT\r\n");
            //********Debug log#end**************//
#endif
            return;
    }//#5>..............Disable Field x fertigation.................//
    // DISABLE<x>
    else if (strncmp(decodedString, disable, 7) == 0) {
        iterator_field = fetchDigits(7);
        if (fieldValve[iterator_field].status == ON && fieldValve[iterator_field].fertigationStage != flushPeriod) {
            onHold = true;
        }
        if (fieldValve[iterator_field].isFertigationEnabled == true) {
            fieldValve[iterator_field].isFertigationEnabled = false; // configuration hold
            __delay_ms(100);
            saveFertigationValveValuesIntoEeprom(eepromAddress[iterator_field], &fieldValve[iterator_field]);
            __delay_ms(100);
            msgIndex = CLEAR;
#ifdef LCD_DISPLAY_ON_H        
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);
            lcdWriteStringAtCenter("Fert. Data Disabled", 2);
            lcdWriteStringAtCenter("For Field No:", 3);
            lcdSetCursor(3,17);
            clearFieldByte();
            sprintf(fieldByte,"%d",iterator_field+1);
            temp = strlen((const char *)fieldByte);
            lcdWriteStringIndex(fieldByte,temp);
#endif
            /***************************/
            publishResponse(cmd9_11,false); // Acknowledge user about successful Fertigation disabled action
#ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            //setBCDdigit(0x05, 0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
            /***************************/
#ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("extractReceivedSms_Hold_OUT\r\n");
            //********Debug log#end**************//
#endif
            return;
        }
    }//#6>..............Activate Filtration.................//
        //Msg Format---------***ACTIVE<Delay1><Space><Delay2><Space><Delay3><Space><OnTime><Space><SeparationTime><Space>***------------//

    else if (strncmp(decodedString, active, 6) == 0) {
        digit = CLEAR;
        count = CLEAR; //count to extract Timestamp (total 6 attributes)                
        for (msgIndex = 6; count < 5; msgIndex++) {
            //is number
            if (isNumber(decodedString[msgIndex])) {
                if (decodedString[msgIndex + 1] == space || decodedString[msgIndex + 1] == '#') {
                    decodedString[msgIndex] = decodedString[msgIndex] - 48;
                    digit = digit + decodedString[msgIndex];
                } else {
                    decodedString[msgIndex] = decodedString[msgIndex] - 48;
                    decodedString[msgIndex] = decodedString[msgIndex] * 10;
                    digit = digit * 10;
                    digit = digit + decodedString[msgIndex];
                }
            } else {
                count++;
                switch (count) {
                    case 1: // code to extract filtrationDelay1;
                        filtrationDelay1 = (unsigned char) digit;
                        digit = CLEAR;
                        break;
                    case 2: // code to extract filtrationDelay2;
                        filtrationDelay2 = (unsigned char) digit;
                        digit = CLEAR;
                        break;
                    case 3: // code to extract filtrationDelay3;
                        filtrationDelay3 = (unsigned char) digit;
                        digit = CLEAR;
                        break;
                    case 4: // code to extract filtrationOnTime;
                        filtrationOnTime = (unsigned char) digit;
                        digit = CLEAR;
                        break;
                    case 5: // code to filtrationSeperationTime
                        filtrationSeperationTime = (unsigned char) digit;
                        digit = CLEAR;
                        filtrationEnabled = true;
                        break;
                }
            }
        }
        __delay_ms(100);
        saveFiltrationSequenceData();
        __delay_ms(100);
        msgIndex = CLEAR;
#ifdef LCD_DISPLAY_ON_H        
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);
        lcdWriteStringAtCenter("Water Filtration", 2);
        lcdWriteStringAtCenter("Sequence Activated", 3);
#endif
        /***************************/
        publishResponse(cmd10_10,false);
#ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        //setBCDdigit(0x05, 0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
        /***************************/
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_Configure_OUT\r\n");
        //********Debug log#end**************//
#endif
        return;
    }//#7>..............Disable filtration.................//
        // DACTIVE
    else if (strncmp(decodedString, dactive, 7) == 0) {
        filtrationEnabled = false; // filtration disabled
        __delay_ms(100);
        saveFiltrationSequenceData();
        __delay_ms(100);
        msgIndex = CLEAR;
        /***************************/
#ifdef LCD_DISPLAY_ON_H        
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);
        lcdWriteStringAtCenter("Water Filtration", 2);
        lcdWriteStringAtCenter("Sequence Deactivated", 3);
#endif
        publishResponse(cmd11_11,false);
        //sendSms(SmsFilt2, userMobileNo, noInfo);
#ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        //setBCDdigit(0x05, 0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
        /***************************/
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_Hold_OUT\r\n");
        //********Debug log#end**************//
#endif
        return;
    }//#7>..............get filtration data.................//
        // FDATA
    else if (strncmp(decodedString, fdata, 5) == 0) {
        if (filtrationEnabled) {
            msgIndex = CLEAR;
            /***************************/
#ifdef LCD_DISPLAY_ON_H        
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);
            lcdWriteStringAtCenter("Fecthing", 2);
            lcdWriteStringAtCenter("Filtration Sequence", 3);
#endif
            //sendSms(SmsFilt4, userMobileNo, filtrationData);
#ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            //setBCDdigit(0x05, 0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
            /***************************/
        } else {
            msgIndex = CLEAR;
#ifdef LCD_DISPLAY_ON_H        
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);
            lcdWriteStringAtCenter("Water Filtration", 2);
            lcdWriteStringAtCenter("Is Not Enabled", 3);
#endif
            publishResponse(cmd12_13,false);
            /***************************/
            //sendSms(SmsFilt3, userMobileNo, noInfo);
#ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            //setBCDdigit(0x05, 0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
            /***************************/
        }
        /***************************/
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_sendDiagnosticData_OUT\r\n");
        //********Debug log#end**************//
#endif
        return;
    }//#8>..............Set RTC Time.................//
        //Msg Format---------***Feed<Space><DD><Space><MM><Space><YY><Space><Hr><Space><Min><Space><Sec>***------------//
        //SMS Location----------63---------<68>-------<71>-------<74>-------<77>-------<80>------<83>//
    else if (strncmp(decodedString, feed, 4) == 0) {
        digit = CLEAR;
        count = CLEAR; //count to extract Timestamp (total 6 attributes)                
        for (msgIndex = 5; count < 6; msgIndex += 3) {
            //is number
            if (isNumber(decodedString[msgIndex])) {
                decodedString[msgIndex] = decodedString[msgIndex] - 48;
                digit = decodedString[msgIndex];
                digit = digit * 10;
                decodedString[msgIndex + 1] = decodedString[msgIndex + 1] - 48;
                digit = digit + decodedString[msgIndex + 1];
                count++;
                switch (count) {
                    case 1: // code to extract DD;
                        currentDD = (unsigned char) digit;
                        digit = CLEAR;
                        break;
                    case 2: // code to extract mm;
                        currentMM = (unsigned char) digit;
                        digit = CLEAR;
                        break;
                    case 3: // code to extract YY;
                        currentYY = (unsigned char) digit;
                        digit = CLEAR;
                        break;
                    case 4: // code to extract Hour;
                        currentHour = (unsigned char) digit;
                        digit = CLEAR;
                        break;
                    case 5: // code to extract Minutes;
                        currentMinutes = (unsigned char) digit;
                        digit = CLEAR;
                        break;
                    case 6: // code to extract Seconds;
                        currentSeconds = (unsigned char) digit;
                        digit = CLEAR;
                        break;
                }
            } else {
                msgIndex = CLEAR;
#ifdef LCD_DISPLAY_ON_H        
                lcdClearLine(2);
                lcdClearLine(3);
                lcdClearLine(4);
                lcdWriteStringAtCenter("Incorrect Local Time", 2);
                lcdWriteStringAtCenter("Found", 3);
#endif
                /***************************/
                /***************************/
                publishNotification("Time Alert",NotRTC6_36,false); // Acknowledge user about successful Authentication
                /***************************/
                //publishResponse();
#ifdef SMS_DELIVERY_REPORT_ON_H
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                //setBCDdigit(0x05, 0);
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
                /***************************/
#ifdef DEBUG_MODE_ON_H
                //********Debug log#start************//
                transmitStringToDebug("extractReceivedSms_Configure_OUT\r\n");
                //********Debug log#end**************//
#endif
                return;
            }
        }
        if (count == 6) {
            feedTimeInRTC();
            msgIndex = CLEAR;
    #ifdef LCD_DISPLAY_ON_H        
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);
            lcdWriteStringAtCenter("System Time Synced", 2);
            lcdWriteStringAtCenter("To Local Time", 3);
    #endif                     
            /***************************/
            publishResponse(cmd13_8,false);
    #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            //setBCDdigit(0x05, 0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
    #endif
            /***************************/
        }
        else {
            msgIndex = CLEAR;
    #ifdef LCD_DISPLAY_ON_H        
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);
            lcdWriteStringAtCenter("System Time Not sync", 2);
            lcdWriteStringAtCenter("Time Error", 3);
    #endif                     
            /***************************/
            publishResponse(cmd14_11,false);
    #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            //setBCDdigit(0x05, 0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
    #endif
            /***************************/
        }
        
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_Configure_OUT\r\n");
        //********Debug log#end**************//
#endif
        return;
    }//#9>..............Get RTC Time.................//
        // Get current Time
    else if (strncmp(decodedString, time, 4) == 0) {
        fetchTimefromRTC(); // Get today's date
        clearTempBytesString();
        temporaryBytesArray[0] = '*';
        temporaryBytesArray[1] = (currentDD / 10) + 48;
        temporaryBytesArray[2] = (currentDD % 10) + 48;
        temporaryBytesArray[3] = ' ';
        temporaryBytesArray[4] = (currentMM / 10) + 48;
        temporaryBytesArray[5] = (currentMM % 10) + 48;
        temporaryBytesArray[6] = ' ';
        temporaryBytesArray[7] = (currentYY / 10) + 48;
        temporaryBytesArray[8] = (currentYY % 10) + 48;
        temporaryBytesArray[9] = ' ';
        temporaryBytesArray[10] = (currentHour / 10) + 48;
        temporaryBytesArray[11] = (currentHour % 10) + 48;
        temporaryBytesArray[12] = ' ';
        temporaryBytesArray[13] = (currentMinutes / 10) + 48;
        temporaryBytesArray[14] = (currentMinutes % 10) + 48;
        temporaryBytesArray[15] = ' ';
        temporaryBytesArray[16] = (currentSeconds / 10) + 48;
        temporaryBytesArray[17] = (currentSeconds % 10) + 48;
        temporaryBytesArray[18] = '#';
        temporaryBytesArray[19] = '\0';
                
        msgIndex = CLEAR;
        /***************************/
#ifdef LCD_DISPLAY_ON_H        
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);
        lcdWriteStringAtCenter("Current Time:", 2);
        lcdSetCursor(3,1);
        lcdWriteStringIndex(temporaryBytesArray+1,17);
#endif
        publishResponse(temporaryBytesArray,false);
        //sendSms(SmsT2, userMobileNo, timeRequired);
#ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        //setBCDdigit(0x05, 0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
        /***************************/
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_Current Time_OUT\r\n");
        //********Debug log#end**************//
#endif
        return;
    }//#10>.............. Extract configured Data for Self Diagnostic.................//
        // extract<x>
    else if (strncmp(decodedString, extract, 7) == 0) {
        iterator_field = fetchDigits(7);
        msgIndex = CLEAR;
        if (fieldValve[iterator_field].isConfigured) {
#ifdef LCD_DISPLAY_ON_H        
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);
            lcdWriteStringAtCenter("Fetching Irri.", 2);
            lcdWriteStringAtCenter("Data", 3);
            lcdWriteStringAtCenter("For Field No:", 4);
            lcdSetCursor(4,17);
            clearFieldByte();
            sprintf(fieldByte,"%d",iterator_field+1);
            temp = strlen((const char *)fieldByte);
            lcdWriteStringIndex(fieldByte,temp);
#endif
            /***************************/
            //sendSms(SmsIrr7, userMobileNo, IrrigationData); // Give diagnostic data
#ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            //setBCDdigit(0x05, 0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
            /***************************/
        } else {
#ifdef LCD_DISPLAY_ON_H        
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);
            lcdWriteStringAtCenter("Irri. not configured", 2);
            lcdWriteStringAtCenter("For Field No:", 3);
            lcdSetCursor(3,17);
            clearFieldByte();
            sprintf(fieldByte,"%d",iterator_field+1);
            temp = strlen((const char *)fieldByte);
            lcdWriteStringIndex(fieldByte,temp);
#endif
            /***************************/
            //sendSms(SmsIrr3, userMobileNo, fieldNoRequired); // Acknowledge user about  Irrigation not configured
#ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            //setBCDdigit(0x05, 0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
            /***************************/
        }
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_sendDiagnosticData_OUT\r\n");
        //********Debug log#end**************//
#endif
        return;
    }//#11>......Set up Motor load condition Manually .......//
    // CT<Space><No Load Cutoff>Space<Full Load Cutoff><Space>
    else if (strncmp(decodedString, ct, 3) == 0) {
        if (Irri_Motor == ON) {
            msgIndex = CLEAR;
#ifdef LCD_DISPLAY_ON_H        
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);
            lcdWriteStringAtCenter("Irri. is Running", 2);
            lcdWriteStringAtCenter("Motor Load Cut-Off", 3);
            lcdWriteStringAtCenter("Not Set", 4);
#endif
            publishResponse(cmd16_9,false);
            /***************************/
            //sendSms(SmsMotor4, userMobileNo, noInfo); // Acknowledge user about Irrigation is active, Motor load cut-off procedure not started
#ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            //setBCDdigit(0x05, 0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
            /***************************/
#ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("Motor load values set successfully_OUT\r\n");
            //********Debug log#end**************//
#endif
            return;
        }
        digit = CLEAR;
        count = CLEAR; //count to extract motor load condition (total 2 attributes)
        for (count = 1, msgIndex = 3; count <= 2; msgIndex++) {
            if (isNumber(decodedString[msgIndex])) { //is number
                if (decodedString[msgIndex + 1] == space || decodedString[msgIndex + 1] == '#' ) {
                    decodedString[msgIndex] = decodedString[msgIndex] - 48;
                    digit = digit + decodedString[msgIndex];
                } else {
                    decodedString[msgIndex] = decodedString[msgIndex] - 48;
                    decodedString[msgIndex] = decodedString[msgIndex] * 10;
                    digit = digit * 10;
                    digit = digit + decodedString[msgIndex];
                }
            } else {
                switch (count) {
                    case 1: // code to extract on period;
                        noLoadCutOff = digit;
                        digit = CLEAR;
                        break;
                    case 2: // code to extract off period;
                        fullLoadCutOff = digit;
                        digit = CLEAR;
                        break;
                }
                count++;
            }
        }
        __delay_ms(100);
        saveMotorLoadValuesIntoEeprom();
        __delay_ms(100);
        msgIndex = CLEAR;
        /***************************/
#ifdef LCD_DISPLAY_ON_H        
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);
        lcdWriteStringAtCenter("Motor Load Cut-Off", 2);
        lcdWriteStringAtCenter("Set Successfully", 3);
#endif
        publishResponse(cmd15_6,false);
        //sendSms(SmsMotor2, userMobileNo, noInfo);
#ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        //setBCDdigit(0x05, 0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
        /***************************/
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("Motor load values set successfully_OUT\r\n");
        //********Debug log#end**************//
#endif
        return;
    }//#15>......GetCTValues for fetching Motor load values.......//
    else if (strncmp(decodedString, getct, 3) == 0) {
        msgIndex = CLEAR;
        lower8bits = noLoadCutOff;
        clearTempBytesString();
        temporaryBytesArray[0] = '*';
        temporaryBytesArray[1] = 'C';
        temporaryBytesArray[2] = 'T';
        temporaryBytesArray[3] = ' ';
        temporaryBytesArray[4] = (unsigned char) ((lower8bits / 1000) + 48);
        temporaryBytesArray[5] = (unsigned char) (((lower8bits % 1000) / 100) + 48);
        temporaryBytesArray[6] = (unsigned char) (((lower8bits % 100) / 10) + 48);
        temporaryBytesArray[7] = (unsigned char) ((lower8bits % 10) + 48);
        temporaryBytesArray[8] = ' ';
        lower8bits = fullLoadCutOff;
        temporaryBytesArray[9] = (unsigned char) ((lower8bits / 1000) + 48);
        temporaryBytesArray[10] = (unsigned char) (((lower8bits % 1000) / 100) + 48);
        temporaryBytesArray[11] = (unsigned char) (((lower8bits % 100) / 10) + 48);
        temporaryBytesArray[12] = (unsigned char) ((lower8bits % 10) + 48);
        temporaryBytesArray[13] = '#';
        temporaryBytesArray[14] = '\0';
#ifdef LCD_DISPLAY_ON_H        
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);   
        lcdWriteStringAtCenter("Fetching Motor", 2);
        lcdWriteStringAtCenter("Load Cut-Off Values", 3);
        lcdSetCursor(4,1);
        lcdWriteStringIndex(temporaryBytesArray+1,14);
#endif
        /***************************/
        publishResponse(temporaryBytesArray,false);
        /***************************/
        /***************************/
        //sendSms(SmsMotor3, userMobileNo, motorLoadRequired);
#ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission	
        //setBCDdigit(0x05, 0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
        /***************************/
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSMS_Get CT Values_OUT\r\n");
        //********Debug log#end**************//
#endif
        return;
    }//#16>......Get FREQUENCY Values for field moisture sensors.......//
    else if (strncmp(decodedString, getfreq, 4) == 0) {
        iterator_field = fetchDigits(4);
        __delay_ms(100);
        isFieldMoistureSensorWetLora(iterator_field);
        if (moistureSensorFailed) {
            moistureSensorFailed = false;
#ifdef LCD_DISPLAY_ON_H
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);
            lcdWriteStringAtCenter("Moisture Sensor", 2);
            lcdWriteStringAtCenter("Is Failed", 3);
            lcdWriteStringAtCenter("For Field No:", 4);
            lcdSetCursor(4,17);
            clearFieldByte();
            sprintf(fieldByte,"%d",iterator_field+1);
            temp = strlen((const char *)fieldByte);
            lcdWriteStringIndex(fieldByte,temp);
#endif
            publishResponse(cmd17_11,false);
            /***************************/
            //sendSms(SmsMS3, userMobileNo, fieldNoRequired);
#ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            //setBCDdigit(0x05, 0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
            /***************************/
        } else {
            /***************************/
            msgIndex = CLEAR;
#ifdef LCD_DISPLAY_ON_H
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);
            lcdWriteStringAtCenter("Fetching", 2);
            lcdWriteStringAtCenter("Moisture Sensor", 3);
            lcdWriteStringAtCenter("Frequency", 4);
#endif
            clearTempBytesString();
            lower8bits = noLoadCutOff;
            temporaryBytesArray[0] = '*';
            temporaryBytesArray[1] = 'F';
            temporaryBytesArray[2] = 'R';
            temporaryBytesArray[3] = 'E';
            temporaryBytesArray[4] = 'Q';
            temporaryBytesArray[5] = ' ';
            lower8bits = moistureLevel;
            temporaryBytesArray[6] = (unsigned char) ((lower8bits / 10000) + 48);
            temporaryBytesArray[7] = (unsigned char) (((lower8bits % 10000) / 1000) + 48);
            temporaryBytesArray[8] = (unsigned char) (((lower8bits % 1000) / 100) + 48);
            temporaryBytesArray[9] = (unsigned char) (((lower8bits % 100) / 10) + 48);
            temporaryBytesArray[10] = (unsigned char) ((lower8bits % 10) + 48);
            temporaryBytesArray[11] = '#';
            temporaryBytesArray[12] = '\0';
            publishResponse(temporaryBytesArray,false);
            /***************************/
            //sendSms(SmsMS2, userMobileNo, frequencyRequired);
#ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            //setBCDdigit(0x05, 0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
            /***************************/

        }

#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_sendMoistureSensorData_OUT\r\n");
        //********Debug log#end**************//
#endif
        return;
    }//#17>......Set up Motor load condition Automatically.......//
        // SETCT<FieldNno.>
    else if (strncmp(decodedString, setct, 3) == 0) {
        iterator_field = fetchDigits(3);
        __delay_ms(100);
        if (Irri_Motor == ON) {
            msgIndex = CLEAR;
#ifdef LCD_DISPLAY_ON_H        
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);
            lcdWriteStringAtCenter("Irri. is Running", 2);
            lcdWriteStringAtCenter("Motor Load Cut-Off", 3);
            lcdWriteStringAtCenter("Not Set", 4);
#endif
            publishResponse(cmd18_12,false);
            /***************************/
            //sendSms(SmsMotor4, userMobileNo, noInfo); // Acknowledge user about Irrigation is active, Motor load cut-off procedure not started
#ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            //setBCDdigit(0x05, 0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
            /***************************/
#ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("Motor load values set successfully_OUT\r\n");
            //********Debug log#end**************//
#endif
            return;
        }
        /***************************/
        // for field no. 01 to 09
        /*
        if (iterator < 9) {
            temporaryBytesArray[0] = 48; // To store field no. of valve in action 
            temporaryBytesArray[1] = iterator + 49; // To store field no. of valve in action 
        }// for field no. 10 to 12
        else if (iterator > 8 && iterator < fieldCount) {
            temporaryBytesArray[0] = 49; // To store field no. of valve in action 
            temporaryBytesArray[1] = iterator + 39; // To store field no. of valve in action 
        }
         */ 
        //__delay_ms(2500);
        //__delay_ms(2500);
        //calibrateMotorCurrent(NoLoad, iterator);
        //__delay_ms(1000);
        calibrateMotorCurrent(FullLoad, iterator_field);
        __delay_ms(100);
        saveMotorLoadValuesIntoEeprom();
        __delay_ms(100);
        msgIndex = CLEAR;
#ifdef LCD_DISPLAY_ON_H        
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);
        lcdWriteStringAtCenter("Motor Load Cut-Off", 2);
        lcdWriteStringAtCenter("Set Successfully", 3);
#endif
        msgIndex = CLEAR;
        lower8bits = noLoadCutOff;
        clearTempBytesString();
        temporaryBytesArray[0] = '*';
        temporaryBytesArray[1] = 'C';
        temporaryBytesArray[2] = 'T';
        temporaryBytesArray[3] = ' ';
        temporaryBytesArray[4] = (unsigned char) ((lower8bits / 1000) + 48);
        temporaryBytesArray[5] = (unsigned char) (((lower8bits % 1000) / 100) + 48);
        temporaryBytesArray[6] = (unsigned char) (((lower8bits % 100) / 10) + 48);
        temporaryBytesArray[7] = (unsigned char) ((lower8bits % 10) + 48);
        temporaryBytesArray[8] = ' ';
        lower8bits = fullLoadCutOff;
        temporaryBytesArray[9] = (unsigned char) ((lower8bits / 1000) + 48);
        temporaryBytesArray[10] = (unsigned char) (((lower8bits % 1000) / 100) + 48);
        temporaryBytesArray[11] = (unsigned char) (((lower8bits % 100) / 10) + 48);
        temporaryBytesArray[12] = (unsigned char) ((lower8bits % 10) + 48);
        temporaryBytesArray[13] = '#';
        temporaryBytesArray[14] = '\0';
#ifdef LCD_DISPLAY_ON_H        
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);   
        lcdWriteStringAtCenter("Fetching Motor", 2);
        lcdWriteStringAtCenter("Load Cut-Off Values", 3);
        lcdSetCursor(4,1);
        lcdWriteStringIndex(temporaryBytesArray+1,14);
#endif
        /***************************/
        publishResponse(temporaryBytesArray,false);
        /***************************/
        /***************************/
        //sendSms(SmsMotor3, userMobileNo, motorLoadRequired);
#ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        //setBCDdigit(0x05, 0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
        /***************************/
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("Motor load values set successfully_OUT\r\n");
        //********Debug log#end**************//
#endif
        return;
    }
    //#19>......MapFieldData.......//
    //               4,5     6,7     8,9     10,11     12,13   14,15   16,17    18,19    20,21
    // MAP<Space><FieldNo.><SlaveNo><IONo.><FieldNo.><SlaveNo><IONo.><FieldNo.><SlaveNo><IONo.><Space>
    else if (strncmp(decodedString, map, 3) == 0) {
        unsigned char index = 4;
        while (decodedString[index] != space) {
            iterator_field = fetchDigits(index);
            if (iterator_field != 255) {
                fieldMap[iterator_field*2] = fetchDigits(index+2) + 1;// Slave No.
                fieldMap[(iterator_field*2)+1] = fetchDigits(index+4) + 1;// IO Pin No.
                index = index +6;
            } else {
                break;
            }
        }
        if (iterator_field == 255) {
#ifdef LCD_DISPLAY_ON_H
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);
            lcdWriteStringAtCenter("Field Mapping", 2);
            lcdWriteStringAtCenter("Error", 3);
#endif 
            msgIndex = CLEAR;
            /***************************/
            publishResponse(cmd19_10,false);
            /***************************/
            /***************************/
            //sendSms(SmsIrr10, userMobileNo, noInfo); //To notify field mapping for Lora slave.
#ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            //setBCDdigit(0x05, 0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
            /***************************/
            
        } else {
#ifdef LCD_DISPLAY_ON_H
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);
            lcdWriteStringAtCenter("Field Mapping", 2);
            lcdWriteStringAtCenter("Completed", 3);
#endif 
            saveFieldMappingIntoEeprom();
            msgIndex = CLEAR;
            /***************************/
            publishResponse(cmd18_7,false);
            /***************************/
            /***************************/
            //sendSms(SmsIrr10, userMobileNo, noInfo); //To notify field mapping for Lora slave.
#ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            //setBCDdigit(0x05, 0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
            /***************************/
        }
    } else if (strncmp(decodedString, alive1, 4) == 0) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);
        lcdWriteStringAtCenter("User Authenticated", 2);
#endif
        msgIndex = CLEAR;
        /***************************/
        //publishResponse(); // Acknowledge user about Authentication failed
#ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission	
        //setBCDdigit(0x05, 0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
        /***************************/
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_Not Authenticated_OUT\r\n");
        //********Debug log#end**************//
#endif
        return;
    } else {
        #ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);
        lcdWriteStringAtCenter("Unknown Command", 2);
#endif
        //setBCDdigit(0x06, 0); // (6.) BCD indication for unknown sms from registered user
        __delay_ms(2000);
        /***************************/
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_UnknownSMS_fromRegisteredUser_OUT\r\n");
        //********Debug log#end**************//
#endif
        return;
    }
}
/************************Actions on Received SMS#End************************************/


/*********** Moisture sensor measurement#Start********/

/*************************************************************************************************************************

This function is called to measure soil moisture of given field and indicate if wet field found .
The Moisture level is measured in terms of frequency of square wave generated by IC555 based on Senor resistance.
The Sensor resistance is high and low for Dry and Wet condition respectively.
This leads the output of IC555 with high and low pulse width.
For Dry condition pulse width is high and for wet condition pulse width is low.
i.e. for Dry condition pulse occurrence is low and for wet condition pulse occurrence is high
Here Timer1 is used to count frequency of pulses by measuring timer count for 1 pulse width and averaging it for 10 pulses.

 **************************************************************************************************************************/
_Bool isFieldMoistureSensorWet() {
    unsigned long moistureLevelAvg = CLEAR;
    unsigned long timer1Value = CLEAR; // To store 16 bit SFR Timer1 Register value
    unsigned long constant = 160000; // Constant to calculate frequency in Hz ~16MHz 16000000/100 to convert freq from 5 digit to 3 digit
    unsigned char itr = CLEAR, avg = 20;

    moistureLevel = CLEAR; // To store moisture level in Hz

#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("isFieldMoistureSensorWet_IN : ");
    transmitNumberToDebug(temporaryBytesArray, 2);
    transmitStringToDebug("\r\n");
    //********Debug log#end**************//
#endif
    moistureLevel = LOW;
    checkMoistureSensor = true;
    moistureSensorFailed = false;
    timer3Count = 5; // 5 second window
    // Averaging measured pulse width
    for (itr = 1; itr <= avg && !moistureSensorFailed; itr++) {
        T1CONbits.TMR1ON = OFF;
        TMR1H = CLEAR;
        TMR1L = CLEAR;
        Timer1Overflow = CLEAR;
        // check field moisture of valve in action
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("MoistureSensor1 == HIGH\r\n");
        //********Debug log#end**************//
#endif
        T3CONbits.TMR3ON = ON; // Start timer thread to unlock system if Sensor fails to respond within 15 sec    
        controllerCommandExecuted = false;
        while (Mois_SNSR == HIGH && controllerCommandExecuted == false);
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("MoistureSensor1 == LOW\r\n");
        //********Debug log#end**************//
#endif
        while (Mois_SNSR == LOW && controllerCommandExecuted == false); // Wait for rising edge
        T1CONbits.TMR1ON = ON; // Start Timer after rising edge detected
        while (Mois_SNSR == HIGH && controllerCommandExecuted == false); // Wait for falling edge
        if (!controllerCommandExecuted) {
            controllerCommandExecuted = true;
            PIR5bits.TMR3IF = SET; //Stop timer thread 
        }
        T1CONbits.TMR1ON = OFF; // Stop timer after falling edge detected
        controllerCommandExecuted = true;
        timer1Value = TMR1L; // Store lower byte of 16 bit timer
        timer1Value |= ((unsigned int) TMR1H) << 8; // Get higher and lower byte of  timer1 register
        moistureLevelAvg = (Timer1Overflow * 65536) + timer1Value;
        moistureLevelAvg *= 2; // Entire cycle width
        moistureLevelAvg = (constant / moistureLevelAvg); // Frequency = 16Mhz/ Pulse Width
        if (itr == 1) {
            moistureLevel = (unsigned int) moistureLevelAvg;
        }
        moistureLevel = moistureLevel / 2;
        moistureLevelAvg = moistureLevelAvg / 2;
        moistureLevel = moistureLevel + (unsigned int) moistureLevelAvg;
        if (moistureSensorFailed) {
            moistureLevel = 0;
        }
    }
    checkMoistureSensor = false;
    //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
    if (moistureLevel >= 150) {
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("isFertigationSensorWet_Yes_Out\r\n");
        //********Debug log#end**************//
#endif
        return true;
    }
    else {
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("isFertigationSensorWet_No_Out\r\n");
        //********Debug log#end**************//
#endif
        return false;
    }
}



/*********** Moisture sensor measurement at Lora Slave #Start********/

/*************************************************************************************************************************

This function is called to send command to lora slave to measure soil moisture of given field and indicate if wet field found .
The Moisture level is measured in terms of frequency of square wave generated by IC555 based on Senor resistance.
The Sensor resistance is high and low for Dry and Wet condition respectively.
This leads the output of IC555 with high and low pulse width.
For Dry condition pulse width is high and for wet condition pulse width is low.
i.e. for Dry condition pulse occurrence is low and for wet condition pulse occurrence is high
Here Timer1 is used to count frequency of pulses by measuring timer count for 1 pulse width and averaging it for 10 pulses.

 **************************************************************************************************************************/
_Bool isFieldMoistureSensorWetLora(unsigned char FieldNo) {
    unsigned char action;
    loraAttempt = 0;
    action = 0x02;
    //setBCDdigit(0x09, 0); // (9.) BCD indication for Moisture Sensor Failure Error
    moistureSensorFailed = false;
    // Averaging measured pulse width

#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    /***************************/
    // for field no. 01 to 09
    if (FieldNo < 9) {
        temporaryBytesArray[0] = 48; // To store field no. of valve in action 
        temporaryBytesArray[1] = FieldNo + 49; // To store field no. of valve in action 
    }// for field no. 10 to 12
    else if (FieldNo > 8 && FieldNo < fieldCount) {
        temporaryBytesArray[0] = 49; // To store field no. of valve in action 
        temporaryBytesArray[1] = FieldNo + 39; // To store field no. of valve in action 
    } else {
        temporaryBytesArray[0] = 57; // To store field no. of valve in action 
        temporaryBytesArray[1] = 57; // To store field no. of valve in action 

    }
    /***************************/
    transmitStringToDebug("isFieldMoistureSensorWetLora_IN : ");
    transmitNumberToDebug(temporaryBytesArray, 2);
    transmitStringToDebug("\r\n");
    //********Debug log#end**************//
#endif

    do {
        sendCmdToLora(action, FieldNo);
    } while (loraAttempt < 2);
    if (LoraConnectionFailed || moistureSensorFailed) { // Unsuccessful Sensor reading
        moistureLevel = CLEAR;
        moistureSensorFailed = true;
    }
    //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
    if ((moistureLevel / 100) >= fieldValve[FieldNo].wetValue) { //Field is full wet, no need to switch ON valve and motor, estimate new due dates
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("isFieldMoistureSensorWetLora_Yes_Out\r\n");
        //********Debug log#end**************//
#endif
        return true;
    } else {
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("isFieldMoistureSensorWetLorat_No_Out\r\n");
        //********Debug log#end**************//
#endif
        return false;
    }

#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    /***************************/
    // for field no. 01 to 09
    if (FieldNo < 9) {
        temporaryBytesArray[0] = 48; // To store field no. of valve in action 
        temporaryBytesArray[1] = FieldNo + 49; // To store field no. of valve in action 
    }// for field no. 10 to 12
    else if (FieldNo > 8 && FieldNo < fieldCount) {
        temporaryBytesArray[0] = 49; // To store field no. of valve in action 
        temporaryBytesArray[1] = FieldNo + 39; // To store field no. of valve in action 
    } else {
        temporaryBytesArray[0] = 57; // To store field no. of valve in action 
        temporaryBytesArray[1] = 57; // To store field no. of valve in action 

    }
    /***************************/
    transmitStringToDebug("isFieldMoistureSensorWetLora_OUT : ");
    transmitNumberToDebug(temporaryBytesArray, 2);
    transmitStringToDebug("\r\n");
    //********Debug log#end**************//
#endif
}

/*********** Motor Dry run condition#Start********/

/*************************************************************************************************************************

This function is called to measure motor phase current to detect dry run condition.
The Dry run condition is measured in terms of voltage of CT connected to one of the phase of motor.
The motor phase current is high  and low  for Wet (load) and Dry (no load) condition respectively.
This leads the output of CT with high and low voltage.
For Dry condition, the CT voltage is high and for wet condition, the CT voltage is low.
Here ADC module is used to measure high/ low voltage of CT thereby identifying load/no load condition.

 **************************************************************************************************************************/

_Bool isMotorInNoLoad(void) {
    unsigned int ctOutput = 0;
    unsigned int temp = 0;
    lowPhaseCurrentDetected = false;
    dryRunDetected = false;
    temp = (fullLoadCutOff) / 10;
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("isMotorInNoLoad_IN\r\n");
    //********Debug log#end**************//
#endif
    // Averaging measured pulse width
    selectChannel(CTchannel);
    ctOutput = getADCResult();
    if (ctOutput > temp && ctOutput <= noLoadCutOff) {
        dryRunDetected = true; //Set Low water level
#ifdef LCD_DISPLAY_ON_H
        displayIcon(dry_icon);
#endif					   
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("isMotorInNoLoad_Dry_Yes_OUT\r\n");
        //********Debug log#end**************//
#endif
        return true;
    } else if (ctOutput == 0 || (ctOutput > 0 && ctOutput <= temp)) { // no phase current
        lowPhaseCurrentDetected = true; //Set phase current low
#ifdef LCD_DISPLAY_ON_H
        displayIcon(dry_icon);
#endif					   
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("isMotorInNoLoad_LowPhase_Yes_OUT\r\n");
        //********Debug log#end**************//
#endif
        return true;
    } else {
        lowPhaseCurrentDetected = false;
        dryRunDetected = false; //Set High water level
#ifdef LCD_DISPLAY_ON_H
        removeIcon(dry_icon);
#endif					   
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("isMotorInNoLoad_Dry_LowPhase_No_OUT\r\n");
        //********Debug log#end**************//
#endif
        return false;
    }
}

/*********** Motor Dry run condition#End********/

/*********** Motor current calibration#Start********/

/*************************************************************************************************************************

This function is called to calibrate motor phase current to set no load, overload condition.
The Dry run condition is measured in terms of voltage of CT connected to one of the phase of motor.
The motor phase current is high  and low  for Wet (load) and Dry (no load) condition respectively.
This leads the output of CT with high and low voltage.
For Dry condition, the CT voltage is high and for wet condition, the CT voltage is low.
Here ADC module is used to measure high/ low voltage of CT thereby identifying load/no load condition.

 **************************************************************************************************************************/

void calibrateMotorCurrent(unsigned char loadType, unsigned char FieldNo) {
    unsigned int ctOutput = 0;
    unsigned char itr = 0, limit = 30;
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("calibrateMotorCurrent_IN\r\n");
    //********Debug log#end**************//
#endif
    if (loadType == FullLoad) {
        switch (FieldNo) {
                __delay_ms(1000);
            case 0:
                Irri_Out1 = ON; // switch on valve for field 1
                break;
            case 1:
                Irri_Out2 = ON; // switch off valve for field 2
                break;
            case 2:
                Irri_Out3 = ON; // switch on valve for field 3
                break;
            case 3:
                Irri_Out4 = ON; // switch on valve for field 4
                break;
            case 4:
                Irri_Out5 = ON; // switch off valve for field 5
                break;
            case 5:
                Irri_Out6 = ON; // switch off valve for field 6
                break;
            case 6:
                Irri_Out7 = ON; // switch on valve for field 7
                break;
            case 7:
                Irri_Out8 = ON; // switch on valve for field 8
                break;
            case 8:
                Irri_Out9 = ON; // switch on valve for field 9
                break;
            case 9:
                Irri_Out10 = ON; // switch on valve for field 10
                break;
            case 10:
                Irri_Out11 = ON; // switch on valve for field 11
                break;
            case 11:
                Irri_Out12 = ON; // switch on valve for field 12
                break;
        }
    }
    if (Irri_Motor != ON) {
        __delay_ms(2500);
        __delay_ms(2500);
        Irri_Motor = ON;
        __delay_ms(100);
#ifdef STAR_DELTA_DEFINITIONS_H
        __delay_ms(500);
        Irri_MotorT = ON;
        __delay_ms(900);
        Irri_MotorT = OFF;
#endif
    }
    __delay_ms(2500);
    __delay_ms(2500);
    // Averaging measured pulse width
    //setBCDdigit(0x0F, 1); // BCD Indication for Flash
    __delay_ms(2500);
    __delay_ms(2500);
    selectChannel(CTchannel);
    if (loadType == NoLoad) {
        limit = 11; //~1.5 min
    }
    for (itr = 0; itr < limit; itr++) {
        ctOutput = getADCResult();
        __delay_ms(2500);
        lower8bits = ctOutput;
        temporaryBytesArray[0] = (unsigned char) ((lower8bits / 1000) + 48);
        //setBCDdigit(temporaryBytesArray[0], 1);
        __delay_ms(1000);
        //setBCDdigit(0x0F, 1);
        __delay_ms(500);
        lower8bits = lower8bits % 1000;
        temporaryBytesArray[0] = (unsigned char) ((lower8bits / 100) + 48);
        //setBCDdigit(temporaryBytesArray[0], 1);
        __delay_ms(1000);
        //setBCDdigit(0x0F, 1);
        __delay_ms(500);
        lower8bits = lower8bits % 100;
        temporaryBytesArray[0] = (unsigned char) ((lower8bits / 10) + 48);
        //setBCDdigit(temporaryBytesArray[0], 1);
        __delay_ms(1000);
        //setBCDdigit(0x0F, 1);
        __delay_ms(500);
        lower8bits = lower8bits % 10;
        temporaryBytesArray[0] = (unsigned char) (lower8bits + 48);
        //setBCDdigit(temporaryBytesArray[0], 1);
        __delay_ms(1000);
        //setBCDdigit(0x0F, 1);
        if (loadType == FullLoad) {
            fullLoadCutOff = ctOutput;
            noLoadCutOff = (7 * fullLoadCutOff) / 10;
        } else if (loadType == NoLoad) {
            noLoadCutOff = ctOutput;
        }
    }
    if (loadType == FullLoad) {
        Irri_Motor = OFF;
    }
    if (loadType == FullLoad) {
        __delay_ms(1000);
        switch (FieldNo) {
            case 0:
                Irri_Out1 = OFF; // switch on valve for field 1
                break;
            case 1:
                Irri_Out2 = OFF; // switch off valve for field 2
                break;
            case 2:
                Irri_Out3 = OFF; // switch on valve for field 3
                break;
            case 3:
                Irri_Out4 = OFF; // switch on valve for field 4
                break;
            case 4:
                Irri_Out5 = OFF; // switch off valve for field 5
                break;
            case 5:
                Irri_Out6 = OFF; // switch off valve for field 6
                break;
            case 6:
                Irri_Out7 = OFF; // switch on valve for field 7
                break;
            case 7:
                Irri_Out8 = OFF; // switch on valve for field 8
                break;
            case 8:
                Irri_Out9 = OFF; // switch on valve for field 9
                break;
            case 9:
                Irri_Out10 = OFF; // switch on valve for field 10
                break;
            case 10:
                Irri_Out11 = OFF; // switch on valve for field 11
                break;
            case 11:
                Irri_Out12 = OFF; // switch on valve for field 12
                break;
        }
    }
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("calibrateMotorCurrent_OUT\r\n");
    //********Debug log#end**************//
#endif
}

/*********** Motor current calibration#End********/

/*********** Lora Slave InActive Action#Start********/

/*************************************************************************************************************************

This function is called to perform actions after detecting dry run condition.
After detecting Dry run condition, stop all active valves and set all valves due from today to tomorrow.
Notify user about all actions

 **************************************************************************************************************************/

void doLoraSlaveInActiveAction(void) {
    unsigned char field_No = CLEAR;
    unsigned int sleepCountVar = CLEAR;
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("doLoraSlaveInActiveAction_IN\r\n");
    //********Debug log#end**************//
#endif
    __delay_ms(100);
    fetchTimefromRTC(); // Get today's date
    __delay_ms(100);
    getDueDate(1); // calculate next day date										 
    for (field_No = 0; field_No < fieldCount; field_No++) {
        if (fieldValve[field_No].status == ON) {
            //__delay_ms(100);
            //powerOffMotor();
            //__delay_ms(1000);
            //deActivateValve(field_No); // Deactivate Valve upon Dry run condition and reset valve to next due time
            //valveDue = false;
            //fieldValve[field_No].status = OFF;
            fieldValve[field_No].cyclesExecuted = fieldValve[field_No].cycles;
            __delay_ms(100);
            //saveIrrigationValveOnOffStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
            //__delay_ms(100);
            //saveIrrigationValveCycleStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
            //__delay_ms(100);
            sleepCountVar = readActiveSleepCountFromEeprom();
            if (fieldValve[field_No].isFertigationEnabled) {
                /************Fertigation switch off due to InActive Lora Slave***********/
                if (fieldValve[field_No].fertigationStage == injectPeriod) {
                    Fert_Motor = OFF; // Switch off fertigation valve in case it is ON
                    //Switch off all Injectors after completing fertigation on Period
                    Fert_Inj1 = OFF;
                    Fert_Inj2 = OFF;
                    Fert_Inj3 = OFF;
                    Fert_Inj4 = OFF;
                    fieldValve[field_No].fertigationStage = OFF;
                    fieldValve[field_No].fertigationValveInterrupted = true;
                    remainingFertigationOnPeriod = readActiveSleepCountFromEeprom();
                    __delay_ms(100);
                    saveRemainingFertigationOnPeriod();
                    __delay_ms(100);
                    saveFertigationValveStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                    __delay_ms(100);
                    /******** Calculate and save Field Valve next date**********/
                    fieldValve[field_No].nextDueDD = (unsigned char) dueDD;
                    fieldValve[field_No].nextDueMM = dueMM;
                    fieldValve[field_No].nextDueYY = dueYY;
                    __delay_ms(100);
                    saveIrrigationValveDueTimeIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                    __delay_ms(100);
#ifdef LCD_DISPLAY_ON_H
                    lcdClearLine(2);
                    lcdClearLine(3);
                    lcdClearLine(4);				
                    lcdWriteStringAtCenter("Field Valve InActive", 2);
                    lcdWriteStringAtCenter("Ferti. Postponed", 3);
                    lcdWriteStringAtCenter("For Field No:", 4);
                    lcdSetCursor(4,17);
                    clearFieldByte();
                    sprintf(fieldByte,"%d",field_No+1); 
                    temp = strlen((const char *)fieldByte);
                    lcdWriteStringIndex(fieldByte,temp);

#endif
                    /***************************/
                    //sendSms(SmsDR1, userMobileNo, fieldNoRequired); // Acknowledge user about dry run detected and action taken
#ifdef SMS_DELIVERY_REPORT_ON_H
                    sleepCount = 2; // Load sleep count for SMS transmission action
                    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                    //setBCDdigit(0x05, 0);
                    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                    //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
                    /***************************/
                } else if (fieldValve[field_No].fertigationStage == wetPeriod) {
                    /******** Calculate and save Field Valve next date**********/
                    fieldValve[field_No].nextDueDD = (unsigned char) dueDD;
                    fieldValve[field_No].nextDueMM = dueMM;
                    fieldValve[field_No].nextDueYY = dueYY;
                    __delay_ms(100);
                    saveIrrigationValveDueTimeIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                    __delay_ms(100);
#ifdef LCD_DISPLAY_ON_H
                    lcdClearLine(2);
                    lcdClearLine(3);
                    lcdClearLine(4);				
                    lcdWriteStringAtCenter("Field Valve InActive", 2);
                    lcdWriteStringAtCenter("Ferti. Postponed", 3);
                    lcdWriteStringAtCenter("For Field No:", 4);
                    lcdSetCursor(4,17);
                    clearFieldByte();
                    sprintf(fieldByte,"%d",field_No+1);
                    temp = strlen((const char *)fieldByte);
                    lcdWriteStringIndex(fieldByte,temp);
#endif
                    /***************************/
                    //sendSms(SmsDR2, userMobileNo, fieldNoRequired); // Acknowledge user about dry run detected and action taken
#ifdef SMS_DELIVERY_REPORT_ON_H
                    sleepCount = 2; // Load sleep count for SMS transmission action
                    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                    //setBCDdigit(0x05, 0);
                    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                    //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
                    /***************************/
                }
            } else {
                if (sleepCountVar > (fieldValve[field_No].onPeriod / 2)) { // major part of valve execution is pending hence shift to next day
                    /******** Calculate and save Field Valve next date**********/
                    fieldValve[field_No].nextDueDD = (unsigned char) dueDD;
                    fieldValve[field_No].nextDueMM = dueMM;
                    fieldValve[field_No].nextDueYY = dueYY;
                    __delay_ms(100);
                    saveIrrigationValveDueTimeIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                    __delay_ms(100);
#ifdef LCD_DISPLAY_ON_H
                    lcdClearLine(2);
                    lcdClearLine(3);
                    lcdClearLine(4);				
                    lcdWriteStringAtCenter("Field Valve InActive", 2);
                    lcdWriteStringAtCenter("Irri. Postponed", 3);
                    lcdWriteStringAtCenter("For Field No:", 4);
                    lcdSetCursor(4,17);
                    clearFieldByte();
                    sprintf(fieldByte,"%d",field_No+1);
                    temp = strlen((const char *)fieldByte);
                    lcdWriteStringIndex(fieldByte,temp);
#endif
                    /***************************/
                    //sendSms(SmsDR3, userMobileNo, fieldNoRequired); // Acknowledge user about dry run detected and action taken
#ifdef SMS_DELIVERY_REPORT_ON_H
                    sleepCount = 2; // Load sleep count for SMS transmission action
                    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                    //setBCDdigit(0x05, 0);
                    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                    //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
                    /***************************/
                } else { // next due date
                    
#ifdef LCD_DISPLAY_ON_H
                    lcdClearLine(2);
                    lcdClearLine(3);
                    lcdClearLine(4);				
                    lcdWriteStringAtCenter("Field Valve InActive", 2);
                    lcdWriteStringAtCenter("Irri. Rescheduled", 3);
                    lcdWriteStringAtCenter("For Field No:", 4);
                    lcdSetCursor(4,17);
                    clearFieldByte();
                    sprintf(fieldByte,"%d",field_No+1);
                    temp = strlen((const char *)fieldByte);
                    lcdWriteStringIndex(fieldByte,temp);
#endif
                    /***************************/
                    //sendSms(SmsDR4, userMobileNo, fieldNoRequired); // Acknowledge user about dry run detected and action taken
#ifdef SMS_DELIVERY_REPORT_ON_H
                    sleepCount = 2; // Load sleep count for SMS transmission action
                    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                    //setBCDdigit(0x05, 0);
                    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                    //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
                    /***************************/
                }
            }
        }
    }
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("doLoraSlaveInActiveAction_OUT\r\n");
    //********Debug log#end**************//
#endif 
}

/*********** Lora Slave InActive Action#End********/

/*********** DRY RUN Action#Start********/

/*************************************************************************************************************************

This function is called to perform actions after detecting dry run condition.
After detecting Dry run condition, stop all active valves and set all valves due from today to tomorrow.
Notify user about all actions

 **************************************************************************************************************************/

void doDryRunAction(void) {
    unsigned char field_No = CLEAR;
    unsigned int sleepCountVar = CLEAR;
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("doDryRunAction_IN\r\n");
    //********Debug log#end**************//
#endif
    __delay_ms(100);
    fetchTimefromRTC(); // Get today's date
    __delay_ms(100);
    getDueDate(1); // calculate next day date										 
    for (field_No = 0; field_No < fieldCount; field_No++) {
        if (fieldValve[field_No].status == ON) {
            __delay_ms(100);
            powerOffMotor();
            __delay_ms(1000);
            deActivateValve(field_No); // Deactivate Valve upon Dry run condition and reset valve to next due time
            valveDue = false;
            fieldValve[field_No].status = OFF;
            fieldValve[field_No].cyclesExecuted = fieldValve[field_No].cycles;
            __delay_ms(100);
            saveIrrigationValveOnOffStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
            __delay_ms(100);
            saveIrrigationValveCycleStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
            __delay_ms(100);
            sleepCountVar = readActiveSleepCountFromEeprom();
            if (fieldValve[field_No].isFertigationEnabled) {
                /************Fertigation switch off due to dry run***********/
                if (fieldValve[field_No].fertigationStage == injectPeriod) {
                    Fert_Motor = OFF; // Switch off fertigation valve in case it is ON
                    //Switch off all Injectors after completing fertigation on Period
                    Fert_Inj1 = OFF;
                    Fert_Inj2 = OFF;
                    Fert_Inj3 = OFF;
                    Fert_Inj4 = OFF;
                    fieldValve[field_No].fertigationStage = OFF;
                    fieldValve[field_No].fertigationValveInterrupted = true;
                    remainingFertigationOnPeriod = readActiveSleepCountFromEeprom();
                    __delay_ms(100);
                    saveRemainingFertigationOnPeriod();
                    __delay_ms(100);
                    saveFertigationValveStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                    __delay_ms(100);
                    /******** Calculate and save Field Valve next date**********/
                    fieldValve[field_No].nextDueDD = (unsigned char) dueDD;
                    fieldValve[field_No].nextDueMM = dueMM;
                    fieldValve[field_No].nextDueYY = dueYY;
                    __delay_ms(100);
                    saveIrrigationValveDueTimeIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                    __delay_ms(100);
                    
#ifdef LCD_DISPLAY_ON_H
                    lcdClearLine(2);
                    lcdClearLine(3);
                    lcdClearLine(4);				
                    lcdWriteStringAtCenter("Dry Run detected", 2);
                    lcdWriteStringAtCenter("Ferti. Postponed", 3);
                    lcdWriteStringAtCenter("For Field No:", 4);
                    lcdSetCursor(4,17);
                    clearFieldByte();
                    sprintf(fieldByte,"%d",field_No+1);
                    temp = strlen((const char *)fieldByte);
                    lcdWriteStringIndex(fieldByte,temp);
#endif
                    /***************************/
                    publishNotification("Dry Run Alert",NotDR1_84,true); // Acknowledge user about successful Authentication
                    /***************************/
                    /***************************/
                    //sendSms(SmsDR1, userMobileNo, fieldNoRequired); // Acknowledge user about dry run detected and action taken
#ifdef SMS_DELIVERY_REPORT_ON_H
                    sleepCount = 2; // Load sleep count for SMS transmission action
                    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                    //setBCDdigit(0x05, 0);
                    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                    //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
                    /***************************/
                } else if (fieldValve[field_No].fertigationStage == wetPeriod) {
                    /******** Calculate and save Field Valve next date**********/
                    fieldValve[field_No].nextDueDD = (unsigned char) dueDD;
                    fieldValve[field_No].nextDueMM = dueMM;
                    fieldValve[field_No].nextDueYY = dueYY;
                    __delay_ms(100);
                    saveIrrigationValveDueTimeIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                    __delay_ms(100);
                    
#ifdef LCD_DISPLAY_ON_H
                    lcdClearLine(2);
                    lcdClearLine(3);
                    lcdClearLine(4);				
                    lcdWriteStringAtCenter("Dry Run detected", 2);
                    lcdWriteStringAtCenter("Ferti. Postponed", 3);
                    lcdWriteStringAtCenter("For Field No:", 4);
                    lcdSetCursor(4,17);
                    clearFieldByte();
                    sprintf(fieldByte,"%d",field_No+1);
                    temp = strlen((const char *)fieldByte);
                    lcdWriteStringIndex(fieldByte,temp);
#endif
                    /***************************/
                    publishNotification("Dry Run Alert",NotDR2_84,true); // Acknowledge user about successful Authentication
                    /***************************/
                    /***************************/
                    //sendSms(SmsDR2, userMobileNo, fieldNoRequired); // Acknowledge user about dry run detected and action taken
#ifdef SMS_DELIVERY_REPORT_ON_H
                    sleepCount = 2; // Load sleep count for SMS transmission action
                    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                    //setBCDdigit(0x05, 0);
                    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                    //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
                    /***************************/
                }
            } else {
                if (sleepCountVar > (fieldValve[field_No].onPeriod / 2)) { // major part of valve execution is pending hence shift to next day
                    /******** Calculate and save Field Valve next date**********/
                    fieldValve[field_No].nextDueDD = (unsigned char) dueDD;
                    fieldValve[field_No].nextDueMM = dueMM;
                    fieldValve[field_No].nextDueYY = dueYY;
                    __delay_ms(100);
                    saveIrrigationValveDueTimeIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                    __delay_ms(100);
                    
#ifdef LCD_DISPLAY_ON_H
                    lcdClearLine(2);
                    lcdClearLine(3);
                    lcdClearLine(4);				
                    lcdWriteStringAtCenter("Dry Run detected", 2);
                    lcdWriteStringAtCenter("Irri. Postponed", 3);
                    lcdWriteStringAtCenter("For Field No:", 4);
                    lcdSetCursor(4,17);
                    clearFieldByte();
                    sprintf(fieldByte,"%d",field_No+1);
                    temp = strlen((const char *)fieldByte);
                    lcdWriteStringIndex(fieldByte,temp);
#endif
                    /***************************/
                    publishNotification("Dry Run Alert",NotDR3_67,true); // Acknowledge user about successful Authentication
                    /***************************/
                    /***************************/
                    //sendSms(SmsDR3, userMobileNo, fieldNoRequired); // Acknowledge user about dry run detected and action taken
#ifdef SMS_DELIVERY_REPORT_ON_H
                    sleepCount = 2; // Load sleep count for SMS transmission action
                    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                    //setBCDdigit(0x05, 0);
                    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                    //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
                    /***************************/
                } else { // next due date
                    
#ifdef LCD_DISPLAY_ON_H
                    lcdClearLine(2);
                    lcdClearLine(3);
                    lcdClearLine(4);				
                    lcdWriteStringAtCenter("Dry Run detected", 2);
                    lcdWriteStringAtCenter("Irri. Rescheduled", 3);
                    lcdWriteStringAtCenter("For Field No:", 4);
                    lcdSetCursor(4,17);
                    clearFieldByte();
                    sprintf(fieldByte,"%d",field_No+1);
                    temp = strlen((const char *)fieldByte);
                    lcdWriteStringIndex(fieldByte,temp);
#endif
                     /***************************/
                    publishNotification("Dry Run Alert",NotDR4_72,true); // Acknowledge user about successful Authentication
                    /***************************/
                    /***************************/
                    //sendSms(SmsDR4, userMobileNo, fieldNoRequired); // Acknowledge user about dry run detected and action taken
#ifdef SMS_DELIVERY_REPORT_ON_H
                    sleepCount = 2; // Load sleep count for SMS transmission action
                    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                    //setBCDdigit(0x05, 0);
                    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                    //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
                    /***************************/
                }
            }
            /*
            if (Phase_Input) {
#ifdef LCD_DISPLAY_ON_H
                lcdClear();
                lcdWriteStringAtCenter("Phase Loss Detected", 2);
#endif

                sendSms(SmsPh3, userMobileNo, noInfo); // Acknowledge user about Phase failure detected and action taken
#ifdef SMS_DELIVERY_REPORT_ON_H
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                //setBCDdigit(0x05, 0);
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif

            }
            else {
#ifdef LCD_DISPLAY_ON_H
                lcdClear();
                lcdWriteStringAtCenter("All Phase Detected", 2);
#endif

                sendSms(SmsPh6, userMobileNo, noInfo); // Acknowledge user about Phase failure detected and action taken
#ifdef SMS_DELIVERY_REPORT_ON_H
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                //setBCDdigit(0x05, 0);
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif

            }*/

        } else if ((currentDD == fieldValve[field_No].nextDueDD && currentMM == fieldValve[field_No].nextDueMM && currentYY == fieldValve[field_No].nextDueYY)) { // Shift all valves due today to next  date
            /******** Calculate and save Field Valve next date**********/
            fieldValve[field_No].nextDueDD = (unsigned char) dueDD;
            fieldValve[field_No].nextDueMM = dueMM;
            fieldValve[field_No].nextDueYY = dueYY;
            fieldValve[field_No].cyclesExecuted = fieldValve[field_No].cycles;
            __delay_ms(100);
            saveIrrigationValveDueTimeIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
            __delay_ms(100);
            saveIrrigationValveCycleStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
            __delay_ms(100);
            /***********************************************/
        }
    }
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("doDryRunAction_OUT\r\n");
    //********Debug log#end**************//
#endif 
}

/*********** DRY RUN Action#End********/

/*********** Low Phase Action#Start********/

/*************************************************************************************************************************

This function is called to perform actions after detecting low phase condition.
After detecting low phase condition, stop all active valves and set them due when phase current recovers
Notify user about all actions

 **************************************************************************************************************************/

void doLowPhaseAction(void) {
    unsigned char field_No = CLEAR;
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("dolowPhaseAction_IN\r\n");
    //********Debug log#end**************//
#endif
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdClearLine(4);				
    lcdWriteStringAtCenter("Low Phase Current", 2); 
    lcdWriteStringAtCenter("Suspending Actions", 3);
    lcdWriteStringAtCenter("Restart System", 3);
#endif
    /***************************/
    //sendSms(SmsPh2, userMobileNo, noInfo); // Acknowledge user about low phase current
#ifdef SMS_DELIVERY_REPORT_ON_H
    sleepCount = 2; // Load sleep count for SMS transmission action
    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
    //setBCDdigit(0x05, 0);
    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
    //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
    /***************************/
    if (valveDue) {
        for (field_No = 0; field_No < fieldCount; field_No++) {
            if (fieldValve[field_No].status == ON) {
                powerOffMotor();
                __delay_ms(1000);
                deActivateValve(field_No); // Deactivate Valve upon phase failure condition and reset valve to next due time
                /************Fertigation switch off due to low phase detection***********/
                if (fieldValve[field_No].fertigationStage == injectPeriod) {
                    Fert_Motor = OFF; // Switch off fertigation valve in case it is ON
#ifdef LCD_DISPLAY_ON_H
                    removeIcon(fert_icon);
#endif	  
                    //Switch off all Injectors after completing fertigation on Period
                    Fert_Inj1 = OFF;
                    Fert_Inj2 = OFF;
                    Fert_Inj3 = OFF;
                    Fert_Inj4 = OFF;

                   
#ifdef LCD_DISPLAY_ON_H   
                    lcdClearLine(2);
                    lcdClearLine(3);
                    lcdClearLine(4);				
                    lcdWriteStringAtCenter("Fertigation Stopped", 2);
                    lcdWriteStringAtCenter("For Field No.", 3);
                    lcdSetCursor(3,17);
                    clearFieldByte();
                    sprintf(fieldByte,"%d",field_No+1);
                    temp = strlen((const char *)fieldByte);
                    lcdWriteStringIndex(fieldByte,temp);
#endif
                    /***************************/
                    publishNotification("Fertigation Alert",NotFert6_33,true); // Acknowledge user about successful Authentication
                    /***************************/
                    /***************************/
                    //sendSms(SmsFert6, userMobileNo, fieldNoRequired); // Acknowledge user about successful Fertigation stopped action due to low phase detection
#ifdef SMS_DELIVERY_REPORT_ON_H
                    sleepCount = 2; // Load sleep count for SMS transmission action
                    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                    //setBCDdigit(0x05, 0);
                    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider           
                    //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
                    /***************************/
                }
            }
        }
    }
    phaseFailureActionTaken = true;
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("dolowPhaseAction_OUT\r\n");
    //********Debug log#end**************//
#endif
}

/*********** Low Phase Action#End********/

/*********** Phase Failure Action#Start********/

/*************************************************************************************************************************

This function is called to perform actions after detecting phase failure condition.
After detecting phase failure condition, stop all active valves and set them due when phase recovers
Notify user about all actions

 **************************************************************************************************************************/

void doPhaseFailureAction(void) {
    unsigned char field_No = CLEAR;
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("doPhaseFailureAction_IN\r\n");
    //********Debug log#end**************//
#endif
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdClearLine(4);				
    lcdWriteStringAtCenter("Phase Loss Detected", 2); 
    lcdWriteStringAtCenter("Suspending Actions", 3);
#endif
    /***************************/
    //sendSms(SmsPh1, userMobileNo, noInfo); // Acknowledge user about phase failure
#ifdef SMS_DELIVERY_REPORT_ON_H
    sleepCount = 2; // Load sleep count for SMS transmission action
    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
    //setBCDdigit(0x05, 0);
    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
    //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
    /***************************/
    if (valveDue) {
        for (field_No = 0; field_No < fieldCount; field_No++) {
            if (fieldValve[field_No].status == ON) {
                powerOffMotor();
                __delay_ms(1000);
                deActivateValve(field_No); // Deactivate Valve upon phase failure condition and reset valve to next due time
                /************Fertigation switch off due to Phase failure***********/
                if (fieldValve[field_No].fertigationStage == injectPeriod) {
                    Fert_Motor = OFF; // Switch off fertigation valve in case it is ON
#ifdef LCD_DISPLAY_ON_H
                    removeIcon(fert_icon);
#endif	  
                    //Switch off all Injectors after completing fertigation on Period
                    Fert_Inj1 = OFF;
                    Fert_Inj2 = OFF;
                    Fert_Inj3 = OFF;
                    Fert_Inj4 = OFF;

                    
#ifdef LCD_DISPLAY_ON_H   
                    lcdClearLine(2);
                    lcdClearLine(3);
                    lcdClearLine(4);				
                    lcdWriteStringAtCenter("Fertigation Stopped", 2);
                    lcdWriteStringAtCenter("For Field No.", 3);
                    lcdSetCursor(3,17);
                    clearFieldByte();
                    sprintf(fieldByte,"%d",field_No+1);
                    temp = strlen((const char *)fieldByte);
                    lcdWriteStringIndex(fieldByte,temp);
#endif
                    /***************************/
                    publishNotification("Fertigation Alert",NotFert6_33,true); // Acknowledge user about successful Authentication
                    /***************************/
                    /***************************/
                    //sendSms(SmsFert6, userMobileNo, fieldNoRequired); // Acknowledge user about successful Fertigation stopped action due to PhaseFailure
#ifdef SMS_DELIVERY_REPORT_ON_H
                    sleepCount = 2; // Load sleep count for SMS transmission action
                    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                    //setBCDdigit(0x05, 0);
                    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider           
                    //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
                    /***************************/
                }
            }
        }
    }
    phaseFailureActionTaken = true;
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("doPhaseFailureAction_OUT\r\n");
    //********Debug log#end**************//
#endif
}

/*********** Phase Failure Action#End********/

/*********** RTC battery drained#Start********/

/*************************************************************************************************************************

This function is called to measure motor phase current to detect dry run condition.
The Dry run condition is measured in terms of voltage of CT connected to one of the phase of motor.
The motor phase current is high  and low  for Wet (load) and Dry (no load) condition respectively.
This leads the output of CT with high and low voltage.
For Dry condition, the CT voltage is high and for wet condition, the CT voltage is low.
Here ADC module is used to measure high/ low voltage of CT thereby identifying load/no load condition.

 **************************************************************************************************************************/

_Bool isRTCBatteryDrained(void) {
    unsigned int batteryVoltage = 0;
    unsigned int batteryVoltageCutoff = 0;//555; //~2.7 v
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("isRTCBatteryDrained_IN\r\n");
    //********Debug log#end**************//
#endif
	rtcBatteryLevelChecked = true;							  
    selectChannel(RTCchannel);
    RTC_Trigger = ENABLED;
    __delay_ms(50);
    batteryVoltage = getADCResult();
    RTC_Trigger = DISABLED;
    if (batteryVoltage <= batteryVoltageCutoff) {
        lowRTCBatteryDetected = true;
#ifdef LCD_DISPLAY_ON_H
        displayIcon(battery_icon);
#endif	  
        __delay_ms(100);
        saveRTCBatteryStatus();
        __delay_ms(100);
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("isRTCBatteryDrained_Yes_OUT\r\n");
        //********Debug log#end**************//
#endif
        return true;
    } else {
#ifdef LCD_DISPLAY_ON_H
        removeIcon(battery_icon);
#endif	  
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("isRTCBatteryDrained_No_OUT\r\n");
        //********Debug log#end**************//
#endif
        return false;
    }
}

/*********** RTC battery drained#End********/


/*********** Check RYB phase Detection#Start********/

/*************************************************************************************************************************

This function is called to detect phase failure condition.
The phase failure condition is measured by scanning three phase input lines.
Each phase line is converted into TTL signal using Digital signal comparator.high  and low for lost (out) and present (In) condition respectively.
The output of Comparator is high  and low for lost (out) and present (In) condition respectively.
For Phase failure condition, at least one comparator output is high and for Phase detected condition all comparator output is low.
Here Rising Edge and Falling Edge is used to detect high/ low Comparator Output.

 **************************************************************************************************************************/

_Bool phaseFailure(void) {
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("phaseFailure_IN\r\n");
    //********Debug log#end**************//
#endif
    if (!Phase_Input) { //All 3 phases are ON
        phaseFailureDetected = false;
#ifdef LCD_DISPLAY_ON_H
        removeIcon(phase_icon);
#endif	  
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("phaseFailure_No_OUT\r\n");
        //********Debug log#end**************//
#endif
        return false; //no Phase failure
    } else { // one phase is lost
        phaseFailureDetected = false; //true
#ifdef LCD_DISPLAY_ON_H
        displayIcon(phase_icon);
#endif	  
        phaseFailureActionTaken = false;
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("phaseFailure_Yes_OUT\r\n");
        //********Debug log#end**************//
#endif
        return false; //phase failure // true
    }
}

/*********** Motor Dry run condition#End********/


/*********** Motor Power On #Start********/

/*************************************************************************************************************************

This function is called to power on motor
The purpose of this function is to activate relays to Switch ON Motor
 *
 **************************************************************************************************************************/
void powerOnMotor(void) {
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("powerOnMotor_IN\r\n");
    //********Debug log#end**************//
#endif
    __delay_ms(100);
    Irri_Motor = ON;
#ifdef LCD_DISPLAY_ON_H
        displayIcon(irri_icon);
#endif	  
    Timer0Overflow = 0;
    T0CON0bits.T0EN = ON; // Start timer0 to initiate 1 min cycle
    if (filtrationEnabled) {
        filtrationCycleSequence = 1;
#ifdef LCD_DISPLAY_ON_H
        displayIcon(filt_icon);
#endif	  
    } else {
        filtrationCycleSequence = 99;
    }
    dryRunCheckCount = 0;
#ifdef STAR_DELTA_DEFINITIONS_H
    __delay_ms(500);
    Irri_MotorT = ON;
    __delay_ms(900);
    Irri_MotorT = OFF;
#endif

#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("powerOnMotor_OUT\r\n");
    //********Debug log#end**************//
#endif
}

/*********** Motor Power On#End********/


/*********** Motor Power Off #Start********/

/*************************************************************************************************************************

This function is called to power Off motor
The purpose of this function is de-activate relays to Switch OFF Motor

 **************************************************************************************************************************/

void powerOffMotor(void) {
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("powerOffMotor_IN\r\n");
    //********Debug log#end**************//
#endif
    T0CON0bits.T0EN = OFF; // stop timer0
    __delay_ms(100);
    Filt_Out1 = OFF; // switch off filtration  valve if it is ON
    __delay_ms(50);
    Filt_Out2 = OFF; // switch off filtration  valve if it is ON
    __delay_ms(50);
    Filt_Out3 = OFF; // switch off filtration  valve if it is ON
    __delay_ms(50);
    Irri_Motor = OFF; // switch off Motor
    __delay_ms(50);
#ifdef LCD_DISPLAY_ON_H
    removeIcon(irri_icon);
#endif
#ifdef LCD_DISPLAY_ON_H
    removeIcon(filt_icon);
#endif

#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("powerOffMotor_OUT\r\n");
    //********Debug log#end**************//
#endif
}

/*********** Motor Power Off#End********/


/*********** Field Valve Activation#Start********/

/*************************************************************************************************************************

This function is called to activate valve
The purpose of this function is to activate mentioned field valve and notify user about activation through SMS
	
 **************************************************************************************************************************/
void activateValve(unsigned char FieldNo) {
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("sendActivateValveCmdToLora_IN\r\n");
    //********Debug log#end**************//
#endif
    unsigned char action;
	currentFieldNo = FieldNo+1;	
    clearFieldByte();
	sprintf(fieldByte,"%d",FieldNo+1);
    temp = strlen((const char *)fieldByte);
    loraAttempt = 0;
    action = 0x00; // activate valve action
    do {
        sendCmdToLora(action, FieldNo);
    } while (loraAttempt < 2);
    if (!LoraConnectionFailed && loraAttempt == 99) { // Successful Valve Activation
        // check field no. of valve in action
        fieldValve[FieldNo].status = ON; //notify field valve status
        valveDue = true; // Set Valve ON status
        loraAliveCount = CLEAR;
        loraAliveCountCheck = CLEAR;
        isLoraSlaveAlive = true;
        __delay_ms(100);
        saveIrrigationValveOnOffStatusIntoEeprom(eepromAddress[FieldNo], &fieldValve[FieldNo]);
        __delay_ms(100);
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("Valve: ");
        transmitNumberToDebug(temporaryBytesArray, 2);
        transmitStringToDebug("\r\n");
        //********Debug log#end**************//
#endif

        if (moistureSensorFailed) {
            moistureSensorFailed = false;
#ifdef LCD_DISPLAY_ON_H        
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);				
            lcdWriteStringAtCenter("Irrigation Started", 2);
            lcdWriteStringAtCenter("With Sensor Failure", 3);
            lcdWriteStringAtCenter("For Field No:", 4);
            lcdSetCursor(4,17);
            //sprintf((char *)temporaryBytesArray,"%d",FieldNo+1);
            lcdWriteStringIndex(fieldByte,temp);
#endif
            /***************************/
            publishNotification("Irrigation Alert",NotMS1_57,true); // Acknowledge user about successful Authentication
            /***************************/
            /***************************/
            //sendSms(SmsMS1, userMobileNo, fieldNoRequired);
#ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            //setBCDdigit(0x05, 0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
            /***************************/
        } else {
#ifdef LCD_DISPLAY_ON_H        
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);				
            lcdWriteStringAtCenter("Irrigation Started", 2);
            //lcdWriteStringAtCenter("With No Response", 3);
            lcdWriteStringAtCenter("For Field No:", 3);
            lcdSetCursor(3,17);
            //sprintf(fieldByte,"%d",FieldNo+1);
            lcdWriteStringIndex(fieldByte,temp);
#endif
            /***************************/
            publishNotification("Irrigation Alert",NotIrr4_30,true); // Acknowledge user about successful Authentication
            /***************************/
            
            /***************************/
            //sendSms(SmsIrr4, userMobileNo, fieldNoRequired); // Acknowledge user about successful Irrigation started action
#ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            //setBCDdigit(0x05, 0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
            /***************************/
        }
    } else { //Skip current valve execution and go for next
        valveDue = false;
        fieldValve[FieldNo].status = OFF;
        fieldValve[FieldNo].cyclesExecuted = fieldValve[FieldNo].cycles;
        startFieldNo = FieldNo + 1; // scan for next field no.
        __delay_ms(100);
        getDueDate(fieldValve[FieldNo].offPeriod); // calculate next due date of valve
        __delay_ms(100);
        fieldValve[FieldNo].nextDueDD = (unsigned char) dueDD;
        fieldValve[FieldNo].nextDueMM = dueMM;
        fieldValve[FieldNo].nextDueYY = dueYY;
        __delay_ms(100);
        saveIrrigationValveOnOffStatusIntoEeprom(eepromAddress[FieldNo], &fieldValve[FieldNo]);
        __delay_ms(100);
        saveIrrigationValveCycleStatusIntoEeprom(eepromAddress[FieldNo], &fieldValve[FieldNo]);
        __delay_ms(100);
        saveIrrigationValveDueTimeIntoEeprom(eepromAddress[FieldNo], &fieldValve[FieldNo]);
        __delay_ms(100);

        /***************************/
        // for field no. 01 to 09
        /*
        if (FieldNo < 9) {
            temporaryBytesArray[0] = 48; // To store field no. of valve in action 
            temporaryBytesArray[1] = FieldNo + 49; // To store field no. of valve in action 
        }// for field no. 10 to 12
        else if (FieldNo > 8 && FieldNo < fieldCount) {
            temporaryBytesArray[0] = 49; // To store field no. of valve in action 
            temporaryBytesArray[1] = FieldNo + 39; // To store field no. of valve in action 
        }*/
        /***************************/        
#ifdef LCD_DISPLAY_ON_H        
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);				
        lcdWriteStringAtCenter("Irrigation Skipped", 2);
        lcdWriteStringAtCenter("With No Response", 3);
        lcdWriteStringAtCenter("For Field No:", 4);
        lcdSetCursor(4,17);
        //sprintf(fieldByte,"%d",FieldNo+1);
        lcdWriteStringIndex(fieldByte,temp);
#endif
        /***************************/
        publishNotification("Irrigation Alert",NotIrr8_48,true); // Acknowledge user about successful Authentication
        /***************************/
        /***************************/
        //sendSms(SmsIrr8, userMobileNo, fieldNoRequired); // Acknowledge user about Irrigation not started due to Lora connection failure						
#ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        //setBCDdigit(0x05, 0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
        /***************************/
    }
#ifdef DEBUG_MODE_ON_H    
    //********Debug log#start************//
    transmitStringToDebug("activateValve_OUT\r\n");
    //********Debug log#end**************//
#endif
}
/*********** Field Valve Activation#End********/


/*********** Field Valve De-Activation#Start********/

/*************************************************************************************************************************

This function is called to de-activate valve
The purpose of this function is to deactivate mentioned field valve and notify user about De-activation through SMS

 **************************************************************************************************************************/
void deActivateValve(unsigned char FieldNo) {
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("deActivateValve_IN for valve: ");
    //********Debug log#end**************//
#endif
    // check field no. of valve in action
    unsigned char action;
    loraAttempt = 0;
    action = 0x01;
    do {
        sendCmdToLora(action, FieldNo);
    } while (loraAttempt < 2);
    /***************************/
    // for field no. 01 to 09
	/*  
    if (FieldNo < 9) {
        temporaryBytesArray[0] = 48; // To store field no. of valve in action 
        temporaryBytesArray[1] = FieldNo + 49; // To store field no. of valve in action 
    }// for field no. 10 to 12
    else if (FieldNo > 8 && FieldNo < fieldCount) {
        temporaryBytesArray[0] = 49; // To store field no. of valve in action 
        temporaryBytesArray[1] = FieldNo + 39; // To store field no. of valve in action 
    }
    */ 
    /***************************/
    clearFieldByte();
	sprintf(fieldByte,"%d",FieldNo+1);
    temp = strlen((const char *)fieldByte);
    if (!LoraConnectionFailed && loraAttempt == 99) { // Successful Valve DeActivation

        /***************************/
#ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("Valve: ");
        transmitNumberToDebug(fieldByte, 2);
        transmitStringToDebug("\r\n");
        //********Debug log#end**************//
#endif
        /***************************/
#ifdef LCD_DISPLAY_ON_H        
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);				
        lcdWriteStringAtCenter("Irrigation Stopped", 2);
        lcdWriteStringAtCenter("Successfully", 3);
        lcdWriteStringAtCenter("For Field No:", 4);
        lcdSetCursor(4,17);
        //sprintf(fieldByte,"%d",FieldNo+1);
        lcdWriteStringIndex(fieldByte,temp);
#endif
        /***************************/
        publishNotification("Irrigation Alert",NotIrr5_30,true); // Acknowledge user about successful Authentication
        /***************************/
        //sendSms(SmsIrr5, userMobileNo, fieldNoRequired); // Acknowledge user about successful Irrigation stopped action
#ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        //setBCDdigit(0x05, 0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
        /***************************/        
    } else {
        /***************************/
#ifdef LCD_DISPLAY_ON_H        
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);				
        lcdWriteStringAtCenter("Irrigation Stopped", 2);
        lcdWriteStringAtCenter("With No Response", 3);
        lcdWriteStringAtCenter("For Field No:", 4);
        lcdSetCursor(4,17);
        //sprintf(fieldByte,"%d",FieldNo+1);
        lcdWriteStringIndex(fieldByte,temp);
#endif
        /***************************/
        publishNotification("Irrigation Alert",NotIrr9_48,true); // Acknowledge user about successful Authentication
        /***************************/
        //sendSms(SmsIrr9, userMobileNo, fieldNoRequired); // Acknowledge user about Irrigation stopped with Lora connection failure						
#ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        //setBCDdigit(0x05, 0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
        /***************************/       
    }
#ifdef DEBUG_MODE_ON_H    
    //********Debug log#start************//
    transmitStringToDebug("deActivateValve_OUT\r\n");
    //********Debug log#end**************//
#endif
}
/*********** Field Valve De-Activation#End********/


/********************Deep Sleep function#Start************************/

/*************************************************************************************************************************

This function is called to activate deep sleep mode
The purpose of this function is to go into sleep mode until it is interrupted by GSM or Sleep count is reached to 0

 **************************************************************************************************************************/
void deepSleep(void) {
    // check until sleep timer ends given sleep count
    while (sleepCount > 0 && !newSMSRcvd) {
        if (isERROR) {
            isERROR = false;
#ifdef LCD_DISPLAY_ON_H   
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);				
            lcdWriteStringAtCenter("Server Connection", 2);
            lcdWriteStringAtCenter("Error", 3);
#endif
            /*
            if (!isErrorActionTaken) {
                setGsmToLocalTime();
                configureGPRS();
                isErrorActionTaken = true;
            }   
            */ 
        } 
        if (phaseFailureDetected) {
#ifdef LCD_DISPLAY_ON_H   
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);				
            lcdWriteStringAtCenter("Phase Failure", 2);
            lcdWriteStringAtCenter("Detected", 3);
            __delay_ms(2500);
#endif
            if (!phaseFailureActionTaken) {
                doPhaseFailureAction();
            }
            sleepCount = 65500;
            //setBCDdigit(0x03, 0); // (3.) BCD Indication for Phase Failure Error
        } else if (Irri_Motor == ON) { // Motor is ON without any external/Internal interrupt
            saveActiveSleepCountIntoEeprom(); // Save current valve on time
            // check Motor Dry run condition after each sleep count
            if (dryRunCheckCount > 2) {
                if (isMotorInNoLoad()) {							
					if (dryRunDetected) {
						doDryRunAction();
					} else if (lowPhaseCurrentDetected) {
						doLowPhaseAction();
						sleepCount = 65500; // undefined sleep until phase comes back
					}
				}
            }
            if (!isLoraSlaveAlive) { // Lora Slave not alive after 2 min.
                doLoraSlaveInActiveAction();
            }
            if (Irri_Motor == ON) {
#ifdef LCD_DISPLAY_ON_H 
                lcdClearLine(2);
                lcdClearLine(3);
                lcdClearLine(4);				
                lcdWriteStringAtCenter("Irrigation Running", 2);
                lcdWriteStringAtCenter("For Field No: ", 3);
                lcdSetCursor(3,17);
                clearFieldByte();
                sprintf(fieldByte,"%d",currentFieldNo);
                temp = strlen((const char *)fieldByte);
                lcdWriteStringIndex(fieldByte,2);
#endif
            }
        } else if (dryRunDetected) {
#ifdef LCD_DISPLAY_ON_H 
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);				
            lcdWriteStringAtCenter("Dry Run", 2);
            lcdWriteStringAtCenter("Detected", 3);
            //setBCDdigit(0x0C, 0); // (u.) BCD Indication for Dry Run Detected Error
#endif
        } else if (lowPhaseCurrentDetected) {
#ifdef LCD_DISPLAY_ON_H 
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);				
            lcdWriteStringAtCenter("Found Low Phase", 2);
            lcdWriteStringAtCenter("Current", 3);
            //setBCDdigit(0x03, 0); // (3.) BCD Indication for Phase Failure Error
#endif
        } else if (lowRTCBatteryDetected) {
#ifdef LCD_DISPLAY_ON_H 
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);				
            lcdWriteStringAtCenter("Found Low RTC", 2);
            lcdWriteStringAtCenter("Battery", 3);
            //setBCDdigit(0x02, 0); // (2.) BCD Indication for RTC Battery Low Error
#endif
        } else if (systemAuthenticated) {
            isValveConfigured = false;
            for (unsigned char i = 0; i < fieldCount; i++) {
                if (fieldValve[i].isConfigured) {
                    isValveConfigured = true;
                    break;
                }
            }
            if (isValveConfigured) {
#ifdef LCD_DISPLAY_ON_H 
                lcdClearLine(2);
                lcdClearLine(3);
                lcdClearLine(4);
                lcdWriteStringAtCenter("Next Schedule Set On", 2);
                lcdWriteStringAtCenter("Date:", 3);
                lcdWriteStringAtCenter(dueDate, 4);
#endif
            }
            else {
#ifdef LCD_DISPLAY_ON_H 
                lcdClearLine(2);
                lcdClearLine(3);
                lcdClearLine(4);
                lcdWriteStringAtCenter("No Configuration", 2);
                lcdWriteStringAtCenter("is Active", 3);
#endif    
            }      
        }
        Run_led = DARK; // Led Indication for system in Sleep/ Idle Mode
        inSleepMode = true; // Indicate in Sleep mode
        WDTCON0bits.SWDTEN = ENABLED; // Enable sleep mode timer
        if (sleepCount > 0 && !newSMSRcvd) {
#ifdef LCD_DISPLAY_ON_H
            displayIcon(clock_icon);
#endif
            Sleep(); // CPU sleep. Wakeup when Watchdog overflows, each of 16 Seconds if value of WDTPS is 4096
            
#ifdef LCD_DISPLAY_ON_H
            removeIcon(clock_icon);
#endif		   
        }
        if (lowBattery) { // low battery from slave
#ifdef LCD_DISPLAY_ON_H
            displayIcon(battery_icon);
#endif
            /***************************/
            publishNotification("null",NotSLV2_6,false); // Acknowledge user about successful Authentication
            /***************************/
            /***************************/
            publishNotification("Field Battery Alert",NotSLV1_39,true); // Acknowledge user about successful Authentication
            /***************************/
            lowBattery = false;
        } else if (resetSlave) {
            if (!lowRTCBatteryDetected) {
#ifdef LCD_DISPLAY_ON_H
                removeIcon(battery_icon);
#endif
            }
            /***************************/
            publishNotification("null",NotSLV3_7,false); // Acknowledge user about successful Authentication
            /***************************/
            /***************************/
            publishNotification("Field Battery Alert",NotSLV4_38,true); // Acknowledge user about successful Authentication
            /***************************/
            resetSlave = false;   
        }
        if (valveDue) {
            __delay_ms(1500); // compensate for new sms when valve is active
        }
        WDTCON0bits.SWDTEN = DISABLED; //turn off sleep mode timer
        //Run_led = GLOW; // Led Indication for system in Operational Mode
        if (!valveDue && !phaseFailureDetected && !lowPhaseCurrentDetected) {
            sleepCount--; // Decrement sleep count after every sleep cycle
        }
    }
    if (sleepCount == 0 && !newSMSRcvd) {
        __delay_ms(2000); // To compensate incoming SMS if valve is due within 10 minutes
    }
    inSleepMode = false; // Indicate not in sleep mode
}
/********************Deep Sleep function#End************************/


/*************Initialize#Start**********/

/*************************************************************************************************************************

This function is called to initialized system
The purpose of this function is to define port lines, interrupt priorities and to configure Timer and UART module.

 **************************************************************************************************************************/
void configureController(void) {

    BSR = 0x0f; // Set BSR for Banked SFR
    
    LATA = 0x00; // Set all output bits to zero for PORTA
    TRISA = 0b11100000; // Set RA<0:4> --Output~MUX Control lines, RA<5> --Input~ <Mux i/p>, RB<6:7> --Inputs~ <OSC1-2>
    ANSELA = 0b00100000; // RA<0:4,6:7> --Digital, RA<5> -- Analog
    WPUA = 0x00; //Weak Pull-up disabled
    ODCONA = 0x00; //Output drives both high-going and low-going signals (source and sink current)
    SLRCONA = 0xFF; //Port pin slew rate is limited
    INLVLA = 0xFF; //ST input used for port reads and interrupt-on-change
    
    LATB = 0x00; // Set all output bits to zero for PORTB
    TRISB = 0b11000111; // Set RB<0> --input~3ph interrupt, RB<1:2> --input~ <Temp,windspeed sensor>, RB<3:5> --outputs~ <Filtration1-3>,  RB<6:7> --input~ <PH,EC sensor>
    ANSELB = 0b11000110; // RB<0,3:5> -- Digital, RB<1:2,6:7> --Analog
    WPUB = 0x00; //Weak Pull-up Disabled
    ODCONB = 0x00; //Output drives both high-going and low-going signals (source and sink current)
    SLRCONB = 0xFF; //Port pin slew rate is limited
    INLVLB = 0xFF; //ST input used for port reads and interrupt-on-change
    IOCBN = 0b00000001; //Interrupt-on-Change Negative Edge Enable bits for RB0 --RYB phase
    IOCBP = 0b00000001; //Interrupt-on-Change Positive Edge Enable bits for RB0 --RYB phase
    IOCBF = 0b00000000; // Clear all initial IOC flags
    PIE0bits.IOCIE = ENABLED; //Peripheral Interrupt-on-Change Enabled

    LATC = 0x00; // Set all output bits to zero for PORTC
    TRISC = 0b10111000; // RC<0:2> --Outputs~<Filtration4:6>, RC<3:4> --Output~<SCL,SDA(LCD)>, RC<5> --Input~<Moisture sensor>, RC<6> --Output~TX1 (LORA), RC<7> --Input~ RX1 (LORA)
    WPUC = 0b00011000; ///* Set pull-up resistors for RC3 and RC4 */
    ODCONC = 0x00; //Output drives both high-going and low-going signals (source and sink current)
    SLRCONC = 0xFF; //Port pin slew rate is limited
    INLVLC = 0xFF; //ST input used for port reads and interrupt-on-change

    LATD = 0x00; // Set all output bits to zero for PORTD
    TRISD = 0b01100011; // RD<0:1> --Input~ <CT,RTCbattery>, RD<2:4,7> --Outputs~<IrrControlLines1:3,4>,  RD<5:6> --Outputs~ <SCL,SDA(RTC)>
    ANSELD = 0b00000011; // RD<0:1> --Analog , RD<2:7> --Digital 
    WPUD = 0x00; //Weak Pull-up disabled
    ODCOND = 0x00; //Output drives both high-going and low-going signals (source and sink current)
    SLRCOND = 0xFF; //Port pin slew rate is limited
    INLVLD = 0xFF; //ST input used for port reads and interrupt-on-change

    LATE = 0x00; // Set all output bits to zero for PORTE
    TRISE = 0b00001110; // RE<0> --Output~ <TX3(GSM)>, RE<1> --Input~ <RX3(GSM)>, RE<2:3> --Input~ <Water,Fertflowsensor>, RE<4:6> --Output~ <FertInj1:3>, RE<7> --Output~ <IrrControlLine5>
    ANSELE = 0b00000000; // RE<0:7> --Digital
    WPUE = 0x00; //Weak Pull-up disabled //Weak Pull-up disabled
    ODCONE = 0x00; //Output drives both high-going and low-going signals (source and sink current)
    SLRCONE = 0xFF; //Port pin slew rate is limited
    INLVLE = 0xFF; //ST input used for port reads and interrupt-on-change
    IOCEN = 0b00001100; //Interrupt-on-Change Negative Edge Enable bits for RE2 and RE3 --Irrigation and Fertigation Flow Sensor
    IOCEP = 0b00001100; //Interrupt-on-Change Positive Edge Enable bits for RE2 and RE3 --Irrigation and Fertigation Flow Sensor
    IOCEF = 0b00000000; // Clear all initial IOC flags
    PIE0bits.IOCIE = ENABLED; //Peripheral Interrupt-on-Change Enabled

    LATF = 0x00; // Set all output bits to zero for PORTF
    TRISF = 0x00; // RF<0:7> --Output~<IrrControlLines6:13>
    ANSELF = 0x00; // RF<0:7> --Digital 
    WPUF = 0x00; //Weak Pull-up disabled
    ODCONF = 0x00; //Output drives both high-going and low-going signals (source and sink current)
    SLRCONF = 0xFF; //Port pin slew rate is limited
    INLVLF = 0xFF; //ST input used for port reads and interrupt-on-change

    LATG = 0x00; // Set all output bits to zero for PORTG
    TRISG = 0b00100110; // RG<0,3> --Outputs~ <RTC trg, FertInj4>, RG<1:2> --Inputs~ <Prssure sensor1:2>, RG<4,6:7> --Outputs~ <IrrControlLines14:16>, RG<5> --Input~ MCLR
    ANSELG = 0b00000110; // RG<0,3:7> --Digital, RG<1:2> --Analog
    WPUG = 0x00; //Weak Pull-up disabled
    ODCONG = 0x00; //Output drives both high-going and low-going signals (source and sink current)
    SLRCONG = 0xFF; //Port pin slew rate is limited
    INLVLG = 0xFF; //ST input used for port reads and interrupt-on-change

    LATH = 0x00; // Set all output bits to zero for PORTH
    TRISH = 0b00000000; // Set RH<0:3> as outputs: RH<0> -> MotorPin1, RH<1> -> MotorPin2, RH<2> -> Fertigation Motor, RH<3> -> RUN LED
    WPUH = 0x00; //Weak Pull-up disabled
    ODCONH = 0x00; //Output drives both high-going and low-going signals (source and sink current)
    SLRCONH = 0xFF; //Port pin slew rate is limited
    INLVLH = 0xFF; //ST input used for port reads and interrupt-on-change


    //-----------ADC_Config-----------------------//

    ADREF = 0b00000000; // Reference voltage set to VDD and GND
    ADCON1 = 0X00;
    ADCON2 = 0X00;
    ADCON3 = 0X00;
    ADACQ = 0X00;
    ADCAP = 0X00;
    ADRPT = 0X00;
    ADACT = 0X00;
    ADCON0bits.ADFM = 1; // ADC results Format -- ADRES and ADPREV data are right-justified
    ADCON0bits.ADCS = 1; // ADC Clock supplied from FRC dedicated oscillator
    ADCON0bits.ADON = 1; //ADC is enabled
    ADCON0bits.ADCONT = 0; //ADC Continuous Operation is disabled

    //-----------Timer0_Config (60 sec) used for SLEEP Count control during Motor ON period and to control filtration  cycle sequence followup----------------------//
    //-----------Timer will not halt in sleep mode------------------------------------------------------//

    T0CON0 = 0b00010000; // 16 bit Timer 
    T0CON1 = 0b10011000; // Asynchronous with LFINTOSC 31KHZ as clock source with prescalar 1:256
    TMR0H = 0xE3; // Load Timer0 Register Higher Byte 0xE390
    TMR0L = 0xB0; // Load Timer0 Register Lower Byte FFFF-(60*31K)/(256) = 0xE39F)
    PIR0bits.TMR0IF = CLEAR; // Clear Timer0 Overflow Interrupt at start
    PIE0bits.TMR0IE = ENABLED; // Enables the Timer0 Overflow Interrupt
    IPR0bits.TMR0IP = LOW; // Low Timer0 Overflow Interrupt Priority

    //-----------Timer1_Config used for calculation pulse width of moisture sensor output-----------//
    //-----------Timer will halt in sleep mode------------------------------------------------------//

    T1CON = 0b00000010; // 16 bit Timer with Synchronous mode
    TMR1H = CLEAR; // Clear Timer1 Register Higher Byte
    TMR1L = CLEAR; // Clear Timer1 Register Lower Byte
    TMR1CLK = 0b00000001; //  Clock source as FOSC/4
    PIR5bits.TMR1IF = CLEAR; // Clear Timer1 Overflow Interrupt at start
    PIE5bits.TMR1IE = ENABLED; // Enables the Timer1 Overflow Interrupt
    IPR5bits.TMR1IP = LOW; // Low Timer1 Overflow Interrupt Priority

    //-----------Config Timer2_Config used for PWM1,2,3,4 output-----------//
    //-----------Timer will halt in sleep mode------------------------------------------------------//

    /* TIMER2 clock source is FOSC/4 */
    T2CLKCONbits.CS = 1;
    /* TIMER2 counter reset */
    T2TMR = 0x00;
    /* TIMER2 ON, prescaler 1:32, postscaler 1:1 */
    T2CONbits.OUTPS = 0;
    T2CONbits.CKPS = 5; // 5--1:32  4--1:16  3--1:8  2--1:4 1--1:2 0--1:1
    T2CONbits.T2ON = 1;
    /* Configure the default 2 kHZ*/
    T2PR = 249;

    /* Configure PWM1 START */
    /* Configure CCP pin*/
    //RE4PPS = 0X05; // RE4 PWM output
    /* MODE PWM; EN enabled; FMT left_aligned */
    CCP1CONbits.MODE = 0x0C;
    CCP1CONbits.FMT = 1;
    CCP1CONbits.EN = 0; //1
    /* Selecting Timer 2 */
    CCPTMRS0bits.C1TSEL = 0;
    /* Configure the default 50 %duty cycle */
    CCPR1 = 31680;
    /* Configure PWM1 End */

    /* Configure PWM2 START */
    /* Configure CCP pin*/
    //RE5PPS = 0X06; // RE5 PWM output
    /* MODE PWM; EN enabled; FMT left_aligned */
    CCP2CONbits.MODE = 0x0C;
    CCP2CONbits.FMT = 1;
    CCP2CONbits.EN = 0; //1
    /* Selecting Timer 2 */
    CCPTMRS0bits.C2TSEL = 0;
    /* Configure the default 50 %duty cycle */
    CCPR2 = 31680;
    /* Configure PWM2 End */

    /* Configure PWM3 START */
    /* Configure CCP pin*/
    //RE6PPS = 0X07; // RE6 PWM output
    /* MODE PWM; EN enabled; FMT left_aligned */
    CCP3CONbits.MODE = 0x0C;
    CCP3CONbits.FMT = 1;
    CCP3CONbits.EN = 0; //1
    /* Selecting Timer 2 */
    CCPTMRS0bits.C3TSEL = 0;
    /* Configure the default 50 %duty cycle */
    CCPR3 = 31680;
    /* Configure PWM3 End */

    /* Configure PWM4 START */
    /* Configure CCP pin*/
    //RG3PPS = 0X08; // RG3 PWM output
    /* MODE PWM; EN enabled; FMT left_aligned */
    CCP4CONbits.MODE = 0x0C;
    CCP4CONbits.FMT = 1;
    CCP4CONbits.EN = 0; //1
    /* Selecting Timer 2 */
    CCPTMRS0bits.C4TSEL = 0;
    /* Configure the default 50 %duty cycle */
    CCPR4 = 31680;
    /* Configure PWM4 End */

    //-----------Timer3_Config (1 sec) used if command fails to respond within timer limit----------------------//
    //-----------Timer will halt in sleep mode------------------------------------------------------//

    T3CON = 0b00110010; // 16 bit Timer with synchronous mode with 1:8 pre scale
    TMR3CLK = 0b00000100; // Clock source as LFINTOSC
    TMR3H = 0xF0; // Load Timer3 Register Higher Byte 
    TMR3L = 0xDC; // Load Timer3 Register lower Byte 
    PIR5bits.TMR3IF = CLEAR; // Clear Timer3 Overflow Interrupt at start
    PIE5bits.TMR3IE = ENABLED; // Enables the Timer3 Overflow Interrupt
    IPR5bits.TMR3IP = LOW; // Low Timer3 Overflow Interrupt Priority

    //-----------UART1_Config PRODUCTION LORA-----------------------//
    TX1STA = 0b00100100; // 8 Bit Transmission Enabled with High Baud Rate
    RC1STA = 0b10010000; // 8 Bit Reception Enabled with Continuous Reception
    SP1BRG = 0x0681; // XTAL=16MHz, Fosc=64Mhz for SYNC=0 BRGH=1 BRG16=1 (Asynchronous high 16 bit baud rate) 9600
    RC7PPS = 0x17; //EUSART1 Receive
    RC6PPS = 0x0C; //EUSART1 Transmit
    temp = RC1REG; // Empty buffer
    BAUD1CON = 0b00001000; // 16 Bit Baud Rate Register used
    PIE3bits.RC1IE = ENABLED; // Enables the EUSART Receive Interrupt
    PIE3bits.TX1IE = DISABLED; // Disables the EUSART Transmit Interrupt
    IPR3bits.RC1IP = HIGH; // EUSART Receive Interrupt Priority


    //-----------UART3_Config PRODUCTION GSM/GPRS-----------------------//

    TX3STA = 0b00100100; // 8 Bit Transmission Enabled with High Baud Rate
    RC3STA = 0b10010000; // 8 Bit Reception Enabled with Continuous Reception
    SP3BRG = 0x0089; //0x0681; // XTAL=16MHz, Fosc=64Mhz for SYNC=0 BRGH=1 BRG16=1 (Asynchronous high 16 bit baud rate) 115200
    RE1PPS = 0x21; //EUSART3 Receive
    RE0PPS = 0x10; //EUSART3 Transmit
    temp = RC3REG; // Empty buffer
    BAUD3CON = 0b00001000; // 16 Bit Baud Rate Register used
    PIE4bits.RC3IE = ENABLED; // Enables the EUSART Receive Interrupt
    PIE4bits.TX3IE = DISABLED; // Disables the EUSART Transmit Interrupt
    IPR4bits.RC3IP = HIGH; // EUSART Receive Interrupt Priority

    //-----------I2C_Config_RTC-----------------------//

    SSP2STAT |= 0x80; //Slew Rate Disabled
    SSP2CON1 = 0b00101000; //Master mode 0x28
    SSP2DATPPS = 0x1D; //RD5<-MSSP2:SDA2;  
    SSP2CLKPPS = 0x1E; //RD6<-MSSP2:SCL2;
    RD5PPS = 0x1C; //RD5->MSSP2:SDA2;    
    RD6PPS = 0x1B; //RD6->MSSP2:SCL2;    
    SSP2ADD = 119; //DS1307 I2C address

    //-----------I2C_Config_LCD-----------------------//

    SSP1STAT |= 0x80; //Slew Rate Disabled
    SSP1CON1 = 0b00101000; //Master mode 0x28
    SSP1CON2 = 0x00;
    SSP1STAT = 0x00;
    /* PPS setting for using RC3 as SCL */
    SSP1CLKPPS = 0x13; //RC3<-MSSP2:SCL1;
    RC3PPS = 0x19; //RC3->MSSP2:SCL1; 

    /* PPS setting for using RC4 as SDA */
    SSP1DATPPS = 0x14; //RD5<-MSSP2:SDA1;   
    RC4PPS = 0x1A; //RC4->MSSP2:SDA1;    

    SSP1ADD = ((_XTAL_FREQ / 4) / I2C_BAUDRATE - 1); //For 100K Hz it is 0x9F

    //-----------Interrupt_Config---------------//

    OSCENbits.SOSCEN = DISABLED; //Secondary Oscillator is disabled
    INTCONbits.IPEN = ENABLED; // Enables Priority Levels on Interrupts
    INTCONbits.PEIE = ENABLED; // Enables all unmasked  peripheral interrupts
    INTCONbits.GIE = ENABLED; // Enables all unmasked Global interrupts
    CPUDOZEbits.IDLEN = ENABLED; //Device enters into Idle mode on SLEEP Instruction.
    __delay_ms(3000);
    __delay_ms(3000);
    __delay_ms(3000);
#ifdef LCD_DISPLAY_ON_H    
    lcdInit();
    lcdClear();
    lcdWriteStringAtCenter("Bhoomi Jalsandharan", 2);
    lcdWriteStringAtCenter("Udyami LLP", 3);
    __delay_ms(3000);
    lcdClear();
    lcdWriteStringAtCenter("WireLess Irrigation", 2);
    lcdWriteStringAtCenter("Control System", 3);
    __delay_ms(3000);
#endif
}
/*************Initialize#End**********/

/*************setFactoryPincode#Start**********/

/*************************************************************************************************************************

This function is called to set Factory pincode on loading program for first time
The purpose of this function is to generate and store one time factory pincode

 ***************************************************************************************************************************/
void setFactoryPincode(void) {
    readDeviceProgramStatusFromEeprom();
    __delay_ms(50);
    if (DeviceBurnStatus == false) {
        deviceIDFalg = true;
        DeviceBurnStatus = true;
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);
        lcdWriteStringAtCenter("Setting", 1);
        lcdWriteStringAtCenter("Factory PinCode", 2);
#endif
        randomPasswordGeneration();
        lcdClearLine(1);
        lcdWriteStringAtCenter(factryPswrd, 1);
        __delay_ms(3000);
        saveFactryPswrdIntoEeprom();
        saveDeviceProgramStatusIntoEeprom();
        saveDeviceIDIntoEeprom();
    }
}
/*************setFactoryPincode#End**********/

/*************checkResetType#Start**********/

/*************************************************************************************************************************

This function is called to check reset type on reset interrupt
The purpose of this function is to check which type of reset occurred

 ***************************************************************************************************************************/

unsigned char checkResetType(void) {
    unsigned char resetType = 0;
    // check if system power reset occurred
    if (!PCON0bits.nPOR || !PCON0bits.nRI || !PCON0bits.nRMCLR || !PCON0bits.nBOR || !PCON0bits.nRWDT || PCON0bits.STKOVF || PCON0bits.STKUNF) {
        if (!PCON0bits.nPOR || !PCON0bits.nBOR) {
            PCON0bits.nRMCLR = SET; // Reset reset on MCLR status
            PCON0bits.nRI = SET; // Reset reset on instruction status
            PCON0bits.nRWDT = SET; // Reset WDT status
            PCON0bits.STKOVF = CLEAR; // Reset Stack Overflow status
            PCON0bits.STKUNF = CLEAR; // Reset Stack underflow status
        }
        if (!PCON0bits.nPOR) {
            resetType = PowerOnReset;
            PCON0bits.nPOR = SET; // Reset power status
        } else if (!PCON0bits.nBOR) {
            resetType = LowPowerReset;
            PCON0bits.nBOR = SET; // Reset BOR status
        } else if (!PCON0bits.nRMCLR) {
            resetType = HardReset;
            PCON0bits.nRMCLR = SET; // Reset reset on MCLR status
        } else if (!PCON0bits.nRI) {
            resetType = SoftResest;
            PCON0bits.nRI = SET; // Reset reset on instruction status
        } else if (!PCON0bits.nRWDT) {
            resetType = WDTReset;
            PCON0bits.nRWDT = SET; // Reset WDT status
        } else if (PCON0bits.STKOVF || PCON0bits.STKUNF) {
            resetType = StackReset;
            PCON0bits.STKOVF = CLEAR; // Reset Stack Overflow status
            PCON0bits.STKUNF = CLEAR; // Reset Stack underflow status
        }
        return resetType;
    }
    return resetType;
}
/*************checkResetType#End**********/

/*************hardResetMenu#Start**********/

/*************************************************************************************************************************

This function is called to go in diagnostic modes on hard system reset
The purpose of this function is to perform selected diagnostic actions on System hard Reset.

 ***************************************************************************************************************************/

void hardResetMenu() {
#ifdef LCD_DISPLAY_ON_H
    lcdClear();
    lcdWriteStringAtCenter("System Running In", 2);
    lcdWriteStringAtCenter("Diagnostic Mode", 3);
    __delay_ms(2000);
#endif
    if (resetCount == 0) {
        for (iterator = 1; iterator < 10; iterator++) {
            resetCount++;
            saveResetCountIntoEeprom();
            switch (resetCount) {
                case 1:
#ifdef LCD_DISPLAY_ON_H
                    lcdClear();
                    lcdWriteStringAtCenter("Press Reset Button", 1);
                    lcdWriteStringAtCenter("To Calibrate Motor", 2);
                    lcdWriteStringAtCenter("Current In No Load", 3);
#endif
                    __delay_ms(3000);__delay_ms(3000);__delay_ms(3000);
                    break;
                case 2:
#ifdef LCD_DISPLAY_ON_H
                    lcdClear();
                    lcdWriteStringAtCenter("Press Reset Button", 1);
                    lcdWriteStringAtCenter("To Calibrate Motor", 2);
                    lcdWriteStringAtCenter("Current In Full Load", 3);
#endif
                    __delay_ms(3000);__delay_ms(3000);__delay_ms(3000);
                    break;
                case 3:
#ifdef LCD_DISPLAY_ON_H
                    lcdClear();
                    lcdWriteStringAtCenter("Press Reset Button", 1);
                    lcdWriteStringAtCenter("To Check GSM Signal", 2);
                    lcdWriteStringAtCenter("Network Strength", 3);
#endif
                    __delay_ms(3000);__delay_ms(3000);__delay_ms(3000);
                    break;
                case 4:
                    break;
                case 5:
                    break;
                case 6:
                    break;
                case 7:
#ifdef LCD_DISPLAY_ON_H
                    lcdClear();
                    lcdWriteStringAtCenter("***Factory Reset****", 1);
                    lcdWriteStringAtCenter("Press Reset Button", 2);
                    lcdWriteStringAtCenter("To Delete System", 3);
                    lcdWriteStringAtCenter("Complete Data", 4);
#endif
                    __delay_ms(3000);__delay_ms(3000);__delay_ms(3000);
                    break;
                case 8:
#ifdef LCD_DISPLAY_ON_H
                    lcdClear();
                    lcdWriteStringAtCenter("Press Reset Button", 1);
                    lcdWriteStringAtCenter("To Delete Irrigation", 2);
                    lcdWriteStringAtCenter("Data", 3);
#endif
                    __delay_ms(3000);__delay_ms(3000);__delay_ms(3000);
                    break;
                case 9:
#ifdef LCD_DISPLAY_ON_H
                    lcdClear();
                    lcdWriteStringAtCenter("Press Reset Button", 1);
                    lcdWriteStringAtCenter("To Delete User", 2);
                    lcdWriteStringAtCenter("Registration", 3);
#endif
                    __delay_ms(3000);__delay_ms(3000);__delay_ms(3000);
            }
        }
#ifdef LCD_DISPLAY_ON_H
        lcdClear();
        lcdWriteStringAtCenter("No Diagnostic Menu", 1);
        lcdWriteStringAtCenter("Selected", 2);
        lcdWriteStringAtCenter("Exiting Diagnostic", 3);
        lcdWriteStringAtCenter("Mode", 4);
        __delay_ms(3000);__delay_ms(3000);__delay_ms(3000);
#endif
        resetCount = 0x00;
        saveResetCountIntoEeprom();
    } else {
        switch (resetCount) {
            case 1:
                resetCount = 0x00;
                saveResetCountIntoEeprom();
#ifdef LCD_DISPLAY_ON_H
                lcdClear();
                lcdWriteStringAtCenter("Calibrating Motor", 2);
                lcdWriteStringAtCenter("In No Load Condition", 3);
#endif
                calibrateMotorCurrent(NoLoad, 0);
                Irri_Motor = OFF; //Manual procedure off
                msgIndex = CLEAR;
                /***************************/
                //sendSms(SmsMotor3, userMobileNo, motorLoadRequired);
#ifdef SMS_DELIVERY_REPORT_ON_H
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                //setBCDdigit(0x05, 0);
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
                /***************************/
#ifdef DEBUG_MODE_ON_H
                //********Debug log#start************//
                transmitStringToDebug("actionsOnSystemReset_1_OUT\r\n");
                //********Debug log#end**************//
#endif
                break;
            case 2:
                resetCount = 0x00;
                saveResetCountIntoEeprom();
#ifdef LCD_DISPLAY_ON_H
                lcdClear();
                lcdWriteStringAtCenter("Calibrating Motor", 2);
                lcdWriteStringAtCenter("Full Load Current", 3);
#endif
                calibrateMotorCurrent(FullLoad, 0);
                msgIndex = CLEAR;
                /***************************/
                //sendSms(SmsMotor3, userMobileNo, motorLoadRequired);
#ifdef SMS_DELIVERY_REPORT_ON_H
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                //setBCDdigit(0x05, 0);
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
                /***************************/
#ifdef DEBUG_MODE_ON_H
                //********Debug log#start************//
                transmitStringToDebug("actionsOnSystemReset_2_OUT\r\n");
                //********Debug log#end**************//
#endif
                break;
            case 3:
                resetCount = 0x00;
                saveResetCountIntoEeprom();
#ifdef LCD_DISPLAY_ON_H
                lcdClear();
                lcdWriteStringAtCenter("Checking GSM Signal", 2);
                lcdWriteStringAtCenter("Strength", 3);
                __delay_ms(3000);
#endif
                checkSignalStrength();
                break;
            case 4:
                resetCount = 0x00;
                saveResetCountIntoEeprom();
                for (iterator = 0; iterator < 5; iterator++) {
                    //setBCDdigit(0x0F, 1); // BCD Indication for Flash
                    __delay_ms(500);
                    //setBCDdigit(0x04, 1); // BCD Indication for Reset Action#n
                    __delay_ms(1000);
                }
                break;
            case 5:
                resetCount = 0x00;
                saveResetCountIntoEeprom();
                for (iterator = 0; iterator < 5; iterator++) {
                    //setBCDdigit(0x0F, 1); // BCD Indication for Flash
                    __delay_ms(500);
                    //setBCDdigit(0x05, 1); // BCD Indication for Reset Action#n
                    __delay_ms(1000);
                }
                break;
            case 6:
                resetCount = 0x00;
                saveResetCountIntoEeprom();
                for (iterator = 0; iterator < 5; iterator++) {
                    //setBCDdigit(0x0F, 1); // BCD Indication for Flash
                    __delay_ms(500);
                    //setBCDdigit(0x06, 1); // BCD Indication for Reset Action#n
                    __delay_ms(1000);
                }
                break;
            case 7:
                resetCount = 0x00;
                saveResetCountIntoEeprom();
                if (systemAuthenticated) {
#ifdef LCD_DISPLAY_ON_H
                    lcdClear();
                    lcdWriteStringAtCenter("Deleting", 1);
                    lcdWriteStringAtCenter("System Complete Data", 2);
                    lcdWriteStringAtCenter("Resting To", 3);
                    lcdWriteStringAtCenter("Factory Settings", 4);
                    __delay_ms(3000);
#endif
                    deleteValveData();
                    deleteUserData();
                    __delay_ms(1000);
                    loadDataFromEeprom(); // Read configured valve data saved in EEprom
                }
                break;
            case 8:
                resetCount = 0x00;
                saveResetCountIntoEeprom();
                if (systemAuthenticated) {
#ifdef LCD_DISPLAY_ON_H
                    lcdClear();
                    lcdWriteStringAtCenter("Deleting", 1);
                    lcdWriteStringAtCenter("Irrigation Data", 2);
                    __delay_ms(3000);
#endif
                    deleteValveData();
                    __delay_ms(1000);
                    loadDataFromEeprom(); // Read configured valve data saved in EEprom
                }
                break;
            case 9:
                resetCount = 0x00;
                saveResetCountIntoEeprom();
                if (systemAuthenticated) {
#ifdef LCD_DISPLAY_ON_H
                    lcdClear();
                    lcdWriteStringAtCenter("Deleting", 1);
                    lcdWriteStringAtCenter("User Registration", 2);
                    lcdWriteStringAtCenter("Data", 3);
                    __delay_ms(3000);
#endif
                    deleteUserData();
                    __delay_ms(1000);
                    loadDataFromEeprom(); // Read configured valve data saved in EEprom
                }
        }
        resetCount = 0x00;
        saveResetCountIntoEeprom();
    }
}

/*************hardResetMenu#End**********/

/*************actionsOnSystemReset#Start**********/

/*************************************************************************************************************************

This function is called to do actions on system reset
The purpose of this function is to perform actions on Power on reset or System hard Reset.

 ***************************************************************************************************************************/
void actionsOnSystemReset(void) {
    unsigned char resetType = CLEAR;
    /*
    unsigned int adcOutput = CLEAR;
    */ 
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("actionsOnSystemReset_IN\r\n");
    //********Debug log#end**************//
#endif
    configureController(); // set Microcontroller ports, ADC, Timer, I2C, UART, Interrupt Config
    __delay_ms(3000); //warming time
    __delay_ms(3000); // warming time
    __delay_ms(3000); //warming time
    __delay_ms(3000); // warming time
    __delay_ms(3000); // warming time
    setGsmToLocalTime();
    setFactoryPincode();
    resetType = checkResetType();
    /*
    if (resetType != HardReset) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);
        lcdWriteStringAtCenter("System is Booting Up", 2);
        for (unsigned char i = 0; i < 10; i++) {
            sprintf((char *)temporaryBytesArray, "%d%c", (i+1)*10,0x25);
            lcdWriteStringAtCenter((const char *)temporaryBytesArray, 3);
            //__delay_ms(2000);
        } 
#endif
    } else if (resetType == HardReset) {
        readResetCountFromEeprom();
        hardResetMenu(); 
    }
    */
     
	if(gsmSetToLocalTime) {
        getDateFromGSM(); // Get today's date from Network
        __delay_ms(100);
        feedTimeInRTC(); // Feed fetched date from network into RTC
        __delay_ms(100);
    }
    loadDataFromEeprom(); // Read configured valve data saved in EEprom
    //configureGPRS();
    configureMQTT(); // Configure GSM in TEXT mode
    //__delay_ms(1000); 					   
    //deleteMsgFromSIMStorage(); // Clear GSM storage memory for new Messages
    while(!deviceIdSet) {
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(1);
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);
        lcdWriteStringAtCenter("Waiting For", 2);
        lcdWriteStringAtCenter("DeviceID", 3);
#endif       
        controllerCommandExecuted = true;
        msgIndex = CLEAR;
        __delay_ms(3000);
        //strncpy(pwd, factryPswrd, 6); // consider factory password
        sleepCount = 65500; // Set Default Sleep count until next sleep count is calculated
        deepSleep(); // Sleep with default sleep count until system is configured
        // check if Sleep count executed with interrupt occurred due to new SMS command reception
        __delay_ms(200);
        if (newSMSRcvd) {
#ifdef LCD_DISPLAY_ON_H
            displayIcon(sms_icon);
#endif
            __delay_ms(500);
            newSMSRcvd = false; // received cmd is processed										
            extractReceivedSms(); // Read received SMS
            //deleteMsgFromSIMStorage();
        } 
    }
    if (deviceIDFalg) {
        lcdClearLine(1);
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);
        lcdWriteStringAtCenter("Resetting Controller", 2);
        lcdWriteStringAtCenter("For New Device ID", 3);
        __delay_ms(300);
        __delay_ms(300);
        Reset();
    }
    while (!systemAuthenticated) { // check if System not yet configured
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(1);
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);
        lcdWriteStringAtCenter(factryPswrd, 1);
        lcdWriteStringAtCenter("Waiting For", 2);
        lcdWriteStringAtCenter("Authentication", 3);
#endif
        /***************************/
        publishNotification("Factory Key",factryPswrd,false); // Acknowledge user about successful Authentication
        /***************************/
        controllerCommandExecuted = true;
        msgIndex = CLEAR;
        //strncpy(pwd, factryPswrd, 6); // consider factory password
        sleepCount = 65500; // Set Default Sleep count until next sleep count is calculated
        deepSleep(); // Sleep with default sleep count until system is configured
        // check if Sleep count executed with interrupt occurred due to new SMS command reception
        
        if (newSMSRcvd) {
#ifdef LCD_DISPLAY_ON_H
            displayIcon(sms_icon);
            __delay_ms(500);
#endif
            newSMSRcvd = false; // received cmd is processed										
            extractReceivedSms(); // Read received SMS
            //deleteMsgFromSIMStorage();
        }
    }
    if (systemAuthenticated) { // check if system is authenticated and valve action is due
        if (phaseFailure()) { // phase failure detected
            sleepCount = 65500;
#ifdef LCD_DISPLAY_ON_H
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);
            lcdWriteStringAtCenter("System Restarted With", 2);
            lcdWriteStringAtCenter("Phase Failure", 3); 
            lcdWriteStringAtCenter("Suspended All Actions", 4);
#endif
            /***************************/
            //sendSms(SmsSR01, userMobileNo, noInfo); // Acknowledge user about System restarted with phase failure
#ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission 
            //setBCDdigit(0x05, 0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider 
            //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
            phaseFailureActionTaken = true;
        } else {
            startFieldNo = 0;
            // check if System is configured
            for (iterator_field = 0; iterator_field < fieldCount; iterator_field++) { // scan all valves priority wise
                // check if any field valve status was true after reset
                if (fieldValve[iterator_field].status == ON) {
                    startFieldNo = iterator_field; // start action from interrupted field irrigation valve
                    //getDueDate(fieldValve[iterator].offPeriod); // calculate next due date of valve
                    fetchTimefromRTC();
                    /*** Check if System Restarted on next day of Due date ***/
                    // if year over passes ||  if month  over passes ||  if day over passes 
                    if ((currentYY > fieldValve[iterator_field].nextDueYY) || 
                            (currentMM > fieldValve[iterator_field].nextDueMM && 
                            currentYY == fieldValve[iterator_field].nextDueYY) || 
                            (currentDD > fieldValve[iterator_field].nextDueDD && 
                            currentMM == fieldValve[iterator_field].nextDueMM && 
                            currentYY == fieldValve[iterator_field].nextDueYY) || 
                            (currentHour > fieldValve[iterator_field].motorOnTimeHour && 
                            currentDD == fieldValve[iterator_field].nextDueDD && 
                            currentMM == fieldValve[iterator_field].nextDueMM && 
                            currentYY == fieldValve[iterator_field].nextDueYY)) {
                        valveDue = false; // Clear Valve Due
                        fieldValve[iterator_field].status = OFF;
                        fieldValve[iterator_field].cyclesExecuted = fieldValve[iterator_field].cycles;
                        if (fieldValve[iterator_field].isFertigationEnabled) {
                            if (fieldValve[iterator_field].fertigationStage == injectPeriod) {
                                fieldValve[iterator_field].fertigationStage = OFF;
                                fieldValve[iterator_field].fertigationValveInterrupted = true;
                                remainingFertigationOnPeriod = readActiveSleepCountFromEeprom();
                                saveRemainingFertigationOnPeriod();
                            } else if (fieldValve[iterator_field].fertigationStage == flushPeriod || fieldValve[iterator_field].fertigationStage == wetPeriod) {
                                fieldValve[iterator_field].fertigationStage = OFF;
                            }
                        }
                        __delay_ms(100);
#ifdef DEBUG_MODE_ON_H
                        //********Debug log#start************//
                        transmitStringToDebug("System restarted with Due valve on next day\r\n");
                        //********Debug log#end**************//
#endif
                        break;
                    } else { // if system restarted on same day with due valve
                        valveDue = true; // Set valve ON status
#ifdef DEBUG_MODE_ON_H 
                        //********Debug log#start************//
                        transmitStringToDebug("System restarted with Due valve on same day\r\n");
                        //********Debug log#end**************//
#endif
                        break;
                    }
                }
            }
            if (valveDue) {
                dueValveChecked = true;      
                clearFieldByte();
				sprintf(fieldByte,"%d",iterator_field+1);	
                temp = strlen((const char *)fieldByte);
                switch (resetType) {
                    case PowerOnReset:
#ifdef LCD_DISPLAY_ON_H
                    lcdClearLine(2);
                    lcdClearLine(3);
                    lcdClearLine(4);
                    lcdWriteStringAtCenter("System Restarted For", 2);
                    lcdWriteStringAtCenter("Power Interrupt", 3); 
                    lcdWriteStringAtCenter("For Field No.", 4);
                    lcdSetCursor(4,17);
                    lcdWriteStringIndex(fieldByte,temp);
#endif	   
                    //sendSms(SmsSR02, userMobileNo, fieldNoRequired); // Acknowledge user about system restarted with Valve action
                    break;
                    case LowPowerReset:
#ifdef LCD_DISPLAY_ON_H
                    lcdClearLine(2);
                    lcdClearLine(3);
                    lcdClearLine(4);
                    lcdWriteStringAtCenter("System Restarted For", 2);
                    lcdWriteStringAtCenter("Low Power", 3); 
                    lcdWriteStringAtCenter("For Field No.", 4);
                    lcdSetCursor(4,17);
                    lcdWriteStringIndex(fieldByte,temp);
#endif
                    //sendSms(SmsSR03, userMobileNo, fieldNoRequired); // Acknowledge user about system restarted with Valve action
                    break;
                case HardReset:
#ifdef LCD_DISPLAY_ON_H
                    lcdClearLine(2);
                    lcdClearLine(3);
                    lcdClearLine(4);
                    lcdWriteStringAtCenter("System Restarted For", 2);
                    lcdWriteStringAtCenter("Diagnostic Mode", 3); 
                    lcdWriteStringAtCenter("For Field No.", 4);
                    lcdSetCursor(4,17);
                    lcdWriteStringIndex(fieldByte,temp);
#endif
                    //sendSms(SmsSR04, userMobileNo, fieldNoRequired); // Acknowledge user about system restarted with Valve action
                    break;
                case SoftResest:
#ifdef LCD_DISPLAY_ON_H
                    lcdClearLine(2);
                    lcdClearLine(3);
                    lcdClearLine(4);
                    lcdWriteStringAtCenter("System Restarted For", 2);
                    lcdWriteStringAtCenter("Phase Detection", 3); 
                    lcdWriteStringAtCenter("For Field No.", 4);
                    lcdSetCursor(4,17);
                    lcdWriteStringIndex(fieldByte,temp);
#endif
                    //sendSms(SmsSR05, userMobileNo, fieldNoRequired); // Acknowledge user about system restarted with Valve action
                    break;
                case WDTReset:
#ifdef LCD_DISPLAY_ON_H
                    lcdClearLine(2);
                    lcdClearLine(3);
                    lcdClearLine(4);
                    lcdWriteStringAtCenter("System Restarted For", 2);
                    lcdWriteStringAtCenter("Timer Time OUT", 3); 
                    lcdWriteStringAtCenter("For Field No.", 4);
                    lcdSetCursor(4,17);
                    lcdWriteStringIndex(fieldByte,temp);
#endif
                    //sendSms(SmsSR06, userMobileNo, fieldNoRequired); // Acknowledge user about system restarted with Valve action
                    break;
                case StackReset:
#ifdef LCD_DISPLAY_ON_H
                    lcdClearLine(2);
                    lcdClearLine(3);
                    lcdClearLine(4);
                    lcdWriteStringAtCenter("System Restarted For", 2);
                    lcdWriteStringAtCenter("Stack Error", 3); 
                    lcdWriteStringAtCenter("For Field No.", 4);
                    lcdSetCursor(4,17);
                    lcdWriteStringIndex(fieldByte,temp);
#endif
                    //sendSms(SmsSR07, userMobileNo, fieldNoRequired); // Acknowledge user about system restarted with Valve action
                    break;
                }
                resetType = CLEAR;
                /***************************/
#ifdef SMS_DELIVERY_REPORT_ON_H
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission 
                //setBCDdigit(0x05, 0);
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
                /***************************/
#ifdef DEBUG_MODE_ON_H
                //********Debug log#start************//
                transmitStringToDebug("System restarted with Due valve\r\n");
                //********Debug log#end**************//
#endif
                sleepCount = readActiveSleepCountFromEeprom();
            } else { // check if no valve action is due
#ifdef LCD_DISPLAY_ON_H
                lcdClearLine(2);
                lcdClearLine(3);
                lcdClearLine(4);
                lcdWriteStringAtCenter("System Restarted", 2);
#endif
                switch (resetType) {
                    case PowerOnReset:
                        //sendSms(SmsSR08, userMobileNo, noInfo); // Acknowledge user about system restarted with Valve action
                        break;
                    case LowPowerReset:
                        //sendSms(SmsSR09, userMobileNo, noInfo); // Acknowledge user about system restarted with Valve action
                        break;
                    case HardReset:
                        //sendSms(SmsSR10, userMobileNo, noInfo); // Acknowledge user about system restarted with Valve action
                        break;
                    case SoftResest:
                        //sendSms(SmsSR11, userMobileNo, noInfo); // Acknowledge user about system restarted with Valve action
                        break;
                    case WDTReset:
                        //sendSms(SmsSR12, userMobileNo, noInfo); // Acknowledge user about system restarted with Valve action
                        break;
                    case StackReset:
                        //sendSms(SmsSR13, userMobileNo, noInfo); // Acknowledge user about system restarted with Valve action
                        break;
                }
                resetType = CLEAR;
                /***************************/
#ifdef SMS_DELIVERY_REPORT_ON_H
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission 
                //setBCDdigit(0x05, 0);
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider 
                //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
                /***************************/
#ifdef DEBUG_MODE_ON_H
                //********Debug log#start************//
                transmitStringToDebug("System restarted W/O Due Valve\r\n");
                //********Debug log#end**************//
#endif
            }
        }
    }
    if (isRTCBatteryDrained()) {
#ifdef LCD_DISPLAY_ON_H   
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);				
        lcdWriteStringAtCenter("RTC Battery is low", 2);
        lcdWriteStringAtCenter("Replace RTC battery", 3);
#endif  
        /***************************/
        publishNotification("null",NotRTC2_6,false); // Acknowledge user about successful Authentication
        /***************************/
        /***************************/
        publishNotification("RTC Battery Alert",NotRTC1_30,false); // Acknowledge user about successful Authentication
        /***************************/
        /***************************/
        //sendSms(SmsRTC1, userMobileNo, noInfo); // Acknowledge user about Please replace RTC battery
#ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        //setBCDdigit(0x05, 0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
        /***************************/
        if (gsmSetToLocalTime) {
            getDateFromGSM(); // Get today's date from Network
            __delay_ms(1000);
            feedTimeInRTC(); // Feed fetched date from network into RTC
            __delay_ms(1000);
        }
    } else if (lowRTCBatteryDetected) {
        lowRTCBatteryDetected = false;
        __delay_ms(100);
        saveRTCBatteryStatus();
        __delay_ms(100);
        if (gsmSetToLocalTime) {
            getDateFromGSM(); // Get today's date from Network
            __delay_ms(1000);
            feedTimeInRTC(); // Feed fetched date from network into RTC
            __delay_ms(1000);
#ifdef LCD_DISPLAY_ON_H   
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);				
            lcdWriteStringAtCenter("New RTCBattery Found", 2);
            lcdWriteStringAtCenter("System Time Synced", 3);
            lcdWriteStringAtCenter("To Local Time", 4);
#endif
            __delay_ms(1000);
            /***************************/
            publishNotification("null",NotRTC3_7,false); // Acknowledge user about successful Authentication
            /***************************/
            /***************************/
            publishNotification("RTC Battery Alert",NotRTC4_36,false); // Acknowledge user about successful Authentication
            /***************************/
            /***************************/
            //sendSms(SmsRTC3, userMobileNo, noInfo); // Acknowledge user about New RTC battery found, system time is set to local time
#ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            //setBCDdigit(0x05, 0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
            /***************************/
        } else {
#ifdef LCD_DISPLAY_ON_H   
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);				
            lcdWriteStringAtCenter("New RTCBattery Found", 2);
            lcdWriteStringAtCenter("Sync System Manually", 3);
            lcdWriteStringAtCenter("To Local Time", 4);
#endif
            __delay_ms(1000);
            /***************************/
            publishNotification("null",NotRTC3_7,false); // Acknowledge user about successful Authentication
            /***************************/
            /***************************/
            publishNotification("RTC Battery Alert",NotRTC4_36,false); // Acknowledge user about successful Authentication
            /***************************/
            /***************************/
            //sendSms(SmsRTC4, userMobileNo, noInfo); // Acknowledge user about New RTC battery found, please set system time manually to local time
#ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            //setBCDdigit(0x05, 0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
            /***************************/
        }
    } else {
        /***************************/
        publishNotification("null",NotRTC3_7,false); // Acknowledge user about successful Authentication
        /***************************/
    }
}
/*************actionsOnSystemReset#End**********/

/*************actionsOnSleepCountFinish#Start**********/

/*************************************************************************************************************************

This function is called to do actions after completing sleep count
The purpose of this function is to perform actions after awaking from deep sleep.

 ***************************************************************************************************************************/
void actionsOnSleepCountFinish(void) {
    unsigned char field_No = CLEAR;
    if (valveDue && sleepCount == 0 && !dryRunDetected && !phaseFailureDetected && !onHold && !lowPhaseCurrentDetected) {
        for (field_No = 0; field_No < fieldCount; field_No++) {
            // upon completing first delay start period sleep , switch ON fertigation valve
            if (fieldValve[field_No].status == ON && fieldValve[field_No].isFertigationEnabled && fieldValve[field_No].fertigationStage == wetPeriod && isLoraSlaveAlive) {
                __delay_ms(1000);
                Fert_Motor = ON; // switch on fertigation valve for given field after start period 
#ifdef LCD_DISPLAY_ON_H
                displayIcon(fert_icon);
#endif	  
                // Injector code             
                // Initialize all count to zero
                injector1OnPeriodCnt = CLEAR;
                injector2OnPeriodCnt = CLEAR;
                injector3OnPeriodCnt = CLEAR;
                injector4OnPeriodCnt = CLEAR;

                injector1OffPeriodCnt = CLEAR;
                injector2OffPeriodCnt = CLEAR;
                injector3OffPeriodCnt = CLEAR;
                injector4OffPeriodCnt = CLEAR;

                injector1CycleCnt = CLEAR;
                injector2CycleCnt = CLEAR;
                injector3CycleCnt = CLEAR;
                injector4CycleCnt = CLEAR;

                // Initialize Injectors values to configured values          
                injector1OnPeriod = fieldValve[field_No].injector1OnPeriod;
                injector2OnPeriod = fieldValve[field_No].injector2OnPeriod;
                injector3OnPeriod = fieldValve[field_No].injector3OnPeriod;
                injector4OnPeriod = fieldValve[field_No].injector4OnPeriod;

                injector1OffPeriod = fieldValve[field_No].injector1OffPeriod;
                injector2OffPeriod = fieldValve[field_No].injector2OffPeriod;
                injector3OffPeriod = fieldValve[field_No].injector3OffPeriod;
                injector4OffPeriod = fieldValve[field_No].injector4OffPeriod;

                injector1Cycle = fieldValve[field_No].injector1Cycle;
                injector2Cycle = fieldValve[field_No].injector2Cycle;
                injector3Cycle = fieldValve[field_No].injector3Cycle;
                injector4Cycle = fieldValve[field_No].injector4Cycle;

                // Initialize injector cycle
                if (injector1OnPeriod > 0) {
                    Fert_Inj1 = ON;
                    injector1OnPeriodCnt++;
                }
                if (injector2OnPeriod > 0) {
                    Fert_Inj2 = ON;
                    injector2OnPeriodCnt++;
                }
                if (injector3OnPeriod > 0) {
                    Fert_Inj3 = ON;
                    injector3OnPeriodCnt++;
                }
                if (injector4OnPeriod > 0) {
                    Fert_Inj4 = ON;
                    injector4OnPeriodCnt++;
                }
                fieldValve[field_No].fertigationStage = injectPeriod;
                if (fieldValve[field_No].fertigationValveInterrupted) {
                    fieldValve[field_No].fertigationValveInterrupted = false;
                    remainingFertigationOnPeriod = readRemainingFertigationOnPeriodFromEeprom();
                    sleepCount = remainingFertigationOnPeriod; // Calculate SleepCounnt after fertigation interrupt due to power off
                } else {
                    sleepCount = fieldValve[field_No].fertigationONperiod; // calculate sleep count for fertigation on period 
                }
                __delay_ms(100);
                saveFertigationValveStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                __delay_ms(100);
                saveActiveSleepCountIntoEeprom(); // Save current valve on time 
                __delay_ms(100);

                /***************************/
                // for field no. 01 to 09
                /*
                if (field_No < 9) {
                    temporaryBytesArray[0] = 48; // To store field no. of valve in action 
                    temporaryBytesArray[1] = field_No + 49; // To store field no. of valve in action 
                }// for field no. 10 to 12
                else if (field_No > 8 && field_No < fieldCount) {
                    temporaryBytesArray[0] = 49; // To store field no. of valve in action 
                    temporaryBytesArray[1] = field_No + 39; // To store field no. of valve in action 
                }
                */ 
                /***************************/

                /***************************/
#ifdef LCD_DISPLAY_ON_H   
                lcdClearLine(2);
                lcdClearLine(3);
                lcdClearLine(4);				
                lcdWriteStringAtCenter("Fertigation Started", 2);
                lcdWriteStringAtCenter("For Field No.", 3);
                lcdSetCursor(3,17);
                clearFieldByte();
                sprintf(fieldByte,"%d",field_No+1);
                temp = strlen((const char *)fieldByte);
                lcdWriteStringIndex(fieldByte,temp);
#endif                
                //sendSms(SmsFert5, userMobileNo, fieldNoRequired); // Acknowledge user about successful Fertigation started action
#ifdef SMS_DELIVERY_REPORT_ON_H
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                //setBCDdigit(0x05, 0);
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
                /***************************/
                /*Send sms*/
                break;
            }// Upon completing fertigation on period sleep, switch off fertigation valve
            else if (fieldValve[field_No].status == ON && fieldValve[field_No].isFertigationEnabled && fieldValve[field_No].fertigationStage == injectPeriod && isLoraSlaveAlive) {
                __delay_ms(1000);
                Fert_Motor = OFF; // switch off fertigation valve for given field after on period                
#ifdef LCD_DISPLAY_ON_H
                removeIcon(fert_icon);
#endif	  
                //Switch off all Injectors after completing fertigation on Period
                Fert_Inj1 = OFF;
                Fert_Inj2 = OFF;
                Fert_Inj3 = OFF;
                Fert_Inj4 = OFF;
                fieldValve[field_No].fertigationStage = flushPeriod;
                fieldValve[field_No].fertigationInstance--;
                if (fieldValve[field_No].fertigationInstance == 0) {
                    fieldValve[field_No].isFertigationEnabled = false;
                }
                __delay_ms(100);
                saveFertigationValveValuesIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                __delay_ms(100);
                sleepCount = fieldValve[field_No].onPeriod - (fieldValve[field_No].fertigationDelay + fieldValve[field_No].fertigationONperiod); // calculate sleep count for on period of Valve 
                __delay_ms(100);
                saveActiveSleepCountIntoEeprom(); // Save current valve on time 
                __delay_ms(100);
                /***************************/
                // for field no. 01 to 09
                /*
                if (field_No < 9) {
                    temporaryBytesArray[0] = 48; // To store field no. of valve in action 
                    temporaryBytesArray[1] = field_No + 49; // To store field no. of valve in action 
                }// for field no. 10 to 12
                else if (field_No > 8 && field_No < fieldCount) {
                    temporaryBytesArray[0] = 49; // To store field no. of valve in action 
                    temporaryBytesArray[1] = field_No + 39; // To store field no. of valve in action 
                }
                */ 
                /***************************/
                if (fertigationDry) { // Fertigation executed with low fertigation level  detection
                    fertigationDry = false;
                    /***************************/
#ifdef LCD_DISPLAY_ON_H   
                    lcdClearLine(2);
                    lcdClearLine(3);
                    lcdClearLine(4);				
                    lcdWriteStringAtCenter("Fertigation Stopped", 2);
                    lcdWriteStringAtCenter("With Low Fert. Level", 3);
                    lcdWriteStringAtCenter("For Field No.", 4);
                    lcdSetCursor(4,17);
                    clearFieldByte();
                    sprintf(fieldByte,"%d",field_No+1);
                    temp = strlen((const char *)fieldByte);
                    lcdWriteStringIndex(fieldByte,temp);
#endif              
                    /***************************/
                    publishNotification("Fertigation Alert",NotFert8_59,true); // Acknowledge user about successful Authentication
                    /***************************/
                    //sendSms(SmsFert8, userMobileNo, fieldNoRequired); // Acknowledge user about Fertigation stopped action due to low fertilizer level
#ifdef SMS_DELIVERY_REPORT_ON_H
                    sleepCount = 2; // Load sleep count for SMS transmission action
                    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                    //setBCDdigit(0x05, 0);
                    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider 
                    //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
                    /***************************/
                    /*Send sms*/
                    break;
                } else if (moistureSensorFailed) { // Fertigation executed with level sensor failure
                    moistureSensorFailed = false;
#ifdef LCD_DISPLAY_ON_H   
                    lcdClearLine(2);
                    lcdClearLine(3);
                    lcdClearLine(4);				
                    lcdWriteStringAtCenter("Fertigation Stopped", 2);
                    lcdWriteStringAtCenter("With Sensor Failure", 3);
                    lcdWriteStringAtCenter("For Field No.", 4);
                    lcdSetCursor(4,17);
                    clearFieldByte();
                    sprintf(fieldByte,"%d",field_No+1);
                    temp = strlen((const char *)fieldByte);
                    lcdWriteStringIndex(fieldByte,temp);
#endif
                    /***************************/
                    publishNotification("Fertigation Alert",NotFert7_70,true); // Acknowledge user about successful Authentication
                    /***************************/
                    
                    /***************************/
                    //sendSms(SmsFert7, userMobileNo, fieldNoRequired); // Acknowledge user about successful Fertigation stopped action
#ifdef SMS_DELIVERY_REPORT_ON_H
                    sleepCount = 2; // Load sleep count for SMS transmission action
                    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                    //setBCDdigit(0x05, 0);
                    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider 
                    //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
                    /***************************/
                    /*Send sms*/
                    break;
                } else { // Fertigation executed without low level detection and without level sensor failure
#ifdef LCD_DISPLAY_ON_H   
                    lcdClearLine(2);
                    lcdClearLine(3);
                    lcdClearLine(4);				
                    lcdWriteStringAtCenter("Fertigation Stopped", 2);
                    lcdWriteStringAtCenter("For Field No.", 3);
                    lcdSetCursor(3,17);
                    clearFieldByte();
                    sprintf(fieldByte,"%d",field_No+1);
                    temp = strlen((const char *)fieldByte);
                    lcdWriteStringIndex(fieldByte,temp);
#endif
                    /***************************/
                    //sendSms(SmsFert6, userMobileNo, fieldNoRequired); // Acknowledge user about successful Fertigation stopped action
#ifdef SMS_DELIVERY_REPORT_ON_H
                    sleepCount = 2; // Load sleep count for SMS transmission action
                    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                    //setBCDdigit(0x05, 0);
                    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider 
                    //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
                    /***************************/
                    /*Send sms*/
                    break;
                }
            }// upon completing entire field valve on period switch off field valve
            else if (fieldValve[field_No].status == ON) {
                fieldValve[field_No].status = OFF;
                if (fieldValve[field_No].cyclesExecuted == fieldValve[field_No].cycles) {
                    fieldValve[field_No].cyclesExecuted = 1; //Cycles execution begin after valve due for first time
                } else {
                    fieldValve[field_No].cyclesExecuted++; //Cycles execution record
                }
                valveDue = false;
                valveExecuted = true; // Valve successfully executed 
                startFieldNo = field_No + 1; // scan for next field no.
                __delay_ms(100);
                saveIrrigationValveNoIntoEeprom(field_No);
                __delay_ms(100);
                saveIrrigationValveOnOffStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                __delay_ms(100);
                saveIrrigationValveCycleStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                __delay_ms(100);
                if (fieldValve[field_No].isFertigationEnabled && isLoraSlaveAlive) {
                    fieldValve[field_No].fertigationStage = OFF;
                    __delay_ms(100);
                    saveFertigationValveStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                    __delay_ms(100);
                }
                break;
            }
        }
    } else if (onHold) {
        onHold = false;
        for (field_No = 0; field_No < fieldCount; field_No++) {
            if (fieldValve[field_No].status == ON) {
                if (!fieldValve[field_No].isConfigured) {
                    fieldValve[field_No].status = OFF;
                    if (fieldValve[field_No].cyclesExecuted == fieldValve[field_No].cycles) {
                        fieldValve[field_No].cyclesExecuted = 1; //Cycles execution begin after valve due for first time
                    } else {
                        fieldValve[field_No].cyclesExecuted++; //Cycles execution record
                    }
                    if (fieldValve[field_No].fertigationStage == injectPeriod) {
                        Fert_Motor = OFF; // switch off fertigation valve for given field after on period
#ifdef LCD_DISPLAY_ON_H
                        removeIcon(fert_icon);
#endif	  
                        //Switch off all Injectors after completing fertigation on Period
                        Fert_Inj1 = OFF;
                        Fert_Inj2 = OFF;
                        Fert_Inj3 = OFF;
                        Fert_Inj4 = OFF;
                        fieldValve[field_No].fertigationStage = OFF;
                        saveFertigationValveStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                        __delay_ms(100);
                    }
                    valveDue = false;
                    valveExecuted = true; // complete valve for hold
                    startFieldNo = field_No + 1; // scan for next field no.
                    __delay_ms(100);
                    saveIrrigationValveNoIntoEeprom(field_No);
                    __delay_ms(100);
                    saveIrrigationValveOnOffStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                    __delay_ms(100);
                    saveIrrigationValveCycleStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                    __delay_ms(100);
                    break;
                } else if (fieldValve[field_No].fertigationStage == wetPeriod) {
                    sleepCount = readActiveSleepCountFromEeprom();
                    sleepCount = (sleepCount + (fieldValve[field_No].onPeriod - fieldValve[field_No].fertigationDelay)); // Calculate Sleep count after fertigation on hold operation  
                    saveActiveSleepCountIntoEeprom();
                    __delay_ms(100);
                    break;
                } else if (fieldValve[field_No].fertigationStage == injectPeriod) {
                    Fert_Motor = OFF; // switch off fertigation valve for given field after on period
#ifdef LCD_DISPLAY_ON_H
                    removeIcon(fert_icon);
#endif	  
                    //Switch off all Injectors after completing fertigation on Period
                    Fert_Inj1 = OFF;
                    Fert_Inj2 = OFF;
                    Fert_Inj3 = OFF;
                    Fert_Inj4 = OFF;
                    fieldValve[field_No].fertigationStage = OFF;
                    saveFertigationValveStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                    __delay_ms(100);
                    sleepCount = readActiveSleepCountFromEeprom();
                    sleepCount = (sleepCount + (fieldValve[field_No].onPeriod - (fieldValve[field_No].fertigationDelay + fieldValve[field_No].fertigationONperiod))); // Calculate Sleep count during fertigation hold operation
                    saveActiveSleepCountIntoEeprom();
                    __delay_ms(100);
                    break;
                }
            }
        }
    }
}
/*************actionsOnSleepCountFinish#End**********/

/*************actionsOnDueValve#Start**********/

/*************************************************************************************************************************

This function is called to do actions on due valve
The purpose of this function is to perform actions after valve is due.

 ***************************************************************************************************************************/
void actionsOnDueValve(unsigned char field_No) {
    unsigned char last_Field_No = CLEAR;
    wetSensor = false;
    // Check if Field is wet
    if (isFieldMoistureSensorWetLora(field_No)) { //Skip current valve execution and go for next
        wetSensor = true;
        valveDue = false;
        fieldValve[field_No].status = OFF;
        fieldValve[field_No].cyclesExecuted = fieldValve[field_No].cycles;
        startFieldNo = field_No + 1; // scan for next field no.
        __delay_ms(50);
        getDueDate(fieldValve[field_No].offPeriod); // calculate next due date of valve
        __delay_ms(50); // Today's date is not known for next due date
        fieldValve[field_No].nextDueDD = (unsigned char) dueDD;
        fieldValve[field_No].nextDueMM = dueMM;
        fieldValve[field_No].nextDueYY = dueYY;
        __delay_ms(100);
        saveIrrigationValveOnOffStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
        __delay_ms(100);
        saveIrrigationValveCycleStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
        __delay_ms(100);
        saveIrrigationValveDueTimeIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
        __delay_ms(100);

        /***************************/
        // for field no. 01 to 09
        /*
        if (field_No < 9) {
            temporaryBytesArray[0] = 48; // To store field no. of valve in action 
            temporaryBytesArray[1] = field_No + 49; // To store field no. of valve in action 
        }// for field no. 10 to 12
        else if (field_No > 8 && field_No < fieldCount) {
            temporaryBytesArray[0] = 49; // To store field no. of valve in action 
            temporaryBytesArray[1] = field_No + 39; // To store field no. of valve in action 
        }
        */ 
        /***************************/

        /***************************/
#ifdef LCD_DISPLAY_ON_H   
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);
        lcdWriteStringAtCenter("Wet Field Detected", 2);
        lcdWriteStringAtCenter("Irri. Not Started", 3);
        lcdWriteStringAtCenter("For Field No.", 4);
        lcdSetCursor(4,17);
        clearFieldByte();
        sprintf(fieldByte,"%d",field_No+1);
        temp = strlen((const char *)fieldByte);
        lcdWriteStringIndex(fieldByte,temp);
#endif
        /***************************/
        publishNotification("Irrigation Alert",NotIrr6_54,true); // Acknowledge user about successful Authentication
        /***************************/
        //sendSms(SmsIrr6, userMobileNo, fieldNoRequired); // Acknowledge user about Irrigation not started due to wet field detection						
#ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        //setBCDdigit(0x05, 0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
        /***************************/
    }        
    else {
        __delay_ms(100);
        activateValve(field_No); // Activate valve for field
        if (!LoraConnectionFailed) { // Skip next block if activation is failed
            __delay_ms(100);

            //Switch ON Fertigation valve interrupted due to power on same day
            if (fieldValve[field_No].fertigationStage == injectPeriod) {
                powerOnMotor(); // Power ON Motor							 
                __delay_ms(1000);
                Fert_Motor = ON;
#ifdef LCD_DISPLAY_ON_H
                displayIcon(fert_icon);
#endif					   
                // Injector code
                // Initialize all count to zero
                injector1OnPeriodCnt = CLEAR;
                injector2OnPeriodCnt = CLEAR;
                injector3OnPeriodCnt = CLEAR;
                injector4OnPeriodCnt = CLEAR;

                injector1OffPeriodCnt = CLEAR;
                injector2OffPeriodCnt = CLEAR;
                injector3OffPeriodCnt = CLEAR;
                injector4OffPeriodCnt = CLEAR;

                injector1CycleCnt = CLEAR;
                injector2CycleCnt = CLEAR;
                injector3CycleCnt = CLEAR;
                injector4CycleCnt = CLEAR;

                // Initialize Injectors values to configured values
                injector1OnPeriod = fieldValve[field_No].injector1OnPeriod;
                injector2OnPeriod = fieldValve[field_No].injector2OnPeriod;
                injector3OnPeriod = fieldValve[field_No].injector3OnPeriod;
                injector4OnPeriod = fieldValve[field_No].injector4OnPeriod;

                injector1OffPeriod = fieldValve[field_No].injector1OffPeriod;
                injector2OffPeriod = fieldValve[field_No].injector2OffPeriod;
                injector3OffPeriod = fieldValve[field_No].injector3OffPeriod;
                injector4OffPeriod = fieldValve[field_No].injector4OffPeriod;

                injector1Cycle = fieldValve[field_No].injector1Cycle;
                injector2Cycle = fieldValve[field_No].injector2Cycle;
                injector3Cycle = fieldValve[field_No].injector3Cycle;
                injector4Cycle = fieldValve[field_No].injector4Cycle;

                // Initialize injector cycle
                if (injector1OnPeriod > 0) {
                    Fert_Inj1 = ON;
                    injector1OnPeriodCnt++;
                }
                if (injector2OnPeriod > 0) {
                    Fert_Inj2 = ON;
                    injector2OnPeriodCnt++;
                }
                if (injector3OnPeriod > 0) {
                    Fert_Inj3 = ON;
                    injector3OnPeriodCnt++;
                }
                if (injector4OnPeriod > 0) {
                    Fert_Inj4 = ON;
                    injector4OnPeriodCnt++;
                }
                /***************************/
                // for field no. 01 to 09
                /*
                if (field_No < 9) {
                    temporaryBytesArray[0] = 48; // To store field no. of valve in action 
                    temporaryBytesArray[1] = field_No + 49; // To store field no. of valve in action 
                }// for field no. 10 to 12
                else if (field_No > 8 && field_No < fieldCount) {
                    temporaryBytesArray[0] = 49; // To store field no. of valve in action 
                    temporaryBytesArray[1] = field_No + 39; // To store field no. of valve in action 
                }
                */ 
                /***************************/

                /***************************/
#ifdef LCD_DISPLAY_ON_H   
                lcdClearLine(2);
                lcdClearLine(3);
                lcdClearLine(4);								
                lcdWriteStringAtCenter("Fertigation Started", 2);
                lcdWriteStringAtCenter("For Field No.", 3);
                lcdSetCursor(3,17);
                clearFieldByte();
                sprintf(fieldByte,"%d",field_No+1);
                temp = strlen((const char *)fieldByte);
                lcdWriteStringIndex(fieldByte,temp);
#endif
                /***************************/
                publishNotification("Fertigation Alert",NotFert5_33,true); // Acknowledge user about successful Authentication
                /***************************/
                //sendSms(SmsFert5, userMobileNo, fieldNoRequired); // Acknowledge user about successful Fertigation started action
#ifdef SMS_DELIVERY_REPORT_ON_H
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                //setBCDdigit(0x05, 0);
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider           
                //setBCDdigit(0x0F, 0); // Blank "." BCD Indication for Normal Condition
#endif
                /***************************/
                /*Send sms*/

            } else if (valveExecuted) { // DeActivate previous executed field valve
                last_Field_No = readFieldIrrigationValveNoFromEeprom();
                if (last_Field_No != field_No) { // if not multiple cycles for same valve
                    deActivateValve(last_Field_No); // Successful Deactivate valve 
                }
                valveExecuted = false;
            } else { // Switch on Motor for First Valve activation
                powerOnMotor(); // Power ON Motor
            }

            if (fieldValve[field_No].cyclesExecuted == fieldValve[field_No].cycles) {
                /******** Calculate and save Field Valve next Due date**********/
                getDueDate(fieldValve[field_No].offPeriod); // calculate next due date of valve
                fieldValve[field_No].nextDueDD = (unsigned char) dueDD;
                fieldValve[field_No].nextDueMM = dueMM;
                fieldValve[field_No].nextDueYY = dueYY;
                __delay_ms(100);
                saveIrrigationValveDueTimeIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                __delay_ms(100);
                /***********************************************/
            }
        }
    }
}
/*************actionsOnDueValve#End**********/


/*************deleteUserDataOnRequest#Start**********/

/*************************************************************************************************************************

This function is called to delete user data
The purpose of this function is to delete user data and informed user about deletion

 ***************************************************************************************************************************/
void deleteUserData(void) {
#ifdef LCD_DISPLAY_ON_H   
    lcdClearLine(2);
    lcdClearLine(3);
    lcdClearLine(4);
    lcdWriteStringAtCenter("System Reset Occurred", 2);
    lcdWriteStringAtCenter("Factory Code Reset", 3);
#endif
    /***************************/
    //sendSms(SmsSR14, userMobileNo, noInfo);
#ifdef SMS_DELIVERY_REPORT_ON_H
    sleepCount = 2; // Load sleep count for SMS transmission action
    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
    //setBCDdigit(0x05,0);
    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
    //setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
#endif
    /***************************/
    systemAuthenticated = false;
    saveAuthenticationStatus();
    for (iterator = 0; iterator < 10; iterator++) {
        userMobileNo[iterator] = '0';
    }
    saveDeviceIDIntoEeprom();
}
/*************deleteUserDataOnRequest#End**********/


/*************deleteValveDataOnRequest#Start**********/

/*************************************************************************************************************************

This function is called to delete valve configuration data
The purpose of this function is to delete user configured valve data and informed user about deletion

 ***************************************************************************************************************************/
void deleteValveData(void) {
#ifdef LCD_DISPLAY_ON_H   
    lcdClearLine(2);
    lcdClearLine(3);
    lcdClearLine(4);
    lcdWriteStringAtCenter("System Reset Occurred", 2);
    lcdWriteStringAtCenter("Irri. Data Reset", 3);
#endif
    /***************************/
    //sendSms(SmsSR15, userMobileNo, noInfo);
#ifdef SMS_DELIVERY_REPORT_ON_H
    sleepCount = 2; // Load sleep count for SMS transmission action
    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
    //setBCDdigit(0x05,0);
    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
    //setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
#endif
    /***************************/
    filtrationDelay1 = 0;
    filtrationDelay2 = 0;
    filtrationDelay3 = 0;
    filtrationOnTime = 0;
    filtrationSeperationTime = 0;
    filtrationEnabled = false;
    __delay_ms(100);
    saveFiltrationSequenceData();
    __delay_ms(100);
    for (iterator = 0; iterator < fieldCount; iterator++) {
        msgIndex = CLEAR;
        fieldValve[iterator].status = OFF;
        fieldValve[iterator].isConfigured = false;
        fieldValve[iterator].fertigationDelay = 0;
        fieldValve[iterator].fertigationONperiod = 0;
        fieldValve[iterator].fertigationInstance = 0;
        fieldValve[iterator].fertigationStage = OFF;
        fieldValve[iterator].fertigationValveInterrupted = false;
        fieldValve[iterator].isFertigationEnabled = false;

        saveIrrigationValveOnOffStatusIntoEeprom(eepromAddress[iterator], &fieldValve[iterator]);
        __delay_ms(100);
        saveIrrigationValveConfigurationStatusIntoEeprom(eepromAddress[iterator], &fieldValve[iterator]);
        __delay_ms(100);
        saveFertigationValveValuesIntoEeprom(eepromAddress[iterator], &fieldValve[iterator]);
        __delay_ms(100);
    }					  
}
/*************deleteValveDataOnRequest#End**********/

/*************randomPasswordGeneration#Start**********/

/*************************************************************************************************************************

This function is called to generate 6 digit password
The purpose of this function is to randomly generate password of length 6

 ***************************************************************************************************************************/
void randomPasswordGeneration(void) {					  
    // Seed the random-number generator
    // with current time so that the
    // numbers will be different every time
    getDateFromGSM();
    srand((unsigned int) (currentDD + currentHour + currentMinutes + currentSeconds));

    // Array of numbers
    unsigned char numbers[] = "0123456789";

    // Iterate over the range [0, N]
    for (iterator = 0; iterator < 6; iterator++) {
        factryPswrd[iterator] = numbers[rand() % 10];
    }
    factryPswrd[6] = '\0';					  
}

/*************clear gsmResponse string#Start**********/

/*************************************************************************************************************************

This function is called to clear gsm response string
The purpose of this function is to enter null values in gsm response

 ***************************************************************************************************************************/
void clearGsmResponse(void) {
    /***************************/
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("clearGsmResponse_IN\r\n");
    //********Debug log#end**************//
#endif
    // Iterate over the range [0, N]
    for (iterator = 0; iterator < 200; iterator++) {
        gsmResponse[iterator] = '\0';
    }
    msgIndex = CLEAR;
    /***************************/
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("clearGsmResponse_OUT\r\n");
    //********Debug log#end**************//
#endif
}

/*************clear StringToDecode string#Start**********/

/*************************************************************************************************************************

This function is called to clear stringToDecode  string
The purpose of this function is to enter null values in stringToDecode

 ***************************************************************************************************************************/
void clearStringToDecode(void) {
    /***************************/
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("clearStringToDecode_IN\r\n");
    //********Debug log#end**************//
#endif
    // Iterate over the range [0, N]
    for (iterator = 0; iterator < 200; iterator++) {
        //stringToDecode[iterator] = '\0';
        decodedString[iterator] = '\0';
    }
    /***************************/
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("clearStringToDecode_OUT\r\n");
    //********Debug log#end**************//
#endif
}

/*************clear Decoded string#Start**********/

/*************************************************************************************************************************

This function is called to clear Decoded string
The purpose of this function is to enter null values in Decoded string

 ***************************************************************************************************************************/
void clearDecodedString(void) {
    /***************************/
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("clearDecodedString_IN\r\n");
    //********Debug log#end**************//
#endif
    // Iterate over the range [0, N]
    for (iterator = 0; iterator < 200; iterator++) {
        decodedString[iterator] = '\0';
    }
    /***************************/
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("clearDecodedString_OUT\r\n");
    //********Debug log#end**************//
#endif
}

/*************clear tempBytes string#Start**********/

/*************************************************************************************************************************

This function is called to clear Temporary Bytes string
The purpose of this function is to enter null values in Decoded string

 ***************************************************************************************************************************/
void clearTempBytesString(void) {
    /***************************/
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("clearTempBytesString_IN\r\n");
    //********Debug log#end**************//
#endif
    // Iterate over the range [0, N]
    for (iterator = 0; iterator < 26; iterator++) {
        temporaryBytesArray[iterator] = '\0';
    }
    /***************************/
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("clearTempBytesString_OUT\r\n");
    //********Debug log#end**************//
#endif
}

/*************clear tempBytes string#Start**********/

/*************************************************************************************************************************

This function is called to clear Temporary Bytes string
The purpose of this function is to enter null values in Decoded string

 ***************************************************************************************************************************/
void clearFieldByte(void) {
    /***************************/
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("clearFieldByte_IN\r\n");
    //********Debug log#end**************//
#endif
    // Iterate over the range [0, N]
    for (iterator = 0; iterator < 3; iterator++) {
        fieldByte[iterator] = '\0';
    }
    /***************************/
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("clearFieldByte_OUT\r\n");
    //********Debug log#end**************//
#endif
}

/*************Count Pulses #Start**********/

/*************************************************************************************************************************

This function is called to count pulses
The purpose of this function is to count pulses for 10 sec

 ***************************************************************************************************************************/
void countPulses(unsigned char channel) {
    /***************************/
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("countPulses_IN\r\n");
    //********Debug log#end**************//
#endif
#ifdef LCD_DISPLAY_ON_H
    lcdClearLine(2);
    lcdClearLine(3);
    lcdClearLine(4);
    lcdWriteStringAtCenter("Starting Pulse count", 2);
    __delay_ms(3000);
#endif
    switch(channel) {
    case 0 :
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);
        lcdWriteStringAtCenter("Water Flow", 2);
#endif
        checkWaterFlow = true;
        checkFertFlow = false;
        IOCEF = 0b00000000; // Clear all initial IOC flags
        IOCEP = 0b00000100; //Interrupt-on-Change Positive Edge Enable bits for RE2 --Irrigation Flow Sensor
        break;
    case 1 : 
#ifdef LCD_DISPLAY_ON_H
        lcdClearLine(2);
        lcdClearLine(3);
        lcdClearLine(4);
        lcdWriteStringAtCenter("Fertigation Flow", 2);
#endif
        checkWaterFlow = false;
        checkFertFlow = true;
        IOCEF = 0b00000000; // Clear all initial IOC flags
        IOCEP = 0b00001000; //Interrupt-on-Change Positive Edge Enable bits for RE3 --Fertigation Flow Sensor
        break;
    }
    PIE0bits.IOCIE = ENABLED; //Peripheral Interrupt-on-Change Enabled
    timer3Count = 3; // 30 sec window
    pulses = 0;
    controllerCommandExecuted = false;
    isPulseOn = true;
    T3CONbits.TMR3ON = ON;
    while (isPulseOn);
    controllerCommandExecuted = true;
    PIE0bits.IOCIE = DISABLED; //Peripheral Interrupt-on-Change Disabled
    IOCEF = 0b00000000; // Clear all initial IOC flags
    IOCEP = 0b00000000; //Interrupt-on-Change Positive Edge Disabled bits for RE2 and RE3 --Irrigation and Fertigation Flow Sensor
    checkWaterFlow = false;
    checkFertFlow = false;
#ifdef LCD_DISPLAY_ON_H   
    lcdSetCursor(3,10);
    sprintf((char *)temporaryBytesArray,"%d",pulses);
    lcdWriteStringIndex(temporaryBytesArray,3);
    __delay_ms(3000);
#endif
    
    /***************************/
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("countPulses_OUT\r\n");
    //********Debug log#end**************//
#endif
}