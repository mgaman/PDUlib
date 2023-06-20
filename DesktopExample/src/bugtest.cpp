/*
   This example has an overflow on message buffer
*/
#include <iostream>
#include <pdulib.h>
#include <cstring>
//#include "credentials.h"
#include "Utilities.h"

char *pduIn = "07917952140230F2040C91795254198968000022705022327121808080604028180E880468C1003D1C90886442A9582E988C06C4E9783EA09068442A994EA8946AC56AB95EB0986C46ABD96EB89C6EC7EBF97EC0A070482C1A8FC8A472C96C3A9FD0A8744AAD5AAFD8AC76CBED7ABFE0B0784C2E9BCFE8B47ACD6EBBDFF0B87C4EAFDBEFF8BC7ECFEFFBFF";
//char *pduIn = "06918919015000400C918919821020300008227070116181818C050003510201005500700064006100740065004000440065007600690063006500490064003D00320036003200370032004000440065007600690063006500550072006C003D0068007400740070003A002F002F00390034002E003100380032002E003100350034002E00320038002F006B00650079007000610064002F0068006F006D0065004000440065";
//char *pduIn = "06918919015000640A8089050034700008227070115214818C050003CD0201005500700064006100740065004000440065007600690063006500490064003D00320036003200370032004000440065007600690063006500550072006C003D0068007400740070003A002F002F00390034002E003100380032002E003100350034002E00320038002F006B00650079007000610064002F0068006F006D0065004000440065";

void bufOvflow(PDU mypdu) {
    if (mypdu.decodePDU(pduIn)) {
        std::cout << mypdu.getSCAnumber() << std::endl;
        std::cout << mypdu.getSender() << std::endl;
        std::cout << mypdu.getTimeStamp() << std::endl;
        if (mypdu.getOverflow())
            std::cout << "Warning, partial message only\n";
        std::cout << mypdu.getText() << std::endl;
    }
}

// Issue #27
char *lcase = "06910379010023040dd05774983eafc2010000223060804401803c456c318901c564301868da9C822cd0270654cc3893cd2008925c52B34190e5990551909367d609a2229da058ed059B8164341d0c06";
char *ucase = "06910379010023040DD05774983EAfC2010000223060804401803C456C318901C564301868DA9C822CD0270654CC3893CD2008925C52B34190E5990551909367D609A2229DA058ED059B8164341D0C06";
char ltext[200];
char utext[200];
void lowercasePDU(PDU mypdu) {
    // decode the 2 strings, save text part
    if (mypdu.decodePDU(lcase)) {
        strcpy(ltext,mypdu.getText());
        std::cout << "lower " << ltext << std::endl;
    }
    if (mypdu.decodePDU(ucase)) {
        strcpy(utext,mypdu.getText());
        std::cout << "upper " << utext << std::endl;
    }
    // compare the 2 texts
    if (strcmp(ltext,utext) == 0)
        std::cout << "Compare upper/lower succeeded\n";
    else
        std::cout << "Compare upper/lower failed\n";
}

// Issue 28,30
// GSM7 multipart , check first character not lost, Issues 28,30
char *part1 = "07911326040040F5440B911300000000F0000022114180348240A005000300040182757AB90D22BFD96F79790E7A93D37450990E6AA7DDF539C85E6683DEE4F41B149EC3CB7277985E9783EA7417C8196E83E2F5741854C697E5E3343D4C4FBFDDE536284D06B9D3F334285CA7BB40D2F2B83E0FBBC9E132282C0695C3A0F49C5E0685EB74D03C4D07C1DFF379BA5D9F83E2F5B40B141C8FEBF370BB3E0791D3737ADA3DA6A7DF";
char *firstline = "Autem dolores odit et minus vel odio aspernatur ut. Nam quia exercitationem id nisi aut. Recusandae ab ea iste aut sit possimus qui. Accusamus distinctio";

void multipartCharNotLost(PDU mypdu) {
    if (mypdu.decodePDU(part1)) {
        std::cout << mypdu.getText() << std::endl;
        if (strlen(firstline) == strlen(mypdu.getText())) {
            if (strcmp(firstline,mypdu.getText()) == 0)

                std::cout << "String compare succeeded\n";
            else
                std::cout << "String compare failed\n";
        }
        else if (mypdu.getOverflow())
            std::cout << "Buffer Overflow\n";
            else
                std::cout << "Decode failed\n";
    }
}

// Issue #33
// Alphanumeric OA translate too many characters

const char *pduline = "06910379010023040DD05774983EAFC20100002230603023928046456C318901D960B013C8027D62404FCA13447D3A41CB27A6F904519F59D01519A49EA6A02A146479664114E4E4997582A8482708167BC168A0984C078301";
void alphanumericOA(PDU mypdu) {
    if (mypdu.decodePDU(pduline)) {
        std::cout << mypdu.getSCAnumber() << std::endl;
        std::cout << mypdu.getSender() << std::endl;
        std::cout << mypdu.getText() << std::endl;
    }
    else {
        if (mypdu.getOverflow())
            std::cout << "Buffer overflow, change PDU constructor\n";
        else
            std::cout << "Decode failed\n";
    }
}

void gsm7string7(PDU mypdu) {
#if false
const char *line = "07917952140230F2040C917952541989680000222170519401800741F1985C369F01"; //Abcdefg
const char *example = "Abcdefg";
#else
const char *line = "07917952140230F2040C917952541989680000222170519472800E41F1985C369F63B219AD66BB01"; // ABcdefg1234567
const char *example = "Abcdefg1234567";
#endif
    if (mypdu.decodePDU(line)) {
        std::cout << mypdu.getText() << std::endl;
        if (strcmp(mypdu.getText(),example) == 0)
            std::cout << "Compare succeeded\n";
        else
            std::cout << "Compare failed\n";
    }
    else if (mypdu.getOverflow())
        std::cout << "BufferOverflow\n";
        else
            std::cout << "Decode error\n";
}

// issue #38
void issue38(PDU mypdu) {
    const char *pdu = "02919311000181F000000B3EC7690B0683C1603250D04D7FBBD36F90343D1E97D9EC34A8EA02F15A31182C3693C560329B6C46F3815A20282C060ACACBE11808454C01";
    if (mypdu.decodePDU(pdu)) {
        std::cout << mypdu.getSCAnumber() << std::endl;
        std::cout << mypdu.getSender() << std::endl;
        std::cout << mypdu.getTimeStamp() << std::endl;
        std::cout << mypdu.getText() << std::endl;
    }
    else {

    }
}

void issue39(PDU mypdu) {
    const char *pdu = "0791246020099990640681991976000832602111455580840500033107010044006F0073006C006500630068006C00690020006A0073006D0065002000730065002C"
                      "0020007A006500200064006E006500730020006D00610074006500200063006F0020006F0073006C00610076006F007600610074002100200041002000700072006F"
                      "0074006F007A006500200064006100720065006B00200070";
    if (mypdu.decodePDU(pdu)) {
        int *cinfo = mypdu.getConcatInfo();
        if (cinfo[1] == 0)
            std::cout << "This is a standalone message\n";
        else {
            std::cout << "Part " << cinfo[1] << " of " << cinfo[2] << std::endl;
        }
        std::cout << mypdu.getSCAnumber() << std::endl;
        std::cout << mypdu.getSender() << std::endl;
        std::cout << mypdu.getTimeStamp() << std::endl;
        std::cout << mypdu.getText() << std::endl;
    }
    else if (mypdu.getOverflow())
            std::cout << "BufferOverflow\n";
        else
            std::cout << "Decode error\n";
}
#if 0
void issue36(PDU mypdu) {
    const char *lines[] = {
        "multipart first line XX\n",
        "multipart second line YY\n",
    };
    //int cmss=260;    // Issue 36  >= 255
    int cmss=99;    // Issue 36  < 255
    int numparts = sizeof(lines)/sizeof(const char *);
    for (int j=0; j<numparts; j++)  {
        sendSMS(mypdu,Target,lines[j],cmss,numparts,j+1);  // send as multipart
    }  
}
#endif

