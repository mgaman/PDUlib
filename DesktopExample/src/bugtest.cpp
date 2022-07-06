/*
   This example has an overflow on message buffer
*/
#include <iostream>
#include <pdulib.h>

char *pduIn = "07917952140230F2040C91795254198968000022705022327121808080604028180E880468C1003D1C90886442A9582E988C06C4E9783EA09068442A994EA8946AC56AB95EB0986C46ABD96EB89C6EC7EBF97EC0A070482C1A8FC8A472C96C3A9FD0A8744AAD5AAFD8AC76CBED7ABFE0B0784C2E9BCFE8B47ACD6EBBDFF0B87C4EAFDBEFF8BC7ECFEFFBFF";

void bufOvflow(PDU mypdu) {
    mypdu.decodePDU(pduIn);
    std::cout << mypdu.getSCAnumber() << std::endl;
    std::cout << mypdu.getSender() << std::endl;
    std::cout << mypdu.getTimeStamp() << std::endl;
    std::cout << mypdu.getText() << std::endl;
}