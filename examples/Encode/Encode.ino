#include <Arduino.h>
#include <SoftwareSerial.h>
#include "pdulib.h"

PDU mypdu = PDU();
SoftwareSerial GSM(2,3);

// recipient phone number examples
const char *tonat = "*********";   // national number
const char *tointl = "+*********"; // international
// message examples
const char *allascii = "Hello";   // all 7 bit ascii
const char *notasci = "שלום";      // CSU-2 16 bit

void setup() {
  char temp[20];
  Serial.begin(9600);
  GSM.begin(9600);
  GSM.print("AT+CSCA?\r");   // will print your networks SCA number, put it into the next line
  mypdu.setSCAnumber( "+***********");
  int len = mypdu.encodePDU(tonat,allascii); 
  Serial.print(F("SMS length "));Serial.println(len);
  Serial.println(mypdu.getSMS());
  sprintf(temp,"AT+CMGS=%d\r",len);  // generate SMS submit command
  Serial.println(temp);
  GSM.print(F("AT+CMGF=0\r"));   // ensure we are in PDU mode
  delay(1000);
  GSM.print(temp);               // issue SMS submit command
  delay(1000);                   // wait for > prompt to pass
  GSM.print(mypdu.getSMS());     // send the message
}

void loop() {
  if (GSM.available())
    Serial.write(GSM.read());
}