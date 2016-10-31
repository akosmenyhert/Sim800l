/* this library is writing by  Cristian Steib.
 *      steibkhriz@gmail.com
 *  Designed to work with the GSM Sim800l, maybe work with SIM900L
 *
 *  This library use SoftwareSerial, you can define de RX and TX pin in the
 *  header "Sim800l.h" ,by default the pin is RX=10 TX=11.. be sure that gnd
 *  is attached to arduino too.
 *  You can also change the other preferred RESET_PIN
 *
 *  Esta libreria usa SoftwareSerial, se pueden cambiar los pines de RX y TX
 *  en el archivo header, "Sim800l.h", por defecto los pines vienen
 *  configurado en RX=10 TX=11.
 *  Tambien se puede cambiar el RESET_PIN por otro que prefiera
 *
 *    PINOUT:
 *        _____________________________
 *       |  ARDUINO UNO >>>   SIM800L  |
 *        -----------------------------
 *            GND      >>>   GND
 *        RX  10       >>>   TX
 *        TX  11       >>>   RX
 *       RESET 2       >>>   RST
 *
 *   POWER SOURCE 4.2V >>> VCC
 *
 *    Created on: April 20, 2016
 *        Author: Cristian Steib
 *
 *
 */

#include "Arduino.h"
#include "Sim800l.h"
#include <SoftwareSerial.h>

SoftwareSerial SIM(RX_PIN, TX_PIN);

const char PRG_COMMA[]=",";
const char PRG_COLON[]=":";
const char PRG_SLASH[]="/";
const char PRG_ZERO[]="0";
const char PRG_OK[]="OK";
const char PRG_ERR[]="ERR";
const char PRG_AT[]="AT\r\n";
const char PRG_CRLF[]="\r\n";

void Sim800l::begin()
{
    SIM.begin(9600);
#if (LED)
    pinMode(OUTPUT, LED_PIN);
#endif
   // zero terminate string buffer
    _buffer[0]=0;
}

/* PRIVATE METHODS */

// returns LENGTH of string stored at _buffer
int Sim800l::_readSerial()
{
    int bufferLength=0;

    while (1) {
        _timeout = 12000;
        while (!SIM.available() && _timeout > 0) {
            delay(13);
            _timeout--;
        }
        if (!_timeout) {
            // terminate string
            _buffer[bufferLength]=0;
            return bufferLength;
        }
        while (SIM.available()) {
            // prevent overrun
            if (bufferLength<BUFLEN) {
                _buffer[bufferLength]=SIM.read();
                bufferLength++;
            }
        }
    }
}

/* PUBLIC METHODS */

void Sim800l::reset()
{
#if (LED)
    digitalWrite(LED_PIN, 1);
#endif
    digitalWrite(RESET_PIN, 1);
    delay(1000);
    digitalWrite(RESET_PIN, 0);
    delay(1000);
    // wait for the module response
    while (1) {
        SIM.print(PRG_AT);
        _readSerial();
        // got "OK"? leave loop
        if (strstr_P(_buffer,PRG_OK) != NULL) {
            break;
        }
    }
    // wait for sms ready
    while (1) {
        _readSerial();
        // found "SMS"? leave loop
        if (strstr(_buffer,"SMS") != NULL) {
            break;
        }
    }
#if (LED)
    digitalWrite(LED_PIN, 0);
#endif
}

void Sim800l::setPhoneFunctionality()
{
    /* AT+CFUN=<fun>[,<rst>]
       Parameters
       <fun>
         0 Minimum functionality
         1 Full functionality (Default)
         4 Disable phone both transmit and receive RF circuits.
       <rst>
         1 Reset the MT before setting it to <fun> power level.
     */
    SIM.print(F("AT+CFUN=1\r\n"));
}
void Sim800l::signalQuality()
{
    /* Response
       +CSQ: <rssi>,<ber>Parameters
       <rssi>
       0      -115 dBm or less
       1      -111 dBm
       2...30 -110... -54 dBm
       31     -52 dBm or greater
       99     not known or not detectable
       <ber> (in percent):
       0...7  As RXQUAL values in the table in GSM 05.08 [20]
              subclause 7.2.4
       99     Not known or not detectable
     */
    SIM.print(F("AT+CSQ\r\n"));
    _readSerial();
    Serial.println(_buffer);
}

void Sim800l::activateBearerProfile()
{
    // set bearer parameter
    SIM.print(F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\" \r\n"));
    // eat response
    _readSerial();

    // set apn
    SIM.print(F("AT+SAPBR=3,1,\"APN\",\"internet\" \r\n"));
    // eat response
    _readSerial();

    // activate bearer context
    SIM.print(F("AT+SAPBR=1,1 \r\n"));
    delay(1200);
    // eat response
    _readSerial();

    // get context ip address
    SIM.print(F("AT+SAPBR=2,1\r\n "));
    delay(3000);
    // eat response
    _readSerial();
}

void Sim800l::deactivateBearerProfile()
{
    SIM.print(F("AT+SAPBR=0,1\r\n "));
    delay(1500);
    // eat response
    _readSerial();
}

bool Sim800l::answerCall()
{
    SIM.print(F("ATA\r\n"));
    _readSerial();
    // Response in case of data call, if successfully connected
    return (strstr_P(_buffer,PRG_OK)!=NULL);
}

void Sim800l::callNumber(char *number)
{
    SIM.print(F("ATD"));
    SIM.print(number);
    SIM.print(PRG_CRLF);
}

int Sim800l::getCallStatus()
{
    /*
       values of return:
       0 Ready (MT allows commands from TA/TE)
       2 Unknown (MT is not guaranteed to respond to tructions)
       3 Ringing (MT is ready for commands from TA/TE, but the ringer is active)
       4 Call in progress
       -1 error reading status (ADDED --lornix)
     */
    SIM.print(F("AT+CPAS\r\n"));
    _readSerial();
    char* pos=strstr(_buffer,"+CPAS: ");
    if (pos==NULL) {
        // oops, nothing found
        return -1;
    }
    return atoi(pos+7);
    // return _buffer.substring(_buffer.indexOf("+CPAS: ") + 7, _buffer.indexOf("+CPAS: ") + 9).toInt();
}

bool Sim800l::hangoffCall()
{
    SIM.print(F("ATH\r\n"));
    _readSerial();
    return (strstr_P(_buffer,PRG_OK)!=NULL);
}

bool Sim800l::sendSms(char *number, char *text)
{
    // set sms to text mode
    SIM.print(F("AT+CMGF=1\r"));
    // eat response
    _readSerial();

    // command to send sms
    SIM.print(F("AT+CMGS=\""));
    SIM.print(number);
    SIM.print(PRG_CRLF);
    // eat response
    _readSerial();

    SIM.print(text);
    SIM.print(F("\r"));
    delay(100);

    // send EOF (^Z)
    SIM.print((char)26);
    // get response
    _readSerial();

    // expect CMGS:xxx   , where xxx is a number,for the sending sms.
    return (strstr(_buffer,"CMGS")!=NULL);
}

// return number of sms
int Sim800l::getNumberSms(uint8_t index)
{
    readSms(index);
    int len=strlen(_buffer);
    Serial.println(len);
    // avoid empty sms
    if (len>10) {
        char* pos1=strstr(_buffer,"+CMGR:");
        if (pos1!=NULL) {
            pos1=strstr(pos1+1,"\",\"");
            if (pos1!=NULL) {
                return atoi(pos1+3);
            }
        }
    }
    // invalid result
    return -1;
}

char* Sim800l::readSms(uint8_t index)
{
    SIM.print(F("AT+CMGF=1\r"));
    _readSerial();
    if (strstr_P(_buffer,PRG_ERR)==NULL) {
        SIM.print(F("AT+CMGR="));
        SIM.print(index);
        SIM.print(F("\r"));
        _readSerial();
        if (strstr(_buffer,"CMGR:")!=NULL) {
            return _buffer;
        }
    }
    // zero terminate zero answer
    _buffer[0]=0;
    return NULL;
}

bool Sim800l::delAllSms()
{
    SIM.print(F("at+cmgda=\"del all\"\r\n"));
    _readSerial();
    return (strstr_P(_buffer,PRG_OK)!=NULL);
}

// returns -1 if error, otherwise 0;
int Sim800l::RTCtime(int *day, int *month, int *year, int *hour, int *minute, int *second)
{
    SIM.print(F("at+cclk?\r\n"));
    _readSerial();
    if (strstr_P(_buffer,PRG_ERR)!=NULL) {
        return -1;
    }
    char* pos1=strstr(_buffer,"\"");
    if (pos1==NULL) {
        return -1;
    }
    // bump past the \"
    pos1++;
    // work backwards, add terminating zero, then pull integer from string
    pos1[18]=0; *second = atoi(pos1+15);
    pos1[15]=0; *minute = atoi(pos1+12);
    pos1[12]=0; *hour   = atoi(pos1+9);
    pos1[9]=0;  *day    = atoi(pos1+6);
    pos1[6]=0;  *month  = atoi(pos1+3);
    pos1[3]=0;  *year   = atoi(pos1);
    // no error
    return 0;
}

// Get the length of the date,time string of the base of GSM
int Sim800l::dateNet()
{
    SIM.print(F("AT+CIPGSMLOC=2,1\r\n"));
    _readSerial();
    char* posEND=strstr_P(_buffer,PRG_OK);
    if (posEND!=NULL) {
        char* posSTART=strstr_P(_buffer,PRG_COLON);
        if (posSTART!=NULL) {
            posSTART+=2;
            posEND-=4;
            int len=posEND-posSTART+1;
            // copy chunk to beginning of buffer
            strncpy(_buffer,posSTART,len);
            _buffer[len]=0;
            return len;
        }
    }
    return 0;
}

#define LEADING_ZERO(a) { if (a<10) { SIM.print(PRG_ZERO); } SIM.print(a); }

// Update the RTC of the module with the date of GSM.
bool Sim800l::updateRtc(int utc)
{
    activateBearerProfile();
    // get current date/time
    dateNet();
    char* dt=strstr_P(_buffer,PRG_COMMA);
    if (dt!=NULL) {
        // bump past comma
        dt++;
        char* tm = strstr_P(_buffer,PRG_COMMA)+1;
        tm[9]=0; uint8_t mSeconds=atoi(tm+6);
        tm[6]=0; uint8_t mMinutes=atoi(tm+3);
        tm[3]=0; uint8_t mHours=atoi(tm);
        dt[9]=0; uint8_t mDay=atoi(dt+7);
        dt[6]=0; uint8_t mMonth=atoi(dt+4);
        dt[3]=0; uint8_t mYear=atoi(dt);
        int8_t utcQuarters=utc*4;

        SIM.print(F("AT+CCLK=\""));

        LEADING_ZERO(mYear);
        SIM.print(PRG_SLASH);
        LEADING_ZERO(mMonth);
        SIM.print(PRG_SLASH);
        LEADING_ZERO(mDay);

        SIM.print(PRG_COMMA);

        LEADING_ZERO(mHours);
        SIM.print(PRG_COLON);
        LEADING_ZERO(mMinutes);
        SIM.print(PRG_COLON);
        LEADING_ZERO(mSeconds);

        if (utcQuarters<0) {
            SIM.print(F("-"));
        } else {
            SIM.print(F("+"));
        }
        // can be negative, handle it
        if (utcQuarters<0) {
            // make it positive
            utcQuarters=-utcQuarters;
        }
        LEADING_ZERO(utcQuarters);

        SIM.print(PRG_CRLF);
    }
    deactivateBearerProfile();
    return (strstr_P(_buffer,PRG_ERR)!=NULL);
}
