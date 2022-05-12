#include <iostream>
#include <string>
#include <chrono>
#include <string.h>
#include <unistd.h> // write(), read(), close()
#include <queue>

#include "pdulib.h"
#include "config.h"

extern std::queue<int8_t> GtQueue;

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
  "ABC\015\012New Line",  // 6 GSM 7 bit with CR/LF
  "אָאַאְאּאִאֶאֵאֹאֻאֲאֳאֱאׁאׂאֿ"  // nikkud
};

struct sCM {
  int llist ;
  const char *mlist[] ;
};
sCM concatMessage1 = { 6,{
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ\r\n", // gsm 7
  "0123456789\r\n",  // gsm 7
  "Third line\r\n",  // gsm 7
  "Fourth Line\r\n",  // gsm 7
  "אבגדהוז",  // 16 bit
  "😃"      // surrogate pair
  }};

std::string menu = "Menu\n" "  [01234567] send sms\n   c senbd multipart sms";
void sendSMS(int sp,int i);
void sendConcat(int sp, sCM *);

void consoleHandler(int sp) {
    char linein[10];
    std::cout << "Console handler starting\n";
    while (true) {
        std::cin.getline(linein,sizeof(linein));
        switch (linein[0]) {
            case '0' ... '7':
              sendSMS(sp,linein[0]-'0');
              break;
            case 'c':
              sendConcat(sp,&concatMessage1);
              break;
            default:
                std::cout << menu;
        }
    }
}

extern PDU mypdu;

char writeBuf[50];   // general purpose

void waiton(int n) {
  std::cout << "wait " << n << std::endl;
  while (true) {
    if (!GtQueue.empty()) {
      int response = GtQueue.front();
      GtQueue.pop();
      if (response == n)
        return;
    }
  }  
}
/*
    Send a single message from a list
*/
void sendSMS(int sp, int i) {
  // mypdu.setSCAnumber(SCAnumber);  // already set in unsolicited.cpp
  int len = mypdu.encodePDU(Target,message[i]);
  if (len == -1)
    std::cout << "Message too long\n";
  else {
    sprintf(writeBuf,"AT+CMGS=%d\r\n",len);
    write(sp,writeBuf,strlen(writeBuf));
    // should wait for ">" but just do a delay instead
    waiton(1);
    //sleep(5);
    write(sp,mypdu.getSMS(),strlen(mypdu.getSMS()));
    // wait for +CGMS
    waiton(2);
    //sleep(5);
  }
} 
/*
  Send a multi-part message
*/
int refnum = 30;
void sendConcat(int sp,sCM *pMess){
  refnum++;
  for (int i=0; i<pMess->llist;i++) {
    //mypdu.setSCAnumber(SCAnumber);   // already set in unsolicited.cpp
    //std::cout << pMess->mlist[i] << " " << pMess->llist << " " << i+1 << std::endl;
    int len = mypdu.encodePDU(Target,pMess->mlist[i],refnum,pMess->llist,i+1);
    if (len == -1)
      std::cout << "Message too long\n";
    else {
      sprintf(writeBuf,"AT+CMGS=%d\r\n",len);
      write(sp,writeBuf,strlen(writeBuf));
      // should wait for ">" but just do a delay instead
      //sleep(3);
      waiton(1);
      write(sp,mypdu.getSMS(),strlen(mypdu.getSMS()));
      waiton(2);
    }
  }
}
