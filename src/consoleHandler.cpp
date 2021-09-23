#include <iostream>
#include <string>
#include <chrono>
#include <string.h>
#include <unistd.h> // write(), read(), close()

#include "pdulib.h"

std::string menu = "Menu\n" "  s send sms\n";
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
            default:
                std::cout << menu;
        }
    }
}

extern PDU mypdu;

const char *to = "+972545919886";
const char *message[] = {
  "שלום012345678901234567890123456789abcdefABCDEFGHIJ0123456789ABCDEFGHIJ",
  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234" };

char writeBuf[50];   // general purpose
void sendSMS(int sp, int i) {
  int len = mypdu.encodePDU(to,message[i]);
  int buflen = strlen(mypdu.getSMS());
  sprintf(writeBuf,"AT+CMGS=%d\r\n",len);
  write(sp,writeBuf,strlen(writeBuf));
    // should wait for ">" but just do a delay instead
  sleep(1);
  write(sp,mypdu.getSMS(),buflen);
  //write(sp,&end,1);
} 
