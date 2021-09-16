#include <iostream>
#include "pdulib.h"

//aleph bet gimmel dialled from 0545919886 SCA 9725412003
//+CMT: "",27
//                  0         1         2         3         4         5         6         7            
const char pdu[] = "07917952140230F2040C917952541989680008128013713105210805D005D105D205D3";
const char pdu2[] = "0C917952541989680008128013713105210805D005D105D205D3";
char pdubuffer[200];  // should be big enough

PDU pduEngine = PDU();

int main() {
  pduEngine.decodeAddress(pdu,pdubuffer,OCTETS);
  pduEngine.decodePDU(pdu);
  std::cout << "From: " << pduEngine.getSender() << std::endl;
  std::cout << "Time: " << pduEngine.getTimeStamp() << std::endl;
  std::cout << "Message: ";
  int l = pduEngine.getUtf8Length();
  const unsigned char *mess = pduEngine.getText();
  for (int i=0;i<pduEngine.getUtf8Length();i++)
    std::cout << mess[i];
  std::cout << std::endl;

  pduEngine.decodeAddress(pdu2,pdubuffer,NIBBLES);
}