#include <Arduino.h>
#include <SoftwareSerial.h>
#include <pdulib.h>

SoftwareSerial GSM(2,3);
PDU mypdu = PDU();
char temp[30];
char final[50];

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
void setup() {
  Serial.begin(9600);
  GSM.begin(9600);
  mypdu.setSCAnumber("+97254120032");
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
  int len = mypdu.encodePDU("0545919886",final);
  sprintf(temp,"AT+CMGS=%d\r",len);
  Serial.print("SMS length ");Serial.println(len);
  Serial.println(mypdu.getSMS());
  GSM.print(temp);
  delay(1000);
  GSM.print(mypdu.getSMS());
}

void loop() {
  while (GSM.available())
    Serial.write(GSM.read());
}
