/*
 * File name            : Main.c
 * Compiler             : MPLAB XC8/ MPLAB C18 compiler
 * IDE                  : Microchip  MPLAB X IDE v5.25
 * Processor            : PIC18F66K40
 * Author               : Bhoomi Jalasandharan
 * Created on           : July 15, 2020, 05:23 PM
 * Description          : Main source file
 */

/***************************** Header file declarations#start ************************/

#include "congfigBits.h"
#include "variableDefinitions.h"
#include "controllerActions.h"
#include "eeprom.h"
#include "gprs.h"
#include "lora.h"
#include "i2c.h"
#include "i2c_LCD_PCF8574.h"
#ifdef DEBUG_MODE_ON_H
#include "serialMonitor.h"
#endif
/***************************** Header file declarations#end **************************/

//**************interrupt service routine handler#start***********//

/*************************************************************************************************************************

This function is called when an interrupt has occurred at RX pin of ?c which is connected to TX pin of GSM.
Interrupt occurs at 1st cycle of each Data byte.
The purpose of this interrupt handler is to store the data received from GSM into Array called gsmResponse[]
Each response from GSM starts with '+' symbol, e.g. +CMTI: "SM", <index>
The End of SMS is detected by OK command.

 **************************************************************************************************************************/

void __interrupt(high_priority)rxANDiocInterrupt_handler(void) {
    if (PIR4bits.RC3IF) { // Interrupt on GSM RX pin
        Run_led = GLOW; // Led Indication for system in Operational Mode
        rxCharacter = rxByte(); // Read byte received at Reception Register
        // Check if any overrun occur due to continuous reception
        if (RC3STAbits.OERR) {
            RC3STAbits.CREN = 0;
            Nop();
            RC3STAbits.CREN = 1;
        }
        if (!controllerCommandExecuted) { // check if GSM response to µc command is not completed
            if (rxCharacter == '+' && msgIndex == 0) {
                gsmResponse[msgIndex] = rxCharacter; // Load received byte into storage buffer
                msgIndex++; // point to next location for storing next received byte
            } else if (msgIndex > 0 && msgIndex <=200) { // Cascade received data to stored response after receiving first character '+'
                gsmResponse[msgIndex] = rxCharacter; // Load received byte into storage buffer
                if (gsmResponse[msgIndex - 1] == 'O' && gsmResponse[msgIndex] == 'K') { // Cascade till 'OK'  is found
                    isOK = true;
                    isERROR = false;
                    controllerCommandExecuted = true; // GSM response to µc command is completed
                    msgIndex = CLEAR; // Reset message storage index to first character to start reading for next received byte of cmd
                } else if (gsmResponse[msgIndex - 4] == 'E' && gsmResponse[msgIndex - 3] == 'R' && gsmResponse[msgIndex - 2] == 'R' && gsmResponse[msgIndex - 1] == 'O' && gsmResponse[msgIndex] == 'R') { // Cascade till 'OK'  is found
                    isERROR = true;
                    controllerCommandExecuted = true; // GSM response to µc command is completed
                    msgIndex = CLEAR; // Reset message storage index to first character to start reading for next received byte of cmd
                } else if (msgIndex <= 200) { // Read bytes till 200 characters
                    msgIndex++;
                }
            }    
        } else if (rxCharacter == '*') {
            msgIndex = CLEAR; // Reset message storage index to first character to start reading from '+'
            gsmResponse[msgIndex] = rxCharacter; // Load Received byte into storage buffer
            msgIndex++; // point to next location for storing next received byte
            msgStart = true;
            atcmdStart = false;
        } else if ( msgStart && (msgIndex > 0 && msgIndex < 200)) {
            gsmResponse[msgIndex] = rxCharacter; // Load received byte into storage buffer
            msgIndex++; // point to next location for storing next received byte
            // check if storage index is reached to last character of CMTI command
            if (rxCharacter == '#') {
                msgIndex = CLEAR;
                newSMSRcvd = true; // Set to indicate New topic is Received
                msgStart = false;
            }
        } else if (rxCharacter == '+' && !newSMSRcvd) {
            msgIndex = CLEAR; // Reset message storage index to first character to start reading from '+'
            gsmResponse[msgIndex] = rxCharacter; // Load Received byte into storage buffer
            msgIndex++;
            atcmdStart = true;
        } else if ( atcmdStart && (msgIndex > 0 && msgIndex < 14)) {
            gsmResponse[msgIndex] = rxCharacter; // Load received byte into storage buffer
            msgIndex++; // point to next location for storing next received byte
            // check if storage index is reached to last character of CMTI command
            if (msgIndex == 14) {
                if (strncmp(gsmResponse+1, cmqttError, 13) == 0) {
                    clearGsmResponse();
                    isERROR = true;
                    isErrorActionTaken = false;
                }
                atcmdStart = false;   
            }
        } else {
            atcmdStart = false;
        }
        //SIM_led = DARK;  // Led Indication for GSM interrupt is done 
        PIR4bits.RC3IF= CLEAR; // Reset the ISR flag.
    } else if (PIR3bits.RC1IF) { // Interrupt on LORA RX pin
        Run_led = GLOW; // Led Indication for system in Operational Mode
        rxCharacter = rxByteLora(); // Read byte received at Reception Register
        // Check if any overrun occur due to continuous reception
        if (RC1STAbits.OERR) {
            RC1STAbits.CREN = 0;
            Nop();
            RC1STAbits.CREN = 1;
        }
        if (rxCharacter == '#') {
            msgIndex = CLEAR; // Reset message storage index to first character to start reading from '+'
            decodedString[msgIndex] = rxCharacter; // Load Received byte into storage buffer
            msgIndex++; // point to next location for storing next received byte
        } else if (msgIndex > 0 && msgIndex < 25) {
            decodedString[msgIndex] = rxCharacter; // Load received byte into storage buffer
            msgIndex++; // point to next location for storing next received byte
            // check if storage index is reached to last character of CMTI command
            if (rxCharacter == '$') {
                msgIndex = CLEAR;
                controllerCommandExecuted = true; // Set to indicate command received from lora
                if (strncmp(decodedString+1, lowbattery, 15) == 0) {
                    iterator_field = fetchDigits(16);
                    clearDecodedString();
                    clearFieldByte();
                    sprintf(fieldByte,"%d",iterator_field+1);
                    lowBattery = true;
                } else if (strncmp(decodedString+1, resetslave, 10) == 0) {
                    iterator_field = fetchDigits(11);
                    clearDecodedString();
                    clearFieldByte();
                    sprintf(fieldByte,"%d",iterator_field+1);
                    resetSlave = true;
                }
            }
        }
        //SIM_led = DARK;  // Led Indication for LORA interrupt is done 
        PIR3bits.RC1IF= CLEAR; // Reset the ISR flag.
    } else if (PIR0bits.IOCIF) { //Interrupt-on-change pins
        Run_led = GLOW; // Led Indication for system in Operational Mode
        if (IOCEF2 == 1 && checkWaterFlow) {// rising edge of Water flow sensor
            if (isPulseOn) {
               pulses++;  
            }
            //IOCEF &= (IOCEF ^ 0xFF); //Clearing RE2 Interrupt Flags
        } else if (IOCEF3 == 1 && checkFertFlow) {// rising edge of Fertigation flow sensor
            if (isPulseOn) {
               pulses++;  
            }
            //IOCEF &= (IOCEF ^ 0xFF); //Clearing RE3 Interrupt Flags  
        }
        IOCEF &= (IOCEF ^ 0xFF); //Clearing IOCE Interrupt Flags 
        // Rising Edge -- All phase present
        if (IOCBF0 == 1) {
            IOCBF &= (IOCBF ^ 0xFF); //Clearing Interrupt Flags
            if (Phase_Input == 0) { // Phase is present
                __delay_ms(2500);__delay_ms(2500);
                if (Phase_Input == 0) { // to check false trigger
                    //phase is on                
                    phaseFailureDetected = false;
#ifdef LCD_DISPLAY_ON_H
                    removeIcon(phase_icon);
#endif	  
                    __delay_ms(2500);__delay_ms(2500);
                    RESET();
                }
            } else if (Phase_Input == 1) { // Phase is Out
                __delay_ms(2500);__delay_ms(2500);
                if (Phase_Input == 1) { // to check false trigger
#ifdef LCD_DISPLAY_ON_H
                    displayIcon(phase_icon);
#endif	  
                    // phase is out
                    phaseFailureDetected = false; //true
                    phaseFailureActionTaken = false;
                }
            }
        }
        PIR0bits.IOCIF = CLEAR; // Reset the ISR flag.
    }          
}


/*************************************************************************************************************************

This function is called when an interrupt is occurred after 16 bit timer is overflow
The purpose of this interrupt handler is to count no. of overflows that the timer did.

 **************************************************************************************************************************/

void __interrupt(low_priority) timerInterrupt_handler(void) {
    /*To follow filtration  cycle sequence*/
    if (PIR0bits.TMR0IF) {
        Run_led = GLOW; // Led Indication for system in Operational Mode
        PIR0bits.TMR0IF = CLEAR;
        TMR0H = 0xE3; // Load Timer0 Register Higher Byte 
        TMR0L = 0xB0; // Load Timer0 Register Lower Byte
        Timer0Overflow++;
        // Control sleep count decrement for each one minute interrupt when Motor is ON i.e. Valve ON period 
        if (sleepCount > 0 && Irri_Motor == ON) {  // check additional condition of ValveDue
            sleepCount--;
            loraAliveCountCheck++; // increment for each sleep
            if (dryRunCheckCount == 0 || dryRunCheckCount < 3) {
                dryRunCheckCount++;
            }
            if (strncmp(decodedString+1, alive, 5) == 0 && strncmp(decodedString+6, slave, 5) == 0) {
                clearDecodedString();
                loraAliveCount++;  //increment for each alive message               
            }
            if (loraAliveCountCheck <= loraAliveCount+2) { // check if alive count for each sleep is incremented
                #ifdef DEBUG_MODE_ON_H
                //********Debug log#start************//
                transmitStringToDebug("Lora is alive\r\n");
                //********Debug log#end**************//
                #endif
            }
            else {
                #ifdef DEBUG_MODE_ON_H
                //********Debug log#start************//
                transmitStringToDebug("Lora is not alive\r\n");
                //********Debug log#end**************//
                #endif
                isLoraSlaveAlive = false;
                sleepCount = 0;
            }
        } 
		// Check Fertigation Level for each one minute interrupt when Fertigation Motor is ON during Valve ON period 
        //if (Fert_Motor == ON) {
        //    fertigationDry = false;
        //    if (!moistureSensorFailed) {  // to avoid repeated fertigation level check after sensor failure detected
        //        if (isFieldMoistureSensorWet(11)==false) {
        //            if (!moistureSensorFailed) { // to avoid sensor dry detection due to sensor failure
        //                Fert_Motor = OFF;
        //                fertigationDry = true;
        //            }
        //        }
        //    }
        //}
        //To follow fertigation cycle sequence
        if (Fert_Motor == ON) {
            if (Fert_Inj1 == ON) {
                if(injector1OnPeriodCnt == injector1OnPeriod) {
                    Fert_Inj1 = OFF;
                    injector1OnPeriodCnt = CLEAR;
                    injector1OffPeriodCnt++;
                    injector1CycleCnt++;
                }
                else injector1OnPeriodCnt++;
            }
            else if (Fert_Inj1 == OFF) {
                if(injector1OffPeriodCnt == injector1OffPeriod) {
                    if (injector1CycleCnt < injector1Cycle) {
                        Fert_Inj1 = ON;
                        injector1OnPeriodCnt++;
                        injector1OffPeriodCnt = CLEAR;
                    }
                    else injector1OffPeriodCnt = injector1OffPeriod + 1;
                }
                else injector1OffPeriodCnt++;
            }
            if (Fert_Inj2 == ON) {
                if(injector2OnPeriodCnt == injector2OnPeriod) {
                    Fert_Inj2 = OFF;
                    injector2OnPeriodCnt = CLEAR;
                    injector2OffPeriodCnt++;
                    injector2CycleCnt++;
                }
                else injector2OnPeriodCnt++;
            }
            else if (Fert_Inj2 == OFF) {
                if(injector2OffPeriodCnt == injector2OffPeriod) {
                    if (injector2CycleCnt < injector2Cycle) {
                        Fert_Inj2 = ON;
                        injector2OnPeriodCnt++;
                        injector2OffPeriodCnt = CLEAR;
                    }
                    else injector2OffPeriodCnt = injector2OffPeriod + 1;
                }
                else injector2OffPeriodCnt++;
            }
            if (Fert_Inj3 == ON) {
                if(injector3OnPeriodCnt == injector3OnPeriod) {
                    Fert_Inj3 = OFF;
                    injector3OnPeriodCnt = CLEAR;
                    injector3OffPeriodCnt++;
                    injector3CycleCnt++;
                }
                else injector3OnPeriodCnt++;
            }
            else if (Fert_Inj3 == OFF) {
                if(injector3OffPeriodCnt == injector3OffPeriod) {
                    if (injector3CycleCnt < injector3Cycle) {
                        Fert_Inj3 = ON;
                        injector3OnPeriodCnt++;
                        injector3OffPeriodCnt = CLEAR;
                    }
                    else injector3OffPeriodCnt = injector3OffPeriod + 1;
                }
                else injector3OffPeriodCnt++;
            }
            if (Fert_Inj4 == ON) {
                if(injector4OnPeriodCnt == injector4OnPeriod) {
                    Fert_Inj4 = OFF;
                    injector4OnPeriodCnt = CLEAR;
                    injector4OffPeriodCnt++;
                    injector4CycleCnt++;
                }
                else injector4OnPeriodCnt++;
            }
            else if (Fert_Inj4 == OFF) {
                if(injector4OffPeriodCnt == injector4OffPeriod) {
                    if (injector4CycleCnt < injector4Cycle) {
                        Fert_Inj4 = ON;
                        injector4OnPeriodCnt++;
                        injector4OffPeriodCnt = CLEAR;
                    }
                    else injector4OffPeriodCnt = injector4OffPeriod + 1;
                }
                else injector4OffPeriodCnt++;
            }
        }
        //*To follow filtration  cycle sequence*/
        if (filtrationCycleSequence == 99) {    // Filtration is disabled
            Timer0Overflow = 0;
        }
        else if (filtrationCycleSequence == 1 && Timer0Overflow == filtrationDelay1 ) { // Filtration1 Start Delay
                Timer0Overflow = 0;
                Filt_Out1 = ON;
                filtrationCycleSequence = 2;
        }
        else if (filtrationCycleSequence == 2 && Timer0Overflow == filtrationOnTime ) {  // Filtration1 On Period
            Timer0Overflow = 0;
            Filt_Out1 = OFF;
            filtrationCycleSequence = 3;
        }
        else if (filtrationCycleSequence == 3 && Timer0Overflow == filtrationDelay2 ) { // Filtration2 Start Delay
            Timer0Overflow = 0;
            Filt_Out2 = ON;
            filtrationCycleSequence = 4;
        }
        else if (filtrationCycleSequence == 4 && Timer0Overflow == filtrationOnTime ) { // Filtration2 On Period
            Timer0Overflow = 0;
            Filt_Out2 = OFF;
            filtrationCycleSequence = 5;
        }
        else if (filtrationCycleSequence == 5 && Timer0Overflow == filtrationDelay2 ) { // Filtration3 Start Delay
            Timer0Overflow = 0;
            Filt_Out3 = ON;
            filtrationCycleSequence = 6;
        }
        else if (filtrationCycleSequence == 6 && Timer0Overflow == filtrationOnTime ) { // Filtration3 On Period
            Timer0Overflow = 0;
            Filt_Out3 = OFF;
            filtrationCycleSequence = 7;
        }
        else if (filtrationCycleSequence == 7 && Timer0Overflow == filtrationSeperationTime ) { //Filtration Repeat Delay
            Timer0Overflow = 0;
            filtrationCycleSequence = 1;
        }
    }
/*To measure pulse width of moisture sensor output*/
    if (PIR5bits.TMR1IF) {
        Run_led = GLOW; // Led Indication for system in Operational Mode
        Timer1Overflow++;
        PIR5bits.TMR1IF = CLEAR;
    }
    if (PIR5bits.TMR3IF) {
        Run_led = GLOW; // Led Indication for system in Operational Mode
        PIR5bits.TMR3IF = CLEAR;
        TMR3H = 0xF0; // Load Timer3 Register Higher Byte 
        TMR3L = 0xDC; // Load Timer3 Register lower Byte 
        Timer3Overflow++;
        
        if (Timer3Overflow > timer3Count  && !controllerCommandExecuted) {
            controllerCommandExecuted = true; // Unlock key
            Timer3Overflow = 0;
            T3CONbits.TMR3ON = OFF; // Stop timer
            if (checkLoraConnection) {
                LoraConnectionFailed = true;
            }
			else if (checkMoistureSensor) {
                moistureSensorFailed = true;
            }
            else if (isPulseOn) {
                isPulseOn = false;
            }
        } else if (controllerCommandExecuted) {
            Timer3Overflow = 0;
            T3CONbits.TMR3ON= OFF; // Stop timer
        } /*else if (Timer3Overflow > timer3Count && !lcdCmdExecuted) {
            lcdCmdExecuted = true; // Unlock key
            Timer3Overflow = 0;
            T3CONbits.TMR3ON = OFF; // Stop timer    
        }*/
    }
}
//**************interrupt service routine handler#end***********//



//****************************MAIN FUNCTION#Start***************************************//
 void main(void) {
    NOP();
    NOP();
    NOP();
    unsigned char last_Field_No = CLEAR;
    actionsOnSystemReset();
    while (1) {
nxtVlv: if (!valveDue && !phaseFailureDetected && !lowPhaseCurrentDetected) {
            LoraConnectionFailed = false;  // reset slave lora connection failed status for first  field
            wetSensor = false; // reset wet sensor for first wet field detection
            __delay_ms(50);
            scanValveScheduleAndGetSleepCount(); // get sleep count for next valve action
            __delay_ms(50);
            dueValveChecked = true;
        }
        if (valveDue && dueValveChecked) {
            #ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("actionsOnDueValve_IN\r\n");
            //********Debug log#end**************//
            #endif
            dueValveChecked = false;
            actionsOnDueValve(iterator_field);// Copy field no. navigated through iterator. 
            #ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("actionsOnDueValve_OUT\r\n");
            //********Debug log#end**************//
            #endif
        }
        // DeActivate last valve and switch off motor pump
        else if (valveExecuted) {
            LoraConnectionFailed = false;  // reset slave lora connection failed status for last  field
            wetSensor = false; // reset wet sensor for last wet field detection
            powerOffMotor();
            last_Field_No = readFieldIrrigationValveNoFromEeprom();
            deActivateValve(last_Field_No);      // Successful Deactivate valve
            valveExecuted = false;
#ifdef LCD_DISPLAY_ON_H   
            lcdClearLine(2);
            lcdClearLine(3);
            lcdClearLine(4);				
            lcdWriteStringAtCenter("Irrigation Completed", 2);
            lcdWriteStringAtCenter("Motor Switched OFF", 3);
#endif
            /***************************/
            publishNotification("Irrigation Alert",NotMotor1_55,false); // Acknowledge user about successful Authentication
            /***************************/
                        
            /***************************/
            //sendSms(SmsMotor1, userMobileNo, noInfo); // Acknowledge user about successful action
#ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            //setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            //setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
#endif
            /***************************/
            startFieldNo = 0;
            //goto nxtVlv;
        }
        // system on hold
        if (onHold) {
            sleepCount = 0; // Skip Next sleep for performing hold operation
        }
        if(!LoraConnectionFailed && !wetSensor) {   //|| // Skip next block if Activate valve cmd is failed
            controllerCommandExecuted = true;
            msgIndex = CLEAR;
            /****************************/
            deepSleep(); // sleep for given sleep count (	default/calculated )
            /****************************/
            // check if Sleep count executed with interrupt occurred due to new SMS command reception
            #ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug((const char *)gsmResponse);
            transmitStringToDebug("\r\n");
            //********Debug log#end**************//
            #endif
            if (newSMSRcvd) {
#ifdef LCD_DISPLAY_ON_H
                displayIcon(sms_icon);
                __delay_ms(500);
#endif
                #ifdef DEBUG_MODE_ON_H
                //********Debug log#start************//
                transmitStringToDebug("newSMSRcvd_IN\r\n");
                //********Debug log#end**************//
                #endif
               
                newSMSRcvd = false; // received command is processed										
                extractReceivedSms(); // Read received SMS
 
                //deleteMsgFromSIMStorage();
                #ifdef DEBUG_MODE_ON_H
                //********Debug log#start************//
                transmitStringToDebug("newSMSRcvd_OUT\r\n");
                //********Debug log#end**************//
                #endif
            } 
            //check if Sleep count executed without external interrupt
            else {
                #ifdef DEBUG_MODE_ON_H
                //********Debug log#start************//
                transmitStringToDebug("actionsOnSleepCountFinish_IN\r\n");
                //********Debug log#end**************//
                #endif
                actionsOnSleepCountFinish();
                #ifdef DEBUG_MODE_ON_H
                //********Debug log#start************//
                transmitStringToDebug("actionsOnSleepCountFinish_OUT\r\n");
                //********Debug log#end**************//
                #endif
                if ( !rtcBatteryLevelChecked) {
                    if (isRTCBatteryDrained()){
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
                        //sendSms(SmsRTC1, userMobileNo, noInfo); // Acknowledge user about replace RTC battery
#ifdef SMS_DELIVERY_REPORT_ON_H
                        sleepCount = 2; // Load sleep count for SMS transmission action
                        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                        //setBCDdigit(0x05,0);
                        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                        //setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
#endif
                        /***************************/
                    }
                }
            }
        }
    }
}
//****************************MAIN FUNCTION#End***************************************//