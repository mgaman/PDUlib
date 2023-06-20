/*
    Create a multipart message
*/
#include <iostream>
#include <pdulib.h>
#include <string.h>

#include "credentials.h"
#include "Utilities.h"


// Issue 36 was reported for GSM7 data, does it also  appear for 16 bit?
const char *lines[] = {
    "multipart first line XX\n",
    "multipart second line YY\n",
    "multipart third line\n",
    "multipart fourth line\n"
};

#define SEND_MULTI
int cmss=260;    // Issue 36  
//int cmss=99;    // Issue 36  
/*
    When running this test serial port not yet open so need to do it ourselves
*/
char returnedMsg[300];
char originalmess[160];
void issue36(PDU mypdu) {
    // need to set SCA here
    mypdu.setSCAnumber(SCAnumber);
    // send multipart as above, then send it back from phone. compare versions

    // create complete original message
#ifdef SEND_MULTI
    strcpy(originalmess,lines[0]);
    strcat(originalmess,lines[1]);
    strcat(originalmess,lines[2]);
    strcat(originalmess,lines[3]);
#endif
    int numparts = sizeof(lines)/sizeof(const char *);
    for (int j=0; j<numparts; j++)  {
#ifdef SEND_MULTI
        sendSMS(mypdu,Target,lines[j],cmss,numparts,j+1);  // send as multipart
#else
        sendSMS(mypdu,Target,lines[j]);  // send as separate lines
#endif
    }  
    cmss++;
#ifdef SEND_MULTI
    getpdu(returnedMsg);
//    strcpy(returningpdu,"07917952140230F2040C917952541989680000321062910342802DA0769D4D4FC3C3723AC89C96CFE92076DA5D5680DA75363D0D0FCBE9A07979FC769341ECB4BBAC00");
    if (mypdu.decodePDU(returnedMsg)) {
        //std::cout << returningpdu << std::endl;
        std::cout << "from " << mypdu.getSender() << std::endl;
        int *cinfo = mypdu.getConcatInfo();
        if (cinfo[1] == 0)
            std::cout << "This is a standalone message\n";
        else {
            std::cout << "Part " << cinfo[1] << " of " << cinfo[2] << std::endl;
        }
        std::cout << mypdu.getText() << std::endl;
        std::cout << "Original and Returned ";
        if (strcmp(originalmess,mypdu.getText()) == 0)
            std::cout << "are ";
        else
            std::cout << "are not ";
        std::cout << "the same\n";
        strcpy(returnedMsg,mypdu.getText());  // so we can examine it
    }
    else {
        std::cout << "Decode failed\n";
    }
#endif
}