/*
   Check scenarios for Encode errors
   GSM7
        Message > 160 characters
            Workbuff too small
            Workbuff large
        Message <= 160 characters
            Workbuff too small
            Workbuff large
   16 bit
        Message > 70 UCS2 symbols
            Workbuff too small
            Workbuff large
        Message <= UCS2 symbols
            Workbuff too small
            Workbuff large
*/
#include <iostream>
#include <pdulib.h>

// GSM7
const char *gsm7[] = {
                    "01234567890123456789012345678901234567890123456789012345678901234567890123456789"
                    "01234567890123456789012345678901234567890123456789012345678901234567890123456789"
                    "AB",  // 162 chars
                    "01234567890123456789012345678901234567890123456789012345678901234567890123456789"
                    "01234567890123456789012345678901234567890123456789012345678901234567890123456789", // 160 chars
                    "01234567890" // 10 chars fits in default work buffer
                };

const char *bit16[] = {
                        "אא1234567890א1234567890א1234567890א1234567890א12345678901234567890123456789", // > 70 UCS2
                        "012345678901234567890123456789012345678901234567890123456789012345678א",   // 70 UCS2
                        "0123456א" // fits in default work buffer
                    };

const char *multipart[] = {
                        "multipart part 1",
                        "multipart part 2",
                        "multipart part 3",
                        "multipart part 4"
};

/*
   GSM7 data
*/
void EncodeErrorsPart1(PDU mypdu){
  mypdu.setSCAnumber("+9725400000");   // legal format
  //mypdu.setSCAnumber("-9725400000");   // illegal character
  //mypdu.setSCAnumber("+972540000098765432101");  // too long
  for (int i = 0;i<sizeof(gsm7)/sizeof(const char *);i++) {
      std::cout << gsm7[i] << std::endl;
      int len =  mypdu.encodePDU("0541234567",gsm7[i]);
      switch (len) {
          case mypdu.GSM7_TOO_LONG:
              std::cout << "Message too long to send as a single message, change to multipart" << std::endl;
              break;
          case mypdu.WORK_BUFFER_TOO_SMALL:
              std::cout << "Work buffer too small, change PDU constructor" << std::endl;
              break;
          case mypdu.ADDRESS_FORMAT:
                std::cout << "SCA or Target address illegal characters or too long" << std::endl;
              break;
          case mypdu.UCS2_TOO_LONG:
          case mypdu.MULTIPART_NUMBERS:
          case mypdu.ALPHABET_8BIT_NOT_SUPPORTED:
                std::cout << "How did we get here?" << std::endl;
              break;
          default:
                std::cout << "Message length " << len << std::endl;
              break;
      }
  }
}

/*
    Multipart errors
*/
void EncodeErrorsPart2(PDU mypdu) {
  int numparts = sizeof(multipart)/sizeof(const char *);
  //int thispart = 2; // legal
  int thispart = 0; // illegal, start from 1
  //int thispart = numparts+1; // illegal, < numparts
  int ref = 99;
  std::cout << multipart[1] << std::endl;
  int len =  mypdu.encodePDU("0541234567",multipart[1],ref,numparts,thispart);
  switch (len) {
      case mypdu.GSM7_TOO_LONG:
        std::cout << "Message too long to send as a single message, change to multipart" << std::endl;
        break;
      case mypdu.WORK_BUFFER_TOO_SMALL:
        std::cout << "Work buffer too small, change PDU constructor" << std::endl;
        break;
      case mypdu.ADDRESS_FORMAT:
        std::cout << "SCA or Target address illegal characters or too long" << std::endl;
        break;
      case mypdu.MULTIPART_NUMBERS:
        std::cout << "Multipart numbers illogical" << std::endl;
        break;
      case mypdu.UCS2_TOO_LONG:
      case mypdu.ALPHABET_8BIT_NOT_SUPPORTED:
            std::cout << "How did we get here?" << std::endl;
          break;
      default:
            std::cout << "Success. Message length " << len << std::endl;
          break;
  }
}

/*
    UCS2 data
*/
void EncodeErrorsPart3(PDU mypdu) {
  mypdu.setSCAnumber("+9725400000");   // legal format
  //mypdu.setSCAnumber("-9725400000");   // illegal character
  //mypdu.setSCAnumber("+972540000098765432101");  // too long
  for (int i = 0;i<sizeof(bit16)/sizeof(const char *);i++) {
      std::cout << bit16[i] << std::endl;
      int len =  mypdu.encodePDU("0541234567",bit16[i]);
      switch (len) {
          case mypdu.UCS2_TOO_LONG:
              std::cout << "Message too long to send as a single message, change to multipart" << std::endl;
              break;
          case mypdu.WORK_BUFFER_TOO_SMALL:
              std::cout << "Work buffer too small, change PDU constructor" << std::endl;
              break;
          case mypdu.ADDRESS_FORMAT:
                std::cout << "SCA or Target address illegal characters or too long" << std::endl;
              break;
          case mypdu.GSM7_TOO_LONG:
          case mypdu.MULTIPART_NUMBERS:
          case mypdu.ALPHABET_8BIT_NOT_SUPPORTED:
                std::cout << "How did we get here?" << std::endl;
              break;
          default:
                std::cout << "Message length " << len << std::endl;
              break;
      }
  }
}

