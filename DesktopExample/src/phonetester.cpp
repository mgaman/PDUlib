// C++ headers
#include <iostream>
#include <thread>
#include <queue>
#include <string>

// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <pdulib.h>

int serial_port;
std::queue<std::string> inputQueue;
std::queue<int8_t> GtQueue;  // to let us know when a single > arrives

// Create new termios struct, we call it 'tty' for convention
// No need for "= {0}" at the end as we'll immediately write the existing
// config to this struct
struct termios tty;

// threads prototypes
void serialHandler(int);
void unsolicited(int sp);
void startup(int sp);
void consoleHandler(int sp,PDU);

// Read in existing settings, and handle any error
// NOTE: This is important! POSIX states that the struct passed to tcsetattr()
// must have been initialized with a call to tcgetattr() overwise behaviour
// is undefined
PDU mypdu = PDU(350);

// prototypes
void greekDecode(PDU);
void gsm7check(PDU);
void gsm7encode(PDU);
void UdhDecode(PDU);
void bufOvflow(PDU);
void lowercasePDU(PDU);
void multipartCharNotLost(PDU);
void EncodeErrorsPart1(PDU);
void EncodeErrorsPart2(PDU);
void EncodeErrorsPart3(PDU);
void alphanumericOA(PDU);
void gsm7string7(PDU);
void issue36(PDU);
void issue38(PDU);
void issue39(PDU);
// Check for errors
int main(int argc, char *argv[]) {
    // here is the space to run tests in desktop mode without modem
    //greekDecode(mypdu);
    //gsm7check(mypdu);
    //gsm7encode(mypdu);
    //UdhDecode(mypdu);
    //bufOvflow(mypdu);
    //lowercasePDU(mypdu);
    //multipartCharNotLost(mypdu);
    //EncodeGSM7errors(mypdu);
    //EncodeErrorsPart2(mypdu);
    //alphanumericOA(mypdu);
    //gsm7string7(mypdu);
    //issue38(mypdu);
    issue39(mypdu);
    ///////////////////////////////////////////////

    if (argc != 2) {
        std::cout <<"Usage: pduapp serial_port\n\n";
        return 1;
    }
    std::cout << argv[1] << std::endl; 
    serial_port = open(argv[1], O_RDWR);
//    serial_port = open("/dev/ttyACM0", O_RDWR);
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
                std::cout << "Attributes all set\n";
                std::thread t1(serialHandler,serial_port);
                std::thread t2(startup,serial_port);
                t2.join();  // wait until startup finished
                // examples that need modem go here
                issue36(mypdu);
                std::thread t3(unsolicited,serial_port);
                std::thread t4(consoleHandler,serial_port,mypdu);
                while (true) {}
            }
        }
        close(serial_port);
    }
}

