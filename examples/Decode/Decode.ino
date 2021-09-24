#include <Arduino.h>
#include "pdulib.h"

PDU mypdu = PDU();
/*
  When a GSM modem is in PDU mode, an incoming SMS is displayed as
  +CMT: nn
  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

  inpdu below is an example of xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
*/
const char *inpdu = "07917952140230F2040C9179525419896800001280018153832106D17B594ECF03";
void setup() {
  Serial.begin(9600);
  mypdu.decodePDU(inpdu);
  Serial.println(mypdu.getSender());
  Serial.println(mypdu.getTimeStamp());
  Serial.println(mypdu.getText());
}

void loop() {
}