#include <iostream>
#include <string>
#include <queue>
#include <chrono>
#include <unistd.h> // write(), read(), close()
#include <string.h>

extern std::queue<std::string> inputQueue;
/*
    Initialization of modem where we send a command and expect a response
    usually OK within a set time
*/
#define DEFAULT_APN "uinternet"

const char *atcommands[] = {
    "ATE1\r"
    ,"AT+CLIP=1\r" // enable callerid same A6/SIM900
    ,"AT+CMGF=0\r"  // SMS PDU mode  same
//    ,"AT+CSCA?\r"    // get SCSA number
#if 0
    ,"AT+SAPBR=3,1,\"APN\",\"" DEFAULT_APN "\"\r" 
    ,"AT+SAPBR=2,1\r"  // GET IP ADDRESS
    ,"AT+SAPBR=4,1\r"
    ,"AT+SAPBR=1,1\r"  // OPEN
    ,"AT+SAPBR=2,1\r"  // GET IP ADDRESS
    ,"AT+HTTPINIT\r"
    ,"AT+HTTPPARA=\"CID\",1\r"
    ,"AT+HTTPPARA=\"URL\",\"henrylineage.com\"\r"
    ,"AT+HTTPACTION=0\r"
#endif
};
#define LAST_CASE (sizeof(atcommands)/sizeof(const char *)-1)

void startup(int sp) {
    std::cout << "Startup starting\n";
    int stage = 0;
    int atindex = 0;
    bool running = true;
    bool virginState = true;    // modem already registered
    std::string response = "";
    while (running) {
        if (virginState || !inputQueue.empty()) {
            if (!virginState) {   // normal running
                response = inputQueue.front();
                if (response.compare("\n") != 0)  // dont print empty line
                    std::cout<< response << std::endl;
                inputQueue.pop();
            }
            else
                virginState = false;
//            std::cout << "Stage " << stage << std::endl;
            switch (stage) {
                case 0:  // waiting for CREG   
#if 0
                    if (response.compare(0,6,"NORMAL") == 0) {
                        stage = 0;
                    }
                    if (response.compare(0,8,"+CREG: 1") == 0) {
#else   
                    if (true) {
#endif
                        stage = 1;
                        write(sp,atcommands[atindex++],strlen(atcommands[atindex]));  // no echo
                    }
                    break;
                case 1 ... LAST_CASE:
                    if (response.compare(0,2,"OK") == 0 || response.compare(0,5,"ERROR")== 0) {
                         //   vTaskDelay(5000/portTICK_PERIOD_MS);
                            sleep(5); 
                            write(sp,atcommands[atindex++],strlen(atcommands[atindex]));  // rest of commands
                            stage++;
                        }
                    break;
                default:
                    // got to end of list
                    if (response.compare(0,2,"OK") == 0) {
                        std::cout << "Startup finished\n";
                        running = false;
                    }
                    break;
            }
        }
    }
}
