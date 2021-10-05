#include <Arduino.h>
#include <SoftwareSerial.h>
#include "pdulib.h"

PDU mypdu = PDU();
SoftwareSerial GSM(2,3);

// recipient phone number examples
const char *tonat = "************";   // national number
// message examples
const char *allascii = "Hello";   // all 7 bit ascii
const char *notascii = "שלום";      // CSU-2 16 bit
char final[40];  // final message
char temp[20];   // temporary work area

#define brokenheart 0x1f494

void setup() {
  Serial.begin(9600);
  GSM.begin(9600);
  GSM.print("AT+CSCA?\r");   // will print your networks SCA number, put it into the next line
  delay(100);
  GSM.print(F("AT+CMGF=0\r"));   // ensure we are in PDU mode
  delay(100);
  mypdu.setSCAnumber( "***********");
  // start building a message
  strcpy(final,allascii);
  mypdu.buildUtf16(brokenheart,temp);  // construct string for emoji
  strcat(final,temp);           // add to message
  strcat(final,notascii);       // complete the message
  int len = mypdu.encodePDU(tonat,final); 
  if (len > 1) {
    sprintf(temp,"AT+CMGS=%d\r",len);  // generate SMS submit command
      // next 3 lines not needed but nice to know
    Serial.print(F("SMS length "));Serial.println(len);
    Serial.println(mypdu.getSMS());
    Serial.println(temp);
    GSM.print(temp);               // issue SMS submit command
    delay(1000);                   // wait for > prompt to pass
    GSM.print(mypdu.getSMS());     // send the message    
  }
  else
    Serial.println("???");
}

void loop() {
  if (GSM.available())
    Serial.write(GSM.read());
}