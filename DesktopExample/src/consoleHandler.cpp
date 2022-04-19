#include <iostream>
#include <string>
#include <chrono>
#include <string.h>
#include <unistd.h> // write(), read(), close()

#include "pdulib.h"
#include "config.h"

const char *message[] = {
  "0123456789ABCDEFGHIJ0123456789ABCDEFGHIJ0123456789ABCDEFGHIJאבגד",  // 0 GSM 16 bit
  "abcd🍖😃אבגד",  // 1 surrogate pairs
  "@£$¥èéùìòÇ Øø åÅ"  // 2 gsm 7 BIT
  "Δ_ΦΓΛΩΠΨΣΘΞ ÆæßÉ"
  " !\"#¤%&'()*+,-./"
  "0123456789:;<=>?"
  "¡ABCDEFGHIJKLMNO"
  "PQRSTUVWXYZÄÖÑÜ§"
  "¿abcdefghijklmno"
  "pqrstuvwxyzäöñüà",
  "^{}[]\\~|€",    // 3 GSM 7 bit escaped
  "12345678",   // 4 gsm 7 bit 8 characters
  "1234567",   // 5 7 gsm 7 bit 7 character
  "ABC\015\012New Line"  // 6 GSM 7 bit with CR/LF
};

std::string menu = "Menu\n" "  [0123456] send sms\n";
void sendSMS(int sp,int i);
void consoleHandler(int sp) {
    char linein[10];
    std::cout << "Console handler starting\n";
    while (true) {
        std::cin.getline(linein,sizeof(linein));
        switch (linein[0]-'0') {
//            case 0 ... (sizeof(message)/sizeof(const char *))-1:
            case 0 ... 6:
              sendSMS(sp,linein[0]-'0');
              break;
            default:
                std::cout << menu;
        }
    }
}

extern PDU mypdu;

char writeBuf[50];   // general purpose
void sendSMS(int sp, int i) {
  mypdu.setSCAnumber(SCAnumber);
  int len = mypdu.encodePDU(Target,message[i]);
  if (len == -1)
    std::cout << "Message too long\n";
  else {
    sprintf(writeBuf,"AT+CMGS=%d\r\n",len);
    write(sp,writeBuf,strlen(writeBuf));
    // should wait for ">" but just do a delay instead
    sleep(1);
    write(sp,mypdu.getSMS(),strlen(mypdu.getSMS()));
  }
} 
