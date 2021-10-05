#include <iostream>
#include <string>
#include <chrono>
#include <string.h>
#include <unistd.h> // write(), read(), close()

#include "pdulib.h"

std::string menu = "Menu\n" "  [sStT] send sms\n";
void sendSMS(int sp,int i);
void consoleHandler(int sp) {
    char linein[10];
    std::cout << "Console handler starting\n";
    while (true) {
        std::cin.getline(linein,sizeof(linein));
        switch (linein[0]) {
            case 's':
                sendSMS(sp,0);
                break;
            case 'S':
                sendSMS(sp,1);
                break;
            case 't':
                sendSMS(sp,2);
                break;
            case 'T':
                sendSMS(sp,3);
                break;
            default:
                std::cout << menu;
        }
    }
}

extern PDU mypdu;

const char *to = "**********";   // place destination phonr number here
const char *sca = "*********";   // place your SCA number here
const char *message[] = {
  "שלום012345678901234567890123456789abcdefABCDEFGHIJ0123456789ABCDEFGHIJ",  // GSM 16 bit
  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234", // all ascii
  "J£mjòø",   // GSM 7 bit 
  "abcd🍖😃אבגד"  // surrogate pairs
  };

char writeBuf[50];   // general purpose
void sendSMS(int sp, int i) {
  mypdu.setSCAnumber(sca);
  int len = mypdu.encodePDU(to,message[i]);
  sprintf(writeBuf,"AT+CMGS=%d\r\n",len);
  write(sp,writeBuf,strlen(writeBuf));
    // should wait for ">" but just do a delay instead
  sleep(1);
  write(sp,mypdu.getSMS(),strlen(mypdu.getSMS()));
} 
