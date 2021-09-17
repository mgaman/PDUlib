// C++ headers
#include <iostream>
#include <thread>
#include <queue>
#include <string>
#include <chrono>

// C library headers
#include <stdio.h>
#include <string.h>
#include "pdulib.h"

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

int serial_port;
struct termios tty;
void sendSMS();

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout <<"Usage: pduapp serial_port\n\n";
        return 1;
    }
    serial_port = open(argv[1], O_RDWR);
    if (serial_port < 0) 
        std::cout << "Error " << errno << " from open:" << strerror(errno) << std::endl;
    else {
        std::cout << argv[1] << " opened\n";
        if(tcgetattr(serial_port, &tty) != 0) {
            std::cout << "Error " << errno << "from tcgetattr: " << strerror(errno) << std::endl;
        }
        else {
            // set 8 bits baud rate 9600 No parity, 1 stop , no flow control
            tty.c_cflag &= ~PARENB;
            tty.c_cflag &= ~CSTOPB;
            tty.c_cflag &= ~CSIZE; // Clear all the size bits
            tty.c_cflag |= CS8;
            tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control
            tty.c_lflag &= ~ICANON;  // disable canonical mode
            tty.c_lflag &= ~ECHO; // Disable echo
            tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
            tty.c_iflag &= ~ICRNL;  // do not translate cr to lf
            tty.c_iflag &= ~IGNCR;  // do not ignore cr
            // Set in/out baud rate to be 9600
            cfsetispeed(&tty, B9600);
            cfsetospeed(&tty, B9600);
            // Save tty settings, also checking for error
            if (tcsetattr(serial_port, TCSANOW, &tty) != 0) 
                std::cout << "Error" << errno << " from tcsetattr: " << strerror(errno) << std::endl;
            else {
                sendSMS();
            }
        }
        close(serial_port);
    }
}

char pdubuffer[200];  // should be big enough
PDU pduEngine = PDU();
const char *from = "972545919886";
//const char *message = "hello there";
const char *message = "aaaששש";
char writeBuf[50];   // general purpose
void sendSMS() {
  int len = pduEngine.encodePDU(from,INTL_NUMERIC,message,ALPHABET_16BIT);
  std::cout << pduEngine.getSMS() << std::endl;
  int buflength = strlen(pduEngine.getSMS());
  sprintf(writeBuf,"AT+CMGS=%d\r\n",len);
  write(serial_port,writeBuf,strlen(writeBuf));
    // should wait for ">" but just do a delay instead
  sleep(1);   // 1/10 sec
  write(serial_port,pduEngine.getSMS(),buflength);
} 