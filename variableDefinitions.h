/*
 * File name            : variableDefinitions.h
 * Compiler             : MPLAB XC8/ MPLAB C18 compiler
 * IDE                  : Microchip  MPLAB X IDE v5.25
 * Processor            : PIC18F66K40
 * Author               : Bhoomi Jalasandharan
 * Created on           : July 15, 2020, 05:23 PM
 * Description          : Definitions of variable
 */

#ifndef Variable_DEFINITIONS_H // Variable definitions
#define	Variable_DEFINITIONS_H

/*
#ifndef DEBUG_MODE_ON_H // Debug Mode On
#define	DEBUG_MODE_ON_H
#endif
*/

#ifndef Encryption_ON_H // Data Encryption On
#define	Encryption_ON_H
#endif


#ifndef LCD_DISPLAY_ON_H // LCD DISPLAY ON
#define	LCD_DISPLAY_ON_H
#endif
 

/*
#ifndef SMS_DELIVERY_REPORT_ON_H // SMS_DELIVERY_REPORT On
#define	SMS_DELIVERY_REPORT_ON_H
#endif
*/

/*
#ifndef DOL_DEFINITIONS_H // for direct online motor start
#define DOL_DEFINITIONS_H
#endif
*/

//*
#ifndef STAR_DELTA_DEFINITIONS_H // for star delta motor start
#define	STAR_DELTA_DEFINITIONS_H
#endif
//*/

// include processor files - each processor file is guarded.  
#include "congfigBits.h"


#define _XTAL_FREQ 64000000
#define I2C_BAUDRATE 100000
/** Macro definition for library functions that triggers "suspicious pointer conversion" warning#start **/
#define strncpy(a,b,c)     strncpy((char*)(a),(char*)(b),(c)) 
#define strncmp(a,b,c)     strncmp((char*)(a),(char*)(b),(c))
#define strlwr(a)      strlwr((char*)(a))
/*** Macro definition for library functions that triggers "suspicious pointer conversion" warning#end ***/

/***************************** Port line definition#start ****************************/
//Control lines

#define Irri_Out1   PORTDbits.RD2            // To control field1 valve
#define Irri_Out2   PORTDbits.RD3            // To control field2 valve
#define Irri_Out3   PORTDbits.RD4            // To control field3 valve
#define Irri_Out4   PORTDbits.RD7            // To control field4 valve
#define Irri_Out5   PORTEbits.RE7            // To control field5 valve
#define Irri_Out6   PORTFbits.RF0            // To control field6 valve
#define Irri_Out7   PORTFbits.RF1            // To control field7 valve
#define Irri_Out8   PORTFbits.RF2            // To control field8 valve
#define Irri_Out9   PORTFbits.RF3            // To control field9 valve
#define Irri_Out10  PORTFbits.RF4           // To control field10 valve
#define Irri_Out11  PORTFbits.RF5           // To control field11 valve
#define Irri_Out12  PORTFbits.RF6           // To control field12 valve
#define Irri_Out13  PORTFbits.RF7           // To control field13 valve
#define Irri_Out14  PORTGbits.RG4           // To control field14 valve
#define Irri_Out15  PORTGbits.RG6           // To control field15 valve
#define Irri_Out16  PORTGbits.RG6           // To control field16 valve
#define Irri_Motor  PORTHbits.RH0           // To control Motor Pump  -- DOL Red (Stop) Button (NO Relay connection)) // Deactivate DOL Manual Red button 
#define Irri_MotorT PORTHbits.RH1           // To control Capacitor Relay -- Irri_MotorT -- DOL (Start) Green Button (NO Relay connection))
#define Fert_Motor  PORTHbits.RH2           // To control fertigation valve
#define Filt_Out1   PORTBbits.RB3           // To control filtration1 valve
#define Filt_Out2   PORTBbits.RB4           // To control filtration2 valve
#define Filt_Out3   PORTBbits.RB5           // To control filtration3 valve
#define Filt_Out4   PORTCbits.RC0           // To control filtration4 valve
#define Filt_Out5   PORTCbits.RC1           // To control filtration5 valve
#define Filt_Out6   PORTCbits.RC2           // To control filtration6 valve
#define Fert_Inj1   PORTEbits.RE4           // To control injector1 valve
#define Fert_Inj2   PORTEbits.RE5           // To control injector2 valve
#define Fert_Inj3   PORTEbits.RE6           // To control injector3 valve
#define Fert_Inj4   PORTGbits.RG3           // To control injector4 valve


//Sensor Inputs
#define Mois_SNSR       PORTCbits.RC5           // Moisture sensor measurement
#define Irri_FlowSNSR   PORTEbits.RE2           // Irrigation Flow sensor measurement
#define Fert_FlowSNSR   PORTEbits.RE3           // Fertigation Flow sensor measurement
#define Ph_SNSR         PORTBbits.RB1           // Soil Ph sensor measurement
#define EC_SNSR         PORTBbits.RB2           // Soil Ec sensor measurement
#define Temp_SNSR       PORTBbits.RB6           // Wind speed sensor measurement
#define Wind_SNSR       PORTBbits.RB7           // Temperature sensor measurement
#define Pressure_SNSR1  PORTGbits.RG1           // Pressure1 sensor measurement
#define Pressure_SNSR2  PORTGbits.RB2           // Pressure2 sensor measurement



//Mux Control lines
#define Mux_S0  PORTAbits.RA0           // MUX ADDR1
#define Mux_S1  PORTAbits.RA1           // MUX ADDR2
#define Mux_S2  PORTAbits.RA2           // MUX ADDR3
#define Mux_S3  PORTAbits.RA3           // MUX ADDR4
#define Mux_EN  PORTAbits.RA4           // MUX Enable Control Line
#define Mux_Out PORTAbits.RA5           // Mux out sensor measurement


#define Run_led         PORTHbits.RH3            // System running indication
#define RTC_Trigger     PORTGbits.RG0            // RTC Battery measurement control liine
#define Phase_Input     PORTBbits.RB0            // 3 Phase input

/***************************** Port line definition#end ******************************/

/***************************** Analog pin definitions#start **************/
#define CT              0b00011000  //RD0
#define WindSpeed       0b00001111  //RB7
#define Temperature     0b00001110  //RB6
#define RTCBattery      0b00011001  //RD1
#define PhSNSR          0b00001001  //RB1
#define EcSNSR          0b00001010  //RB2
#define PressureSNSR1   0b00110001  //RG1
#define PressureSNSR2   0b00110010  //RG2
#define MuxOutput       0b00000101  //RA5
#define MuxEnable       0b00000100  //RA4
#define MuxS3           0b00000011  //RA3
#define MuxS2           0b00000010  //RA2
#define MuxS1           0b00000001  //RA1
#define MuxS0           0b00000000  //RA0



#define CTchannel           0
#define WSchannel           1
#define TPchannel           2
#define RTCchannel          3
#define PhSNSRchannel       4
#define EcSNSRchannel       5
#define PrsSNSR1channel     6
#define PrsSNSR2channel     7
#define MuxOutputchannel    8
/***************************** Analog pin definition#end ****************/

/***************************** Flowsensor pin definitions#start **************/
#define waterFlow           0
#define fertigationFlow     1
/***************************** Analog pin definition#end ****************/

/***************************** Device Reset Type definitions#start **************/
#define PowerOnReset 1
#define LowPowerReset 2
#define HardReset 3
#define SoftResest 4
#define WDTReset 5
#define StackReset 6
/***************************** Device Reset Type definition#end ****************/

/***************************** ICON pOSITION definitions#start **************/
#define blank_icon 9
#define clock_icon 0
#define irri_icon 1
#define filt_icon 2
#define fert_icon 3
#define dry_icon 4
#define phase_icon 5
#define battery_icon 6
#define sms_icon 7
#define bell_icon 8
/***************************** Device Reset Type definition#end ****************/

/***************************** Motor Load Type definitions#start **************/
#define FullLoad 1
#define NoLoad 2
/***************************** Motor Load Type definition#end ****************/

/***************************** Boolean definitions#start **************/
#define ON 1       
#define OFF 0 
#define SET 1
#define CLEAR 0      
#define HIGH 1
#define LOW 0
#define FULL 1
#define EMPTY 0       
#define INPUT 1
#define OUTPUT 0 
#define ENABLED 1
#define DISABLED 0
#define DARK 1
#define GLOW 0
/***************************** Boolean definition#end ****************/

/***************************** Field Valve structure declaration#start ***************/
struct FIELDVALVE {
    unsigned int dryValue;              // 16 BIT  --2 LOCATION 0,1
    unsigned int wetValue;              // 16 BIT  --2 LOCATION 2,3
    unsigned int onPeriod;             	// 16 BIT  --2 LOCATION 4,5
    unsigned int fertigationDelay;      // 16 BIT  --2 LOCATION 6,7
    unsigned int fertigationONperiod;   // 16 BIT  --2 LOCATION 8,9
    unsigned int injector1OnPeriod;     // 16 BIT  --2 LOCATION 10,11
    unsigned int injector2OnPeriod;     // 16 BIT  --2 LOCATION 12,13
    unsigned int injector3OnPeriod;     // 16 BIT  --2 LOCATION 14,15
    unsigned int injector4OnPeriod;     // 16 BIT  --2 LOCATION 16,17
    unsigned int injector1OffPeriod;    // 16 BIT  --2 LOCATION 18,19  
    unsigned int injector2OffPeriod;    // 16 BIT  --2 LOCATION 20,21
    unsigned int injector3OffPeriod;    // 16 BIT  --2 LOCATION 22,23
    unsigned int injector4OffPeriod;    // 16 BIT  --2 LOCATION 24,25
    unsigned char injector1Cycle;       //  8 BIT  --1 LOCATION 26
    unsigned char injector2Cycle;       //  8 BIT  --1 LOCATION 27
    unsigned char injector3Cycle;       //  8 BIT  --1 LOCATION 28
    unsigned char injector4Cycle;       //  8 BIT  --1 LOCATION 29
    unsigned char offPeriod;            //  8 BIT  --1 LOCATION 30
    unsigned char motorOnTimeHour;      //  8 BIT  --1 LOCATION 31
    unsigned char motorOnTimeMinute;    //  8 BIT  --1 LOCATION 32
    unsigned char nextDueDD;            //  8 BIT  --1 LOCATION 33
    unsigned char nextDueMM;            //  8 BIT  --1 LOCATION 34
    unsigned char nextDueYY;            //  8 BIT  --1 LOCATION 35
    unsigned char fertigationInstance;  //  8 BIT  --1 LOCATION 36
	unsigned char priority;             //  8 BIT  --1 LOCATION 37
    unsigned char fertigationStage;     //  8 BIT  --1 LOCATION 38
    unsigned char cycles;               //  8 BIT  --1 LOCATION 39
    unsigned char cyclesExecuted;       //  8 BIT  --1 LOCATION 40
    _Bool status;                       //  8 BIT  --1 LOCATION 41
    _Bool isConfigured;                 //  8 BIT  --1 LOCATION 42
    _Bool isFertigationEnabled;         //  8 BIT  --1 LOCATION 43
    _Bool fertigationValveInterrupted;  //  8 BIT  --1 LOCATION 44
};
/***************************** Field Valve structure declaration#end *****************/

/***************************** LCD Char structure declaration#start ***************/
unsigned char clock[8]     = {0x00,0x0e,0x15,0x17,0x11,0x0e,0x00,0x00};
unsigned char bell[8]      = {0x04,0x0e,0x0e,0x0e,0x1f,0x00,0x04,0x00};
unsigned char irri[8]      = {0x15,0x0E,0x15,0x0E,0x04,0x04,0x04,0x00};
unsigned char fert[8]      = {0x04,0x0e,0x1f,0x1f,0x1f,0x0e,0x04,0x04};
unsigned char sms[8]       = {0x00,0x00,0x1F,0x11,0x1B,0x15,0x1F,0x00};
unsigned char filt[8]      = {0x04,0x07,0x1C,0x07,0x1C,0x07,0x1C,0x04};
unsigned char dry[8]       = {0x00,0x00,0x04,0x0a,0x15,0x1f,0x00,0x00};
unsigned char check[8]     = {0x00,0x01,0x03,0x16,0x1c,0x08,0x00,0x00};
unsigned char phase[8]     = {0x02,0x04,0x08,0x18,0x06,0x04,0x08,0x10};
unsigned char battery[8]   = {0x1F,0x11,0x15,0x15,0x11,0x15,0x11,0x1F};
unsigned char blank[8]     = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
unsigned char * charmap[10] = {clock, irri, filt, fert, dry, phase, battery, sms, bell,blank};
/***************************** LCD Char structure declaration#end ***************/

/************* statically allocated initialized user variables#start *****/
#pragma idata fieldValve
struct FIELDVALVE fieldValve[16] = {0}; //Initialize field valve structure to zero
/************* statically allocated initialized user variables#end *******/


/***************************** EEPROM Address definition#start ***********************/
/** statically allocated initialized user variables#start **/
#pragma idata eepromAddress
                                      // 0        1        2       3       4       5       6       7       8       9       10      11      12      13      14      15      16      17      18      19      20      21
const unsigned int eepromAddress[22] = {0x0000, 0x0030, 0x0060, 0x0090, 0x00C0, 0x00F0, 0x0120, 0x0150, 0x0180, 0x01B0, 0x01E0, 0x0210, 0x0240, 0x0270, 0x02A0, 0x02D0, 0x0300, 0x0330, 0x0360, 0x0390, 0x03C0, 0x03F0}; //EEPROM Address locations from 0x00 t0 0x3FF ~1024KB
/*** statically allocated initialized user variables#end ***/
/***************************** EEPROM Address definition#end *************************/

/***************************** Macros for EEPROM Address location#start **************/
#define field1 0        // To store Field1 Valve data
#define field2 1        // To store Field2 Valve data
#define field3 2        // To store Field3 Valve data
#define field4 3        // To store Field4 Valve data
#define field5 4        // To store Field5 Valve data
#define field6 5        // To store Field6 Valve data
#define field7 6        // To store Field7 Valve data
#define field8 7        // To store Field8 Valve data
#define field9 8        // To store Field9 Valve data
#define field10 9       // To store Field10 Valve data
#define field11 10      // To store Field11 Valve data
#define field12 11      // To store Field12 Valve data
#define field13 12      // To store Field13 Valve data
#define field14 13      // To store Field14 Valve data
#define field15 14      // To store Field15 Valve data
#define field16 15      // To store Field16 Valve data
#define forFiltration 16// To store filtration Valve data 
#define forSystem 17    // To store other system values //forSystem + 13 used
#define forPassword 18  // To store 6 byte user password //forPassword + 12 used
#define forFieldMap 19  // To store field mapping from 19 to 20 location (96 location)
#define forDeviceID 21  // To store 12 byte DeviceID.
/***************************** Macros for EEPROM Address location#end ****************/

/***************************** Macros for Fertigation stages #start ****************/
#define wetPeriod 1   
#define injectPeriod 2
#define flushPeriod 3
/***************************** Macros for Fertigation stages #end ****************/

/***************************** Macros for Additional Info in SMS #start ****************/
#define noInfo 0                // No additional info required
#define newAdmin 1              // To check if password is successfully reset by another user.
#define fieldNoRequired 2       // TO mention field no in SMS
#define timeRequired 3          // To indicate time in SMS
#define secretCodeRequired 4    // To give secret factory code
#define motorLoadRequired 5    // To give motor load values
#define frequencyRequired 6    // To give motor load values
#define IrrigationData 7       // To give Irrigation values
#define filtrationData 8       // To give Filtration values
/***************************** Macros for Additional Info in SMS #end ****************/

/***************************** Global variables definition#start *********************/
/***** System Config definition#start *************************/
unsigned int filtrationSeperationTime = CLEAR; // To store filtration SeperationTime1 in minutes
unsigned int dueDD = CLEAR; // To store calculated due day in DD format
unsigned int sleepCount = CLEAR; // To store sleep count -- max 65550
unsigned int remainingFertigationOnPeriod = CLEAR; //To store remaining sleep count for interrupted fertigation valve
unsigned int lower8bits = CLEAR; // To store lower 8 bits for 16 bit operations
unsigned int higher8bits = CLEAR; // To store higher 8 bits for 16 bit operations
unsigned int moistureLevel = CLEAR; // To store moisture level in Hz
unsigned int injector1OnPeriod = CLEAR; // to store injector 1 on period
unsigned int injector2OnPeriod = CLEAR; // to store injector 2 on period
unsigned int injector3OnPeriod = CLEAR; // to store injector 3 on period
unsigned int injector4OnPeriod = CLEAR; // to store injector 4 on period
unsigned int injector1OnPeriodCnt = CLEAR; // to store injector 1 on period count
unsigned int injector2OnPeriodCnt = CLEAR; // to store injector 2 on period count
unsigned int injector3OnPeriodCnt = CLEAR; // to store injector 3 on period count
unsigned int injector4OnPeriodCnt = CLEAR; // to store injector 4 on period count
unsigned int injector1OffPeriod = CLEAR; // to store injector 1 on period
unsigned int injector2OffPeriod = CLEAR; // to store injector 2 on period
unsigned int injector3OffPeriod = CLEAR; // to store injector 3 on period
unsigned int injector4OffPeriod = CLEAR; // to store injector 4 on period
unsigned int injector1OffPeriodCnt = CLEAR; // to store injector 1 on period count
unsigned int injector2OffPeriodCnt = CLEAR; // to store injector 2 on period count
unsigned int injector3OffPeriodCnt = CLEAR; // to store injector 3 on period count
unsigned int injector4OffPeriodCnt = CLEAR; // to store injector 4 on period count
unsigned int noLoadCutOff = CLEAR;
unsigned int fullLoadCutOff = CLEAR;
unsigned char userMobileNo[11] = ""; // To store 10 byte user mobile no.


//unsigned char null[11] = {'\0'}; // Null.

unsigned char injector1Cycle = CLEAR;
unsigned char injector2Cycle = CLEAR;
unsigned char injector3Cycle = CLEAR;
unsigned char injector4Cycle = CLEAR;
unsigned char injector1CycleCnt = CLEAR;
unsigned char injector2CycleCnt = CLEAR;
unsigned char injector3CycleCnt = CLEAR;
unsigned char injector4CycleCnt = CLEAR;
unsigned char loraAliveCount = CLEAR;
unsigned char loraAliveCountCheck = CLEAR;
unsigned char loraAttempt = CLEAR;
unsigned char timer3Count = CLEAR; // To store timer 0 overflow count
unsigned char rxCharacter = CLEAR; // To store received 1 byte character from GSM through RX pin
unsigned char msgIndex = CLEAR; // To point received character position in Message
unsigned char iterator = CLEAR; // To navigate through iteration in for loop
unsigned char iterator_field = CLEAR;
unsigned char fieldCount = 16;   // To Store no. of fields to configure
unsigned char resetCount = CLEAR; // To store count of reset occurred by MCLR Reset for menu option
unsigned char startFieldNo = 0;  // To indicate starting field irrigation valve no. for scanning
unsigned char space = 0x20; // Represents space Ascii
unsigned char terminateSms = 0x1A; // Represents Ctrl+z to indicate end of SMS
unsigned char enter = 0x0D; // Represents Enter Key ASCII
unsigned char newLine = 0x0A; // Represents New Line Key ASCII
unsigned char hundredsDigit = CLEAR; // To store hundred's digit no. "1XX"
unsigned char tensDigit = CLEAR; // To store ten's digit no. "X1X"
unsigned char unitsDigit = CLEAR; // To store unit's digit no. "XX1"
unsigned char Timer0Overflow = CLEAR; // To store timer0 overflow count
unsigned char Timer1Overflow = CLEAR; // To store timer1 overflow count
unsigned char Timer3Overflow = CLEAR; // To store timer1 overflow count
unsigned char filtrationCycleSequence = CLEAR; // To store filtration  cycle sequence
unsigned char currentYY = CLEAR; // To store extracted year from received date from GSM in YYYY format
unsigned char currentMM = CLEAR; // To store extracted month from received date from GSM in MM format
unsigned char currentDD = CLEAR; // To store extracted day from received date from GSM in DD format
unsigned char currentHour = CLEAR; // To store extracted hour time from received date from GSM in hr format
unsigned char currentMinutes = CLEAR; // To store extracted minutes time from received date from GSM in mm format
unsigned char currentSeconds = CLEAR; // To store extracted seconds time from received date from GSM in mm format
unsigned char dueYY = CLEAR; // To store calculated due year in YYYY format
unsigned char dueMM = CLEAR; // To store calculated due month in MM format
unsigned char filtrationDelay1 = CLEAR; // To store filtration Delay1 in minutes
unsigned char filtrationDelay2 = CLEAR; // To store filtration Delay2 in minutes
unsigned char filtrationDelay3 = CLEAR; // To store filtration Delay3 in minutes
unsigned char filtrationOnTime = CLEAR; // To store filtration OnTime in minutes
unsigned char dryRunCheckCount = CLEAR; // To store dry run check count
unsigned char currentFieldNo = CLEAR; // To store running field no.
unsigned char pulses = CLEAR; // To store no. of pulses for water /fertigation flow sensor
char fieldByte[3] = ""; //To store field no.
char temporaryBytesArray[26] = ""; // To store 26 byte buffer.
char deviceId[] = "a0d1d8668dd8"; // To store 12 byte user device Id.
char dueDate[15] = "";  // To store schedule date.
char pwd[7] = ""; // To store 6 byte user set password.
char factryPswrd[7] = ""; // To store 6 byte factory password until authentication
size_t temp = CLEAR; // Temporary buffer
/***** System Config definition#end *************************/

/******Data Encryption and Decryption#start *****************/
#ifdef Encryption_ON_H
//#pragma idata encodedString 
//unsigned char encodedString[10] = {'\0'};
//#pragma idata stringToEncode
//unsigned char stringToEncode[10] = {'\0'};
//#pragma idata stringToDecode
//unsigned char stringToDecode[200] = {'\0'};
#endif
#pragma idata decodedString
unsigned char decodedString[200] = {'\0'};

#pragma idata mqtt_ca
const char mqtt_ca[] =
"-----BEGIN CERTIFICATE-----\n"
"MIID+DCCAuACFAY6oD+AjH6MNfBfDEhghQAR0Y6GMA0GCSqGSIb3DQEBCwUAMIGv\n"
"MQswCQYDVQQGEwJJTjEUMBIGA1UECAwLTUFIQVJBU0hUUkExDzANBgNVBAcMBk1V\n"
"TUJBSTEZMBcGA1UECgwQQkhPT01JTVFUVEJyb2tlcjEXMBUGA1UECwwOSW9UIEly\n"
"cmlnYXRpb24xFjAUBgNVBAMMDUJIT09NSU1RVFQgQ0ExLTArBgkqhkiG9w0BCQEW\n"
"HmJob29taS5qYWxhc2FuZGhhcmFuQGdtYWlsLmNvbTAgFw0yNTA3MjEwNTQ2MTRa\n"
"GA8yMTI1MDYyNzA1NDYxNFowgb4xCzAJBgNVBAYTAklOMRQwEgYDVQQIDAtNQUhB\n"
"UkFTSFRSQTEPMA0GA1UEBwwGTVVNQkFJMRkwFwYDVQQKDBBCSE9PTUlNUVRUQlJP\n"
"S0VSMRcwFQYDVQQLDA5Jb1QgSXJyaWdhdGlvbjElMCMGA1UEAwwcbXF0dC5iaG9v\n"
"bWlqYWxhc2FuZGhhcmFuLmNvbTEtMCsGCSqGSIb3DQEJARYeYmhvb21pLmphbGFz\n"
"YW5kaGFyYW5AZ21haWwuY29tMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKC\n"
"AQEAqUOcfhmoa+fwGr01jek+aeid53JNAyyd/MT46CKRG1+oio0WrKSdfUj88kTM\n"
"01beAWUCPhz+NQVs6B+a9N0DXmpfqaCNd80yZbVE2Di4aTAixgxf2KLNV6gH382+\n"
"D1Hl8XN9MtsLcG4z2ijGS+5YQrGxvHWouI9mkRs5733c3taTIrJuQUQwbojbAid3\n"
"gw4xPChf15NHqGz9OTKJj4bPHgJ58qVqCG6qa/LczuQX1q9tLcQ/CLqdIGTpdtl7\n"
"l3GmgwRcVqOGLkPP7Q8ZuIV9o+sJ9Yh8+xUPcHl6/phR7qcBWcMEYrfw+OW0Ry+y\n"
"vmhyPVAiVOSLNsZTlC2domrOdwIDAQABMA0GCSqGSIb3DQEBCwUAA4IBAQBrcwXB\n"
"PRY0ksTTJbKTMh6qPjCRnvr0XXqqOlZlACWXS/awSEuvWdkERz7il1kmc5TqvYA0\n"
"0ZnIVSqKD3OpO+gfJxMD8qNhGoxwuYL6jdAwH0kAsa1xmu6hWYbihj5bNhQpfCiN\n"
"1inqwKjz1pqqp4/ALfHb0u6BMOo3ILA/I0Tgr1ef0R7v4Q1cquOyf1R10DjDijPN\n"
"3pqfXap+SSY4+p4eDmbRtKYX428lIo66msSFZ83VFcVCO1p+5A8COA9pyIFu+cdI\n"
"Ab/UfH5y8lvgCUbONd/dlxEretffUd0aiZX4YQ5Dc3tl8sQufeYygoRB/wCzFPqE\n"
"6Ecpy1ZNAwT7cwwK\n"
"-----END CERTIFICATE-----\n";

/***** Data Encryption and Decryption#end *********************/

/***** SMS prototype definition#start *************************/
const char admin[3] = "AU"; // To Administer device
const char alive1[6] = "ALIVE"; // To connect with device
const char set[4] = "SET"; // To Configure valve settings
const char enable[7] = "ENABLE"; // To enable fertigation valve
const char disable[8] = "DISABLE"; // To disable fertigation valve
const char active[7] = "ACTIVE"; // To ACTIVATE filtration valve
const char dactive[8] = "DACTIVE"; // To De-Activate filtration valve
const char hold[5] = "HOLD"; // To hold irrigation valve settings
const char extract[8] = "EXTRACT"; // To extract diagnostic data
const char ok[3] = "ok"; // Acknowledge from GSM	
const char time[5] = "TIME"; // To get current time from RTC
const char feed[5] = "FEED"; // To Set current time into RTC
const char fdata[6] = "FDATA"; // To get filtration cycle data
const char inject[7] = "INJECT"; // To Inject Test Data
const char map[4] = "MAP"; // To map Field valve with LORA Slaves
const char ct[4] = "CT "; // To set motor load values manually
const char setct[6] = "CTSET"; // To set motor load condition through diagnostic
const char secret[12] = "12345678912"; //Secret code to fetch unique factory password
const char getct[4] = "CTV"; // get ct values
const char getfreq[5] = "FREQ"; // get FREQ values
const char countryCode[4] = "+91"; //Country code for GSM
const char mqttConnect[19] = "+CMQTTCONNECT: 0,0"; // successful MQTT connect
/***** SMS prototype definition#end ***************************/

/***** LORA prototype definition#start *************************/
const char on[3] = "ON"; //
const char off[4] = "OFF"; //
const char slave[6] = "SLAVE"; // 
const char ack[4] = "ACK"; // 
const char idle[5] = "IDLE"; // 
const char master[7] = "MASTER"; //
const char error[6] = "ERROR"; //
const char alive[6] = "ALIVE";
const char sensor[7] = "SENSOR";
const char lowbattery[16] = "LOWBATTERYSLAVE";
const char resetslave[11] = "RESETSLAVE";
const char cmqttError[14] = "CMQTTCONNLOST";
/***** LORA prototype definition#end ***************************/

//const char deviceId[13] = "a0d1d8668dd8"; //4269C02CC071
//const char topic[8] = "command"; //COMMAND
//const char Subscribe[25] = "a0d1d8668dd8/command\r\n"; //Subscribe Topic *AU 30b15e313ebe #

//const char topic_publish[9] = "response";//RESPONSE
//const char Publish[26] = "4269c02cc071/response\r\n" ;//Publish Topic

/***** SMS strings definition#start *************************/
//const char SmsAU1[23] = "Admin set successfully"; // Acknowledge user about successful Admin Registration
//const char SmsAU2[51] = "You are no more Admin now. New Admin is set to\r\n"; //To notify old Admin about new Admin.
//const char SmsAU3[22] = "Authentication failed"; // Acknowledge user about Authentication failed
const char SmsAU4_63[64] = "System Authenticated with Phase failure, suspending all actions"; // Acknowledge user about System Authenticated with Phase failure

//const char SmsPwd1[32] = "Login code changed successfully"; // Acknowledge user about successful password update
//const char SmsPwd2[23] = "Login code not changed";
//const char SmsPwd3[23] = "Wrong login code found"; // Acknowledge user about successful motor off action

const char cmd1_6[7]    = "*AUOK#";
const char cmd2_9[10]   = "*AUERROR#";
const char cmd3_9[10]   = "*ALIVEOK#";
const char cmd4_7[8]    = "*SETOK#";
const char cmd5_8[9]    = "*HOLDOK#";
const char cmd6_10[11]  = "*ENABLEOK#";
const char cmd7_17[18]  = "*ENABLEINCORRECT#";
const char cmd8_13[14]  = "*ENABLEERROR#";
const char cmd9_11[12]  = "*DISABLEOK#";
const char cmd10_10[11] = "*ACTIVEOK#";
const char cmd11_11[12] = "*DACTIVEOK#";
const char cmd12_13[14] = "*ACTIVEERROR#";
const char cmd13_8[9]   = "*FEEDOK#";
const char cmd14_11[12] = "*FEEDERROR#";
const char cmd15_6[7]   = "*CTOK#";
const char cmd16_9[10]  = "*CTERROR#";
const char cmd17_11[12] = "*FREQERROR#";
const char cmd18_12[13] = "*CTSETERROR#";
const char cmd18_7[8]   = "*MAPOK#";
const char cmd19_10[11] = "*MAPERROR#";
//const char cmd20[42]  = "*CT<SAPCE><NoLOADVALUE><SPACE><FULLLOAD>#";
//const char cmd17[42]  = "*CT<SAPCE><NoLOADVALUE><SPACE><FULLLOAD>#";
//const char cmd18[7]   = "*FREQ ";
//const char cmd14[30]  = "*T<SPACE><DD MM YY HH MM SS>#";
//const char cmd2[12]   = "*BATTERYOK#";
//const char cmd3[15]   = "*BATTERYERROR#";


//const char CmdIrr1[33]  = "Irrigation configured for field "; // Acknowledge user about successful Irrigation configuration
//const char CmdIrr2[45]  = "Irrigation configuration disabled for field "; // Acknowledge user about successful Irrigation configuration disable action
//const char CmdIrr3[37]  = "Irrigation not configured for field "; // Acknowledge user about  Irrigation not configured
const char NotIrr4_30[]  = "Valve started for field "; // Acknowledge user about successful Irrigation started action
const char NotIrr5_30[]  = "Valve stopped for field "; // Acknowledge user about successful Irrigation stopped action
const char NotIrr6_54[]  = "Wet field detected. Valve not started for field "; // Acknowledge user about Irrigation not started due to wet field detection
const char NotIrr7_16[]  = "Irrigation No:"; // Send diagnostic data for irrigation
const char NotIrr8_48[]  = "Valve skipped with no response from field "; // Acknowledge user about Irrigation skipped due to no response
const char NotIrr9_48[]  = "Valve stopped without response from field "; // Acknowledge user about Irrigation stopped without response
//const char CmdIrr10[36] = "Irrigation field mapped with valves"; // Acknowledge user about Irrigation valves mapped with respective fields

//const char CmdFert1[61] = "Irrigation is not Active. Fertigation not enabled for field "; // Acknowledge user about Fertigation not configured due to disabled irrigation
//const char CmdFert2[53] = "Incorrect values. Fertigation not enabled for field "; // Acknowledge user about Fertigation not configured due to incorrect values
//const char CmdFert3[31] = "Fertigation enabled for field "; // Acknowledge user about successful Fertigation enabled action
//const char CmdFert4[32] = "Fertigation disabled for field "; // Acknowledge user about successful Fertigation disabled action
const char NotFert5_33[] = "Injector started for field "; // Acknowledge user about successful Fertigation started action
const char NotFert6_33[] = "Injector stopped for field "; // Acknowledge user about successful Fertigation stopped action
const char NotFert7_70[] = "Injector stopped with fertilizer level sensor failure for field "; // Acknowledge user about Fertigation stopped action with sensor failure
const char NotFert8_59[] = "Injector stopped with low fertilizer level for field "; // Acknowledge user about Fertigation stopped action with low fertilizer level 

//const char CmdFilt1[27] = "Water filtration activated";
//const char CmdFilt2[29] = "Water filtration deactivated"; 
//const char CmdFilt3[32] = "Water Filtration is not enabled";
//const char CmdFilt4[27] = "Water Filtration Sequence:";

const char NotSR01_59[] = "System restarted with phase failure, suspending all actions"; // Acknowledge user about system restarted with Valve action
const char NotSR02_76[] = "System restarted for Power Interrupt with incomplete Irrigation for field "; // Acknowledge user about system restarted with Valve action
const char NotSR03_73[] = "System restarted for Low Power In with incomplete Irrigation for field "; // Acknowledge user about system restarted with Valve action
const char NotSR04_75[] = "System restarted in Diagnostic Mode with incomplete Irrigation for field "; // Acknowledge user about system restarted with Valve action
const char NotSR05_80[] = "System restarted for All Phase Detection with incomplete Irrigation for field "; // Acknowledge user about system restarted with Valve action
const char NotSR06_72[] = "System restarted for WDT Timeout with incomplete Irrigation for field "; // Acknowledge user about system restarted with Valve action
const char NotSR07_72[] = "System restarted for Stack Error with incomplete Irrigation for field "; // Acknowledge user about system restarted with Valve action
const char NotSR08_36[] = "System restarted for Power Interrupt"; // Acknowledge user about system restarted with No Valve action
const char NotSR09_30[] = "System restarted for Low Power"; // Acknowledge user about system restarted with No Valve action
const char NotSR10_35[] = "System restarted in Diagnostic mode"; // Acknowledge user about system restarted with No Valve action
const char NotSR11_40[] = "System restarted for All Phase Detection"; // Acknowledge user about system restarted with No Valve action
const char NotSR12_32[] = "System restarted for WDT timeout"; // Acknowledge user about system restarted with No Valve action
const char NotSR13_32[] = "System restarted for stack error"; // Acknowledge user about system restarted with No Valve action
const char NotSR14_58[] = "System reset occurred, login code reset to Factory setting"; // Acknowledge user about successful motor off action
const char NotSR15_49[] = "System reset occurred, Irrigation setting deleted"; // Acknowledge user about successful motor off action

const char NotRTC1_30[]   = "Please replace RTC battery"; // Acknowledge user about replace RTC battery
const char NotRTC2_6[]     = "*RLOW#"; // Acknowledge server about low RTC battery
const char NotRTC3_7[]     = "*RFULL#"; // Acknowledge server about full RTC battery
const char NotRTC4_36[]   = "System time is set to local time";
//const char NotRTC5_55[56]   = "Please sync system time manually"; // Acknowledge user about setting RTC through Mobile App
const char NotRTC6_36[]   = "Please sync system time manually"; 
//const char CmdRTC2[41] = "System time synced to current local time"; 

const char NotSLV1_39[]   = "Please replace Battery for Field "; // Acknowledge user about replace Field battery
const char NotSLV2_6[]     = "*FLOW#"; // Acknowledge server about low Field battery
const char NotSLV3_7[]     = "*FFULL#"; // Acknowledge server about full Field battery
const char NotSLV4_38[]   = "Battery Reset Occured for Field ";
//const char NotSLV5[56] = "New RTC battery found, please sync system time manually"; // Acknowledge user about setting RTC through Mobile App
//const char CmdRTC2[41] = "System time synced to current local time"; 

const char NotDR1_84[]   = "Action Suspended. Irrigation scheduled to next day with fertigation for Field "; // Acknowledge user about successful motor off action
const char NotDR2_84[]   = "Action Suspended. Irrigation scheduled to next day with fertigation for Field "; // Acknowledge user about successful motor off action
const char NotDR3_67[]    = "Action Suspended. Irrigation scheduled to next day for Field "; // Acknowledge user about successful motor off action
const char NotDR4_72[]    = "Action Suspended. Irrigation scheduled to next due date for Field "; // Acknowledge user about successful motor off action

//const char CmdT1[27] = "Incorrect local time found"; 
//const char CmdT2[15] = "Current Time: "; 

const char NotMotor1_55[] = "Action completed for due fields. Motor switched off"; // Acknowledge user about successful motor off action
//const char CmdMotor2[35] = "Motorload cut-off set successfully"; // Acknowledge user about successful motor off action
//const char CmdMotor3[47] = "NoLoad and FullLoad Motor thresholds set to : "; // Acknowledge user about motor load values
//const char CmdMotor4[49] = "Irrigation is active, Motor load cut-off not set"; // Acknowledge user about Irrigation is active, Motor load cut-off procedure not started

//const char CmdConnect[17] = "System Connected"; // Acknowledge user about successful connection

//const char SmsTest[19] = "Test Data Injected"; 

//const char SmsFact1[15] = "Factory Key : "; // Factory key details

const char NotPh1_50[] = "Phase failure detected, suspending all actions"; // Acknowledge user about Phase failure status
const char NotPh2_72[] = "Low Phase current detected, actions suspended, please restart system"; // Acknowledge user about successful motor off action
const char NotPh3_23[] = "Phase loss detected"; // Acknowledge user about phase failure status
//const char SmsPh4[25] = "Phase Y failure detected"; // Acknowledge user about phase failure status
//const char SmsPh5[25] = "Phase B failure detected"; // Acknowledge user about phase failure status
const char NotPh6_22[] = "All Phase detected"; // Acknowledge user about phase status

const char NotMS1_57[] = "Moisture sensor is failed, Valve started for field "; // Acknowledge user about failure in moisture sensor
//const char CmdMS2[43] = "Moisture sensor frequency value for field "; // Acknowledge user measured moisture sensor value
//const char CmdMS3[37] = "Moisture sensor is failed for field "; // Acknowledge user about failure in moisture sensor
/***** SMS string definition#end ***************************/

/***** statically allocated initialized user variables#start **/
#pragma idata gsmResponse 
char gsmResponse[200] = "HELLO"; // To store the received message from GSM -- message can be command response or SMS
#pragma idata fieldMap 
char fieldMap[96] = {'\0'}; // To store mapped valves with field no. (16 fields))
/***** statically allocated initialized user variables#end ****/


/************* Booleans definition#start *********************************/ 
_Bool deviceIdSet = false;                      // To check if system is set for device id.
_Bool systemAuthenticated = false;              // To check if system is initialized by user for first time.
_Bool newSMSRcvd = false;                       // To check if communication is first initialized by GSM.
_Bool checkMoistureSensor = false;              // To check status of Moisture sensor																					 
_Bool moistureSensorFailed = false;             // status of Moisture sensor
_Bool controllerCommandExecuted = false;        // To check response to system cmd.
_Bool currentDateCalled = false;                // To avoid repetitive fetching of date through GSM
_Bool sleepCountChangedDueToInterrupt = false;  // TO check if sleep count need to calculate again if change due to GSM interrupt
_Bool inSleepMode = false;                      // To check if system is in sleep mode
_Bool dryRunDetected = false;                   // To state water level of tank --true for Low and --false for Full
_Bool lowPhaseCurrentDetected = false;          // To state motor not started due to low phase current
_Bool valveDue = false;                         // To indicate if any valve is due for operation after checking
_Bool valveExecuted = false;                    // To indicate if valve executed
_Bool onHold = false;                           // To indicate Field valve hold status
_Bool dueValveChecked = false;					// To indicate valve due is checked at latest 
_Bool correctDate = false;          			// To indicate received date is correct
_Bool phaseFailureDetected = false;             // To indicate phase failure
_Bool lowRTCBatteryDetected = false;            // To store RTC Battery level Status
_Bool rtcBatteryLevelChecked = false;           // To indicate condition to check RTC battery level
_Bool phaseFailureActionTaken = false;          // To indicate action taken after phase failure detection
_Bool filtrationEnabled = false;                // To store tank filtration operation status    
_Bool cmtiCmd = false;                          // Set to indicate cmti command received
_Bool DeviceBurnStatus = false;                 // To store Device program status
_Bool gsmSetToLocalTime = false;                // To indicate if gsm set to local timezone
_Bool cmdRceived = false;                       // Set to indicate lora command received
_Bool checkLoraConnection = false;
_Bool LoraConnectionFailed = false;
_Bool wetSensor = false;                        // To indicate if sensor is wet
_Bool fertigationDry = false;                   // To indicate fertigation level
_Bool isLoraSlaveAlive = false;                 // To indicate lora slave is Alive
_Bool checkWaterFlow = false;                   // To indicate water flow measurement is on.
_Bool checkFertFlow = false;                    // To indicate fertigation flow measurement is on.
_Bool isPulseOn = false;                        // To indicate pulse is on.
_Bool isOK = false;                             // To indicate OK Response of AT commands.
_Bool isERROR = false;                          // To indicate ERROR Response of AT commands.
_Bool isErrorActionTaken = false;               // To indicate Action on ERROR Response of AT commands.
_Bool isNotification = false;                   // To indicate notification received from MQTT Broker
_Bool isValveConfigured = false;                // To indicate at-least one is configured
_Bool msgStart = false;                         // To indicate msg started
_Bool atcmdStart = false;                       // To indicate AT cmd started
//_Bool lcdCmdExecuted = false;                 // To indicate lcd cmd executed
_Bool lowBattery = false;                       // To indicate low battery for slave
_Bool resetSlave = false;                       // To indicate slave is reset
_Bool deviceIDFalg = false;                    // To indicate DeviceID is set for reset action
/************* BOOLeans definition#end ***********************************/
/***************************** Global variables definition#end ***********************/

#endif
/* DOL_DEFINITIONS_H */