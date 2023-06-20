
#include <iostream>
//#include <thread>
#include <queue>
//#include <string>

// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
//#include <fcntl.h> // Contains file controls like O_RDWR
//#include <errno.h> // Error integer and strerror() function
//#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

#include <pdulib.h>
#include "Utilities.h"

extern int serial_port;
extern std::queue<std::string> inputQueue;
extern std::queue<int8_t> GtQueue; // to let us know when a single > arrives

static char writeBuf[50];  // work area

void sendSMS(PDU mypdu,const char *target,const char *message,unsigned short cms,unsigned char numparts,unsigned char partnum)
{
  // mypdu.setSCAnumber(SCAnumber);  // already set in unsolicited.cpp
  int len = mypdu.encodePDU(target, message,cms,numparts,partnum);
  if (len < 0) {
    std::cout << "Error " << len << std::endl;
    switch (len) {
      case PDU::OBSOLETE_ERROR:
        break;
      case PDU::UCS2_TOO_LONG:
        std::cout << "UCS2 too long\n";
        break;
      case PDU::GSM7_TOO_LONG:
        std::cout << "GSM7 too long\n";
        break;
      case PDU::MULTIPART_NUMBERS:
        std::cout << "UCS2 check multipart parameters\n";
        break;
      case PDU::ADDRESS_FORMAT:
        std::cout << target << " format\n";
        break;
      case PDU::WORK_BUFFER_TOO_SMALL:
        std::cout << "work buffer too small\n";
        break;
      case PDU::ALPHABET_8BIT_NOT_SUPPORTED:
        break;
    }
  }
  else
  {
    std::cout <<"PDU length " << len << std::endl;
    sprintf(writeBuf, "AT+CMGS=%d\r\n", len);
    write(serial_port, writeBuf, strlen(writeBuf));
    // should wait for ">" but just do a delay instead
    waiton(1);
    write(serial_port, mypdu.getSMS(), strlen(mypdu.getSMS()));
    std::cout << mypdu.getSMS() << std::endl;
    // wait for +CGMS
    waiton(2);
  }
}

void sendSMS(PDU mypdu,const char *Target,const char *message) {
  sendSMS(mypdu,Target,message,0,0,0);   // -1 changed later to 0 to appear as single message
}

// cutdown version of unsolicited 
void getpdu(char * save) {
  bool nextLineSMS = false;
  std::cout << "getpdu started\n";
  while (true) {
    if (!inputQueue.empty()) {
      std::string response = inputQueue.front();
      inputQueue.pop();
      std::cout << response << std::endl;
      if (response.compare(0,5,"+CMT:")== 0)
        nextLineSMS = true;
      else if (response.length() > 2 &&  nextLineSMS) {
          std::cout << response.c_str() << std::endl;
          std::cout << "Length " << strlen(response.c_str()) << std::endl;
          strcpy(save,response.c_str());
          return;
      }
    }
  }
}

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
