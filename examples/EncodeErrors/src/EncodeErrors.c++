#include <Arduino.h>
#include <pdulib.h>

PDU mypdu = PDU(350);

// GSM7
const char *gsm7[] = {
                    "01234567890123456789012345678901234567890123456789012345678901234567890123456789"
                    "01234567890123456789012345678901234567890123456789012345678901234567890123456789"
                    "AB",  // 162 chars
                    "01234567890123456789012345678901234567890123456789012345678901234567890123456789"
                    "01234567890123456789012345678901234567890123456789012345678901234567890123456789", // 160 chars
                    "01234567890" // 10 chars fits in default work buffer
                };

const char *bit16[] = {
                        "אא1234567890א1234567890א1234567890א1234567890א12345678901234567890123456789", // > 70 UCS2
                        "012345678901234567890123456789012345678901234567890123456789012345678א",   // 70 UCS2
                        "0123456א" // fits in default work buffer
                    };

const char *multipart[] = {
                        "multipart part 1",
                        "multipart part 2",
                        "multipart part 3",
                        "multipart part 4"
};

/*
   GSM7 data
*/
void part1(){
  // Uncomment ONE of the following 3 lines
  mypdu.setSCAnumber("+9725400000");   // legal format
  //mypdu.setSCAnumber("-9725400000");   // illegal character
  //mypdu.setSCAnumber("+972540000098765432101");  // too long
  for (int i = 0;i<sizeof(gsm7)/sizeof(const char *);i++) {
      Serial.println(gsm7[i]);
      int len =  mypdu.encodePDU("0541234567",gsm7[i]);
      switch (len) {
          case mypdu.GSM7_TOO_LONG:
              Serial.println("Message too long to send as a single message, change to multipart");
              break;
          case mypdu.WORK_BUFFER_TOO_SMALL:
              Serial.println("Work buffer too small, change PDU constructor");
              break;
          case mypdu.ADDRESS_FORMAT:
                Serial.println("SCA or Target address illegal characters or too long");
              break;
          case mypdu.UCS2_TOO_LONG:
          case mypdu.MULTIPART_NUMBERS:
          case mypdu.ALPHABET_8BIT_NOT_SUPPORTED:
                Serial.println("How did we get here?");
              break;
          default:
                Serial.print("Message length ");  Serial.println(len);
              break;
      }
  }
}

/*
    Multipart errors
*/
void part2() {
  // uncomment ONE of the following 3 lines
  int thispart = 2; // legal
  //int thispart = 0; // illegal, start from 1
  //int thispart = numparts+1; // illegal, < numparts
  int numparts = sizeof(multipart)/sizeof(const char *);
  int ref = 99;
  Serial.println(multipart[1]);
  int len =  mypdu.encodePDU("0541234567",multipart[1],ref,numparts,thispart);
  switch (len) {
      case mypdu.GSM7_TOO_LONG:
        Serial.println("Message too long to send as a single message, change to multipart");
        break;
      case mypdu.WORK_BUFFER_TOO_SMALL:
        Serial.println("Work buffer too small, change PDU constructor");
        break;
      case mypdu.ADDRESS_FORMAT:
        Serial.println("SCA or Target address illegal characters or too long");
        break;
      case mypdu.MULTIPART_NUMBERS:
        Serial.println("Multipart numbers illogical");
        break;
      case mypdu.UCS2_TOO_LONG:
      case mypdu.ALPHABET_8BIT_NOT_SUPPORTED:
            Serial.println("How did we get here?");
          break;
      default:
            Serial.print("Success. Message length ");  Serial.println(len);
          break;
  }
}

/*
    UCS2 data
*/
void part3() {
  // uncomment ONE of the following 3 lines
  mypdu.setSCAnumber("+9725400000");   // legal format
  //mypdu.setSCAnumber("-9725400000");   // illegal character
  //mypdu.setSCAnumber("+972540000098765432101");  // too long
  for (int i = 0;i<sizeof(bit16)/sizeof(const char *);i++) {
      Serial.println(gsm7[i]);
      int len =  mypdu.encodePDU("0541234567",bit16[i]);
      switch (len) {
          case mypdu.UCS2_TOO_LONG:
              Serial.println("Message too long to send as a single message, change to multipart");
              break;
          case mypdu.WORK_BUFFER_TOO_SMALL:
              Serial.println("Work buffer too small, change PDU constructor");
              break;
          case mypdu.ADDRESS_FORMAT:
                Serial.println("SCA or Target address illegal characters or too long");
              break;
          case mypdu.GSM7_TOO_LONG:
          case mypdu.MULTIPART_NUMBERS:
          case mypdu.ALPHABET_8BIT_NOT_SUPPORTED:
                Serial.println("How did we get here?");
              break;
          default:
                Serial.print("Message length ");  Serial.println(len);
              break;
      }
  }
}

void setup() {
  Serial.begin(9600);
#ifdef PM
  Serial.println("Using PM");
#else
  Serial.println("Not using PM");
#endif
  //part1();
  //part2();
  part3();
}

void loop() {
  // put your main code here, to run repeatedly:
}