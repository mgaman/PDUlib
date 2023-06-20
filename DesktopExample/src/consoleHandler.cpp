#include <iostream>
#include <string>
#include <string.h>

#include "pdulib.h"
//#include "credentials.h"
#include "Utilities.h"
static const char *Target = "0545919886";

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
  "אָאַאְאּאִאֶאֵאֹאֻאֲאֳאֱאׁאׂאֿ",  // 7 nikkud  each char is surrogate pair i.e. 4 bytes will overflow
  "אָאַאְאּאֵ"  // 8 nikkud  each char is surrogate pair i.e. 4 bytes
};

const char *concatMessage[] = { 
  "ABCDEFGHIJKLMNOPQRSTUVW\r\n", // gsm 7
  "0123456789\r\n",  // gsm 7
  "Third line\r\n",  // gsm 7
  "Fourth Line\r\n",  // gsm 7
  "אבגדהוז",  // 16 bit
  "😃"      // surrogate pair
};

std::string menu = "Menu\n" "  [012345678] send sms\n   c send multipart sms\n\n";
int cms = 10;

void consoleHandler(int sp,PDU mypdu) {
  char linein[10];
  int numMessages = sizeof(concatMessage)/sizeof(const char *);
  std::cout << "Console handler starting\n";
  while (true) {
    std::cin.getline(linein,sizeof(linein));
    switch (linein[0]) {
      case '0' ... '8':
        sendSMS(mypdu,Target,message[linein[0]-'0']);
        break;
      case 'c':
        for (int j=0;j<numMessages;j++)
          sendSMS(mypdu,Target,concatMessage[j],cms,numMessages,j+1);
        cms++;
        break;
      default:
        std::cout << menu;
        break;
    }
  }
}
