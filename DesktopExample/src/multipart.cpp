/*
    Create a multipart message
*/
#include <iostream>
#include <pdulib.h>
#include <string.h>

static const char *scanumber = "+9725400000";
static const char *target = "+9725451111111";


const char *lines[] = {
    "multipart first line\n",
    "multipart second line\n",
    "multipart third line\n",
    "multipart fourth line\n",
};

void multipart(PDU mypdu) { 
    mypdu.setSCAnumber(scanumber);
    int numparts = sizeof(lines)/sizeof(const char *);
    for (int j=0; j<numparts; j++)  {
        int len = mypdu.encodePDU(target,lines[j],99,numparts,j+1);  // 99 is ref number, part number starts from 1
        if (len == -1) {
            if (mypdu.getOverflow())
                std::cout << "Buffer overflow, increase buffer size\n";
            else
                std::cout << "Encode problem, check that SCA, Receiver numbers OK\n";
        }
        else
            std::cout << "Success " << mypdu.getSMS() << std::endl;
    }
}