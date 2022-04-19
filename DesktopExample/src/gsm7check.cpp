#include <iostream>
#include <iomanip>
using std::hex;
using std::setw;

#include <pdulib.h>

const char *charsets[] = {
  { "A😀🐐B" } , // surrogate pair   0x1f600 0x1f410
  {
  "@£$¥èéùìòÇ Øø åÅ"
  "Δ_ΦΓΛΩΠΨΣΘΞ ÆæßÉ"
  " !\"#¤%&'()*+,-./"
  "0123456789:;<=>?"
  } ,
  {
  "¡ABCDEFGHIJKLMNO"
  "PQRSTUVWXYZÄÖÑÜ§"
  "¿abcdefghijklmno"
  "pqrstuvwxyzäöñüà"
  "\nNew Line"
  } ,
  { "^{}[]\\~|€"},   // escaped symbols }
  { "àAB€Δ" },  // contains escaped,Greek and non-gsm7
  {
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
    "X" },  // GSM 7 message > 160 septets
  {
    "אבגדהוזחטיאבגדהוזחטיאבגדהוזחטיאבגדהוזחטיאבגדהוזחטיאבגדהוזחטיאבגדהוזחטי"
    "אבגדהוזחטיאבגדהוזחטיאבגדהוזחטיאבגדהוזחטיאבגדהוזחטיאבגדהוזחטיאבגדהוזחטי"
    "a" }, // 16 bit message > 70 octets
};

const char *scanumber = "+9725400000";
const char *target = "+9725451111111";

char ucs2[512];

void gsm7check(PDU mypdu) {
  // scan gsm7 UTF8 string and print out the UCS2 values
  for (unsigned int j=0; j<sizeof(charsets)/sizeof(const char *); j++) {
    int numoctets = mypdu.utf8_to_ucs2(charsets[j],ucs2);
    std::cout << charsets[j] << " Found " << numoctets << " octets" << std::endl;
    unsigned char *source = (unsigned char *) ucs2;
    unsigned short target;
    for (int i=0; i<numoctets/2;i++) {
      // ucs2 is bigendian, swap to little endian
      target = (*source << 8) | *(source+1);
      std::cout << setw(4) << target << " " << setw(4) << i << std::endl;
      source += 2; 
    }
  }
}

bool isAllGSM7(const char *utf8, PDU mypdu) {
  bool gsm7bit = true;
  while (*utf8 && gsm7bit) {
    unsigned short ucs2[2],target;
    int length = mypdu.utf8Length(utf8);
    mypdu.utf8_to_ucs2_single(utf8, ucs2);  // translate to a single ucs2
    // ucs2 is bigendian, swap to little endian
    target = (ucs2[0] << 8) | ((ucs2[0] & 0xff00)>> 8);
    gsm7bit = mypdu.isGSM7(&target);  // surrogate pair header will be flagged not GSM7
    utf8 += length;  // bump to next
  }
  return gsm7bit;
}

void gsm7encode(PDU mypdu) {
  // check if this message is all default GSM 7
  for (unsigned int j = 0; j< sizeof(charsets)/sizeof(const char *);j++) {
    std::cout << charsets[j] << std::endl;
    if (isAllGSM7(charsets[j],mypdu))
      std::cout << "Message is all GSM7\n";
    else
      std::cout << "Message is not all GSM7\n";

    mypdu.setSCAnumber(scanumber);
    int len = mypdu.encodePDU(target,charsets[j]);
    if (len == -1)
      std::cout << "Message too long\n";
    else
      std::cout << mypdu.getSMS() << std::endl;
  }
}

