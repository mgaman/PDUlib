#include <iostream>
#include <string>
#include <chrono>
#include <string.h>
#include <unistd.h> // write(), read(), close()

#include "pdulib.h"
#include "config.h"

std::string menu = "Menu\n" "  [012345] send sms\n";
void sendSMS(int sp,int i);
void consoleHandler(int sp) {
    char linein[10];
    std::cout << "Console handler starting\n";
    while (true) {
        std::cin.getline(linein,sizeof(linein));
        switch (linein[0]) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
                sendSMS(sp,linein[0]-'0');
                break;
            default:
                std::cout << menu;
        }
    }
}

extern PDU mypdu;

const char *message[] = {
  "0123456789ABCDEFGHIJ0123456789ABCDEFGHIJ0123456789ABCDEFGHIJאבגד",  // GSM 16 bit
  "abcd🍖😃אבגד",  // surrogate pairs
  "@£$¥èéùìòÇ Øø åÅ"  // gsm 7 BIT
  "Δ_ΦΓΛΩΠΨΣΘΞ ÆæßÉ"
  " !\"#¤%&'()*+,-./"
  "0123456789:;<=>?"
  "¡ABCDEFGHIJKLMNO"
  "PQRSTUVWXYZÄÖÑÜ§"
  "¿abcdefghijklmno"
  "pqrstuvwxyzäöñüà"
  "^{}[]\\~|€",    // GSM 7 bit escaped
  "{}",
  "12345678",   // 8 gsm 7 bit
  "1234567",   // 7 gsm 7 bit
};

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
