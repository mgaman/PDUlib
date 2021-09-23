#include <Arduino.h>
#include <SoftwareSerial.h>
#include "pdulib.h"

#define POWERUP_PIN 8      // for SIM900 shield
#define MODEM_BAUD 9600

PDU mypdu = PDU();
SoftwareSerial GSM(2,3);

void hwreset() {
	pinMode(POWERUP_PIN,OUTPUT);
	digitalWrite(POWERUP_PIN,LOW);
	delay(1000);
	digitalWrite(POWERUP_PIN,HIGH);
	delay(2500);
	digitalWrite(POWERUP_PIN,LOW);
	delay(3500);
}

void setup() {
  char temp[20];
//  hwreset();
//  delay(10000);
  Serial.begin(115200);
  GSM.begin(9600);
  mypdu.decodePDU("07917952140230F2040C9179525419896800001280018153832106D17B594ECF03");
  Serial.println();
  Serial.println(mypdu.getSender());
  Serial.println(mypdu.getTimeStamp());
  Serial.println(mypdu.getText());
  GSM.print("AT+CSCA?\r");   // will print your networks SCA number, put it into the next line
  mypdu.setSCAnumber( "+***********");
  // write your message and recipient number here
//  int len = mypdu.encodePDU("**********","Hello");   // all ascii, national number
//  int len = mypdu.encodePDU("+**********","Hello");  // all ascii, international number
//  int len = mypdu.encodePDU("**********","שלום");    // 16 bit character, national number
  int len = mypdu.encodePDU("+************","שלום");     // 16 bit character, international number
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