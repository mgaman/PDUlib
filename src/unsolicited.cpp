#include <iostream>
#include <string>
#include <queue>
#include <bitset>
#include <unistd.h> // write(), read(), close()
#include <string.h>
#include "pdulib.h"

extern std::queue<std::string> inputQueue;
PDU mypdu = PDU();

std::string cgregstates[] = {
    "not registered",
    "registered, home",
    "not registered",
    "registration denied",
    "unknown",
    "registered, roaming"
};

/*
    Pop messages off the queue and process them
*/

char *atc = "+AT+CSCA?\r";

void unsolicited(int sp) {
    bool nextLineSMS = false;
    bool ipaddressprinted = false;
    std::cout << "Unsolicited started\n";
    write (sp,atc,strlen(atc));
    while (true) {
        if (!inputQueue.empty()) {
            std::string response = inputQueue.front();
            inputQueue.pop();
            std::cout << response << std::endl;
            // check for known responses
            if (response.compare(0,5,"+CLIP") == 0)    // caller id
            {
                // isolate number
                std::cout << "Incoming call from ";
                int start = response.find("\"");
                std::cout << response.substr(start+1) << std::endl;
            }
            else if (response.compare(0,5,"+CMT:") == 0) { // incoming SMS
                // isolate number
                // +CMT: "",nn
                std::cout << "Incoming SMS length ";
                int value = std::stoi(response.substr(response.find(',')+1));
                std::cout << value << std::endl;
                nextLineSMS = true;
            }
            else if (nextLineSMS) {
                if (mypdu.decodePDU(response.c_str())) {
                    std::cout << "From: " << mypdu.getSender() << std::endl;
                    std::cout << "Message: " << mypdu.getText() << std::endl;
                }
                nextLineSMS = false;
            }
            else if (response.compare(0,6,"+CSCA:") == 0) {  // get sca number
//                std::cout << "SCA number ";
                int start = response.find('"')+1;
                int end = response.find(',') -1;
//                std::cout << response.substr(start,end-start) << std::endl;
                mypdu.setSCAnumber(response.substr(start,end-start).c_str());
//                std::cout << mypdu.getSCAnumber() << std::endl;
            }
#if 0
            else if (response.compare(0,6,"+CIEV:") == 0) {
                char *start = linebuf+7;
//                char *end = strchr(start, ',');
//                *end = 0;
                Serial.print("Event ");
                Serial.println(start);
                // analyse event
                if (strncmp("\"CALL\"",start,6)==0) {
                    start = strchr(start,',');
                    int value = atoi(++start);
                    Serial.print("Call ");
                    Serial.println( value == 0 ? F("disconnected") : F("connected"));
                }
            }
#endif
            else if (response.compare(0,7,"+CGREG:") == 0) {
                int value = std::stoi(response.substr(response.find(' ')+1));
                std::cout << "Network registration is ";
                std::cout << cgregstates[value] << std::endl;
                if (!ipaddressprinted) {
                    write(sp,"AT+CIFSR\r",9);  // Get our ip address
                    ipaddressprinted = true;
                }
            }
            else if (response.compare(0,11,"+HTTPACTION") == 0)
                write(sp,"AT+HTTPREAD\r",12);
        }
    }
}