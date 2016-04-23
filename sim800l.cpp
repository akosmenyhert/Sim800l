#include "Arduino.h"
#include "sim800l.h"
#include <SoftwareSerial.h>

SoftwareSerial SIM(10,11);
long int timeout;
String buffer;


void sim800l::begin(){
	SIM.begin(9600);

}

String readSerial(){
  
  
  timeout=0;
  while  (!SIM.available() && timeout < 900000  ) 
  {
    timeout++;
  }
  if (SIM.available()) {
 	return SIM.readString();
  }
}

bool recvFind(String target,String Data){
	if (Data.indexOf(target) != -1) 
  {
    return true;
  }
  return false;
}

void sim800l::activateBearerProfile(){
  // set bearer parameter 
  SIM.print (F(" AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\" \r\n" ));delay(1000);
  SIM.print (F(" AT+SAPBR=3,1,\"APN\",\"internet\" \r\n" ));delay(1000);  // set apn  
  SIM.print (F(" AT+SAPBR=1,1 \r\n")); delay(1000);// activate bearer context
  SIM.print (F(" AT+SAPBR=2,1\r\n ")); delay(1000);// get context ip address
}

bool sim800l::sendSms(char* number,char* text){
 
 
    SIM.print (F("AT+CMGF=1\r"));  delay(1000);
    SIM.print (F("AT+CMGS=\""));
    SIM.print (number);
    SIM.print(F("\"\r"));       delay(1200);
    SIM.print (text);
    SIM.print ("\r"); 
    delay(1000);
    SIM.print((char)26);
    delay(1000);
    buffer=readSerial();
    while (recvFind("+CMTI:",buffer)  || recvFind("RING",buffer)  ){
    	buffer=readSerial();
    }
    if (recvFind("CMGS",buffer)){
      return true;
    }
    else {
      return false;
    }
}



// PRIVATE METHODS
