#include <iostream>
#include <iomanip>
using std::hex;
using std::setw;

#include <pdulib.h>

char *gsm7 =
#if 1
"@£$¥èéùìòÇ Øø åÅ"
"Δ_ΦΓΛΩΠΨΣΘΞ ÆæßÉ"
" !\"#¤%&'()*+,-./"
"0123456789:;<=>?"
"¡ABCDEFGHIJKLMNO"
"PQRSTUVWXYZÄÖÑÜ§"
"¿abcdefghijklmno"
"pqrstuvwxyzäöñüà"
"^{}[]\\~|€"   // escaped symbols 
"א";
#else
"€א";
#endif

char ucs2[512];
#if 0
struct sRange {
  unsigned short min,max;
};
struct sRange gsm7_legal[] = {
    {32,95},{97,126},{161,161},{163,167},{191,191},{196,201},{209,209},
    {214,214},{216,216},{220,220},{223,224},{228,230},{232,233},{236,236},{241,242},{246,246},
    {248,249},{252,252},{915,916},{920,920},{923,923},{926,926},
    {928,928},{931,931},{934,934},{936,937},{8364,8364}
};

bool isGSM7(unsigned short ucs) {
  for (int i=0; i< sizeof(gsm7_legal)/sizeof(sRange);i++) {
    if (ucs >= gsm7_legal[i].min && ucs <= gsm7_legal[i].max)
      return true;
  }
  return false;
}
#endif

void gsm7check(PDU mypdu) {
  // scan gsm7 UTF8 string and print out the UCS2 values
  int numoctets = mypdu.utf8_to_ucs2(gsm7,ucs2);
  std::cout << "Found " << numoctets << " octets" << std::endl;
  // UCS2 is a bigendian number, we nned to convert to little endian
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