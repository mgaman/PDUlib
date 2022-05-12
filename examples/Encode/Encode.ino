#include <Arduino.h>
#include <SoftwareSerial.h>
#include <pdulib.h>
#include "credentials.h"
/*******************************
 * 
 * Update credentials.h beore running this code
 * 
 ******************************/
SoftwareSerial GSM(2,3);
PDU mypdu = PDU();
char temp[30];

#define ZERO 48    // 7 bit
#define YEN 165    // 16 bit
#define ALEPH 0x5d0
#define SPADE 0x2660
#define ACE_OF_SPADES 0x1f0a1  // surrogate
#define SMILEY 0x1f600
#define POUND 163  // GSM 7 bit
#define LEFT_SQUARE  91 // GSM escape
#define RIGHT_SQUARE  93 // GSM escape
#define EURO 0x20AC  // gsm escape

//#define DO_ALL_GSM7  // Either print 7 bit alphabet characters or 16 bit alphabet characters
//#define PART0      // GSM7 example too large for UNO so split into 2 parts

#ifdef DO_ALL_GSM7

const char *final =
#ifdef PM
    "PM"
#else
    "NoPM"
#endif
#ifdef PART0
  "@£$¥èéùìòÇ Øø åÅ"
  "Δ_ΦΓΛΩΠΨΣΘΞ ÆæßÉ"
  " !\"#¤%&'()*+,-./"
  "0123456789:;<=>?"
  "\015\012New Line"   // cr/lf
#else
  "¡ABCDEFGHIJKLMNO"
  "PQRSTUVWXYZÄÖÑÜ§"
  "¿abcdefghijklmno"
  "pqrstuvwxyzäöñüà"
  "^{}[]\\~|€"   // escaped symbols
#endif
;
#else
char final[70];
#endif

bool runOnce = true;
bool gotGT = false;

void setup() {
  Serial.begin(9600);
  GSM.begin(9600);
#ifdef PM
  Serial.println("Using PM");
#else
  Serial.println("Not using PM");
#endif
}

void loop() {
  if (runOnce) {
    //GSM.println("ATE0");   // turn off echo on modem
    runOnce = false;
    mypdu.setSCAnumber(SCAnumber);
#ifndef DO_ALL_GSM7
    strcpy(final,"hello");
    mypdu.buildUtf(ZERO,temp);
    strcat(final,temp);
    mypdu.buildUtf(YEN,temp);
    strcat(final,temp);
    mypdu.buildUtf(ALEPH,temp);
    strcat(final,temp);
    mypdu.buildUtf(SPADE,temp);
    strcat(final,temp);
    mypdu.buildUtf(ACE_OF_SPADES,temp);
    strcat(final,temp);
    mypdu.buildUtf(SMILEY,temp);
    strcat(final,temp);
    mypdu.buildUtf(POUND,temp);
    strcat(final,temp);
    mypdu.buildUtf(LEFT_SQUARE,temp);
    strcat(final,temp);
    mypdu.buildUtf(RIGHT_SQUARE,temp);
    strcat(final,temp);
    mypdu.buildUtf(EURO,temp);
    strcat(final,temp);
    strcat(final,"שלום");
#endif
    int len = mypdu.encodePDU(Target,final);
    if (len == -1) {
      Serial.println("Message too long");
    }
    else {
      sprintf(temp,"AT+CMGS=%d\r",len);
      Serial.print("SMS length ");Serial.println(len);
      //Serial.println(mypdu.getSMS());
      GSM.print(temp);
    }
  }
  if (gotGT) {
    Serial.println("Send it");
    GSM.print(mypdu.getSMS());
  }
  while (GSM.available()) {
    int c = GSM.read();
    Serial.write((char)c);
    gotGT = c == '>';  // let me know when I can send the payload
  }
  while (Serial.available()) {
    GSM.write(Serial.read());
  }
}
