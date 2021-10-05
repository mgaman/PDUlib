#include <Arduino.h>
#include <SoftwareSerial.h>
#include <pdulib.h>

SoftwareSerial GSM(2,3);
PDU mypdu=PDU();

#define BUF_SIZE 200
char linebuf[BUF_SIZE];
int inCount = 0;
bool nextLinePDU = false;

void setup() {
  Serial.begin(9600);
  GSM.begin(9600);
  Serial.println("Send me text messages.. now  ");
}

void processLine() {
//  Serial.print(linebuf);
  if (strncmp("+CMT:",linebuf,5) == 0) {
    nextLinePDU = true;
  }
  else if (nextLinePDU) {
    if (mypdu.decodePDU(linebuf)) {
      Serial.print("From: "); Serial.println(mypdu.getSender());
      Serial.print("Msg: "); Serial.println(mypdu.getText());
    }
    nextLinePDU = false;
  }
  inCount = 0;  
}
void loop() {
  while (GSM.available()) {
    byte b = GSM.read();
    linebuf[inCount++] = b;
    if (inCount == BUF_SIZE) {
      Serial.println("buffer overflow");
      inCount = 0;
    }
    if (b == 0x0A) {   // end of line
      processLine();
    }
  }
}