#include <Arduino.h>
#include <SoftwareSerial.h>
#include <pdulib.h>

SoftwareSerial GSM(2,3);  // UNO
//SoftwareSerial GSM(10,11);  // Mega2560

PDU mypdu=PDU();

#define BUF_SIZE 340 // at least 340 for a full length message
char linebuf[BUF_SIZE];
int inCount = 0;
bool nextLinePDU = false;

void setup() {
  Serial.begin(9600);
#ifdef PM
  Serial.println("Using PM");
#else
  Serial.println("Not using PM");
#endif

  GSM.begin(9600);
  Serial.println("Send me text messages.. now  ");
}

void processLine() {
  //Serial.print(linebuf);
  if (nextLinePDU) {
    if (mypdu.decodePDU(linebuf)) {
      Serial.print("From: "); Serial.println(mypdu.getSender());
      Serial.print("Timestamp: "); Serial.println(mypdu.getTimeStamp());
      Serial.print("Msg: "); Serial.println(mypdu.getText());
      // is this standalone or multi-part ?
      int *concat = mypdu.getConcatInfo();
      if (concat[0] == 0)
        Serial.println("This is a standalone message");
      else {
        sprintf(linebuf,"Mulitpart %d of %d ref %d",concat[1],concat[2],concat[0]);
        Serial.println(linebuf);
      }
    }
    nextLinePDU = false;
  }
  else if (strncmp("+CMT:",linebuf,5) == 0) {
    nextLinePDU = true;
  }
  inCount = 0;  
}
void loop() {
  while (GSM.available()) {
    byte b = GSM.read();
    //Serial.print(b,HEX);
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