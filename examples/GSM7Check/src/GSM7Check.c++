#include <Arduino.h>
#include <pdulib.h>

char *gsm7 =
"@£$¥èéùìòÇ Øø åÅ"
"Δ_ΦΓΛΩΠΨΣΘΞ ÆæßÉ"
" !\"#¤%&'()*+,-./"
"0123456789:;<=>?"
"¡ABCDEFGHIJKLMNO"
"PQRSTUVWXYZÄÖÑÜ§"
"¿abcdefghijklmno"
"pqrstuvwxyzäöñüà"
"^{}[]\\~|€"   // escaped symbols 
//"א"  // not gsm7
;

PDU mypdu;

bool justOnce = true;
char ucs2[600]; // make sure its big enough
char printbuf[50];

void setup() {
  Serial.begin(9600);
}

void loop() {
  if (justOnce) {
#ifdef PM
    Serial.println("Using PM");
#else
    Serial.println("Not using PM");
#endif
    justOnce = false;
    int numoctets = mypdu.utf8_to_ucs2(gsm7,ucs2);
    sprintf(printbuf,"Found %d octets",numoctets);
    Serial.println(printbuf);
    unsigned char *source = (unsigned char *) ucs2;
    unsigned short target;
    for (int i=0; i<numoctets/2;i++) {
      // ucs2 is bigendian, swap to little endian
      target = (*source << 8) | *(source+1);
  //    std::cout << setw(4) << i << " " << setw(4) << target << std::endl;
      sprintf(printbuf,"%4d %4d",target,i);
      Serial.println(printbuf);
  //    std::cout << setw(4) << target << std::endl;
      if (!mypdu.isGSM7(target)){
        sprintf(printbuf," %d not GSM7",target);
        Serial.println(printbuf);
      }
      source += 2; 
    }
  }
}