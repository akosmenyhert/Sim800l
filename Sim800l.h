/* this library is writing by  Cristian Steib.
 *      steibkhriz@gmail.com
 *  Designed to work with the GSM Sim800l, maybe work with SIM900L
 *
 *   This library use SoftwareSerial, you can define RX and TX pin in the
 *   header "Sim800l.h" ,by default the pin is RX=10 TX=11.. be sure that gnd
 *   is attached to arduino too.
 *   You can also change the other preferred RESET_PIN
 *
 *   Esta libreria usa SoftwareSerial, se pueden cambiar los pines de RX y TX
 *   en el archivo header, "Sim800l.h", por defecto los pines vienen
 *   configurado en RX=10 TX=11.
 *   Tambien se puede cambiar el RESET_PIN por otro que prefiera
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

#ifndef Sim800l_h

#define Sim800l_h
#include <SoftwareSerial.h>
#include "Arduino.h"

#define RX_PIN    10
#define TX_PIN    11
// pin to the reset pin sim800l
#define RESET_PIN 2

// used for indicator led, in case that you don want set to false .
#define LED     true
//pin to indicate states.
#define LED_PIN 13

class Sim800l {
  private:
    int    _timeout;
    String _buffer;
    String _readSerial();

  public:
    void begin();
    void reset();

    // Methods for calling || Funciones de llamadas.
    bool    answerCall();
    void    callNumber(char *number);
    bool    hangoffCall();
    uint8_t getCallStatus();

    // Methods for sms || Funciones de SMS.
    bool sendSms(char *number, char *text);
    // return all the content of sms
    String readSms(uint8_t index);
    // return the number of the sms..
    String getNumberSms(uint8_t index);
    // return :  OK or ERROR ..
    bool delAllSms();

    void signalQuality();
    void setPhoneFunctionality();
    void activateBearerProfile();
    void deactivateBearerProfile();

    // get time with the variables by reference
    void RTCtime(int *day, int *month, int *year, int *hour, int *minute, int *second);
    // return date,time, of the network
    String dateNet();
    // Update the RTC Clock with de Time AND Date of red-.
    bool updateRtc(int utc);
};

#endif
