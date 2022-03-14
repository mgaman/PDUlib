#include <iostream>
#include <iomanip>
using std::hex;
using std::setw;

#include <pdulib.h>

const char *gsm7 =
#if 0
"@£$¥èéùìòÇ Øø åÅ"
"Δ_ΦΓΛΩΠΨΣΘΞ ÆæßÉ"
" !\"#¤%&'()*+,-./"
"0123456789:;<=>?"
"¡ABCDEFGHIJKLMNO"
"PQRSTUVWXYZÄÖÑÜ§"
"¿abcdefghijklmno"
"pqrstuvwxyzäöñüà"
"^{}[]\\~|€"   // escaped symbols 
//"א";  // not GSM7
;
#elif 0
  "àAB€Δ"  // contains escaped,Greek and non-gsm7
#elif 0
"^{}[]\\~|€"   // escaped
#elif 0
  "Øø"
#elif 0
  "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
  "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
  "X"  // GSM 7 message > 160 septets
#else
  "אבגדהוזחטיאבגדהוזחטיאבגדהוזחטיאבגדהוזחטיאבגדהוזחטיאבגדהוזחטיאבגדהוזחטי"
  "אבגדהוזחטיאבגדהוזחטיאבגדהוזחטיאבגדהוזחטיאבגדהוזחטיאבגדהוזחטיאבגדהוזחטי"
  "a" // 16 bit message > 70 octets
#endif
;

const char *scanumber = "+9725400000";
const char *target = "+9725451111111";

char ucs2[512];

void gsm7check(PDU mypdu) {
  // scan gsm7 UTF8 string and print out the UCS2 values
  int numoctets = mypdu.utf8_to_ucs2(gsm7,ucs2);
  std::cout << "Found " << numoctets << " octets" << std::endl;
  unsigned char *source = (unsigned char *) ucs2;
  unsigned short target;
  for (int i=0; i<numoctets/2;i++) {
    // ucs2 is bigendian, swap to little endian
    target = (*source << 8) | *(source+1);
//    std::cout << setw(4) << i << " " << setw(4) << target << std::endl;
    std::cout << setw(4) << target << " " << setw(4) << i << std::endl;
//    std::cout << setw(4) << target << std::endl;
    if (!mypdu.isGSM7(target)){
      std::cout << target << " not GSM7\n";
    }
    source += 2; 
  }
}
void gsm7encode(PDU mypdu) {
  mypdu.setSCAnumber(scanumber);
  int len = mypdu.encodePDU(target,gsm7);
  if (len == -1)
    std::cout << "Message too long\n";
  else
    std::cout << mypdu.getSMS() << std::endl;
}