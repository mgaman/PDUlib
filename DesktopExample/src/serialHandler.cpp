#include <iostream>
#include <string>
#include <queue>
// C library headers
//#include <stdio.h>
#include <string.h>

// Linux headers
//#include <fcntl.h> // Contains file controls like O_RDWR
//#include <errno.h> // Error integer and strerror() function
//#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

// Allocate memory for read buffer, set size according to your needs
#define MAX_LINE_LENGTH 334 // when CMFG=0
static char linebuf[MAX_LINE_LENGTH+10];
static char read_buf [MAX_LINE_LENGTH*2];
int lineoffset = 0;
//int lineNumber = 0;
extern std::queue<std::string> inputQueue;
extern std::queue<std::int8_t> GtQueue;

void serialHandler(int sp) {
        // now endless loop to read and display incoming data
    std::cout << "serial thread started\n";
    int inOffset;
    while (true) {
        int nSerIn = read(sp,read_buf,sizeof(read_buf));
        if (nSerIn > 0) {
//            fwrite(read_buf,1,nSerIn,stdout);
            // buffer into individual lines
            inOffset = 0;
            while (inOffset < nSerIn) {
                linebuf[lineoffset++] = read_buf[inOffset++];
                // looking for a single >
                if (linebuf[lineoffset-1] == '>')
                    GtQueue.push(1);
                // check for cr/lf or buffer full
                // if so print contents and reset offset
                if (linebuf[lineoffset-1] == 0x0a || lineoffset == MAX_LINE_LENGTH) {
                    linebuf[lineoffset] = 0;    // END MARKER
                    inputQueue.push(std::string(linebuf));
                    lineoffset = 0;
                }
                if (strncmp(linebuf,"+CMGS:",6) == 0) { 
                    GtQueue.push(2);
                }
            }       
        }
    }
}
