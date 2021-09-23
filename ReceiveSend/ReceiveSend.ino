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
//  const char *sca = mypdu.getSCAnumber();
  const char *sca = "+97254120032";
  Serial.println();
  mypdu.setSCAnumber(sca);
  Serial.println(mypdu.getSender());
  Serial.println(mypdu.getTimeStamp());
  Serial.println(mypdu.getText());
  //mypdu.setDebug(true);
  // write your message and recipient number here
//  int len = mypdu.encodePDU("0545919886","Hello",ALPHABET_7BIT);
//  int len = mypdu.encodePDU("+972545919886","Hello",ALPHABET_7BIT);
//  int len = mypdu.encodePDU("+972545919886","שלום",ALPHABET_16BIT);
//  int len = mypdu.encodePDU("0545919886","שלום",ALPHABET_16BIT);
  int len = mypdu.encodePDU("0545919886","אabcdefghijklmnopqrstuvwxyz");
 // int len = mypdu.encodePDU("0545919886","אabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQERSTUVWXYZ01234567890.,;'/[]",ALPHABET_16BIT);
  Serial.print(F("SMS length "));Serial.println(len);
  Serial.println(mypdu.getSMS());
  sprintf(temp,"AT+CMGS=%d\r",len);
  Serial.println(temp);
  GSM.print(F("AT+CMGF=0\r"));
  delay(1000);
  GSM.print(temp);
  delay(1000);
  GSM.print(mypdu.getSMS());
}

void loop() {
  if (GSM.available())
    Serial.write(GSM.read());
}