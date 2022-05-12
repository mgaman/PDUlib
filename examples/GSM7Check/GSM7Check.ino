#include <Arduino.h>
#include <pdulib.h>

//#define PART0    // uncomment to do first half, comment to do second half

char *gsm7 =
#ifdef PART0
"@£$¥èéùìòÇ Øø åÅ"
"Δ_ΦΓΛΩΠΨΣΘΞ ÆæßÉ"
" !\"#¤%&'()*+,-./"
"0123456789:;<=>?"
"¡ABCDE"
#else
"FGHIJKLMNO"
"PQRSTUVWXYZÄÖÑÜ§"
"¿abcdefghijklmno"
"pqrstuvwxyzäöñüà"
"^{}[]\\~|€"   // escaped symbols 
"\r\n"     // carriage return/line feed
"א"  // not gsm7
#endif
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
      sprintf(printbuf,"%4d %4d",target,i);
      Serial.println(printbuf);
      if (!mypdu.isGSM7(&target)){
        sprintf(printbuf," %d not GSM7",target);
        Serial.println(printbuf);
      }
      source += 2; 
    }
  }
}