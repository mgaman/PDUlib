<p align="center"><img src="images/modem.png"></p>  

# PDUlib
Encode/Decode PDU for sending/receiving SMS.
## Alphabets
Both the default GSM 7 bit and UCS-2 16 bit alphabets are supported which means that you can, in practice, send and receive in any language you want. 
BTW Emojis can also be sent.  The Arduino IDE does not support inserting emojis into text. The VS Code user should install the Emoji plugin.  
This implementation does not process the User Data Header (UDH). The practical implication of this is that concatenated messages or national language extensions are not supported.
## Target audience
The code is written in plain C++ so it should be usable by both desktop and Arduino coders.
# API
## decodePDU
<b>bool decodePDU(const char *pdu)</b>  
If using a GSM modem, in PDU mode **not TEXT mode** an incoming message is displayed as.  
+CMT: nn    where nn is length  
XXXXXXXXX   where XXXXXX is a string of hexadecimal character. It is this line that should be decoded.  
After decoding the PDU its constituent parts can be recovered with the following methods.
## getSCAnumber
<b>const char *getSCAnumber()</b>  
Returns the number of the SCA from an incoming message, i.e. the Service Centre that delivered the message.  
## getSender
<b>const char *getSender()</b>  
Returns the phone number of the sender from an incoming message.  
## getTimeStamp
<b>const char *getTimeStamp()</b>    
Returns the timestamp of an incoming message in the format YYMMDDHHMMSS.  
## getText
<b>const char *getText()</b>  
Returns the body of an incoming message. Note that it is a UTF-8 string. In a Desktop environment it should be displayable, as is.  However in a resource restricted environment e.g. an OLED screen attached to an Arduino you will probably have to create a solution for non-ASCII characters.
## encodePDU
<b>int encodePDU(const char *recipient,const char *message)</b>  
1. recipient. The phone number of the recipient. It must conform to the following format, numeric only, no embedded white space. An international number must be preceded by '+'.
2. message. The body of the message, in UTF-8 format. This is typically what gets typed in from any keyboard driver. The code will scan the message to deduce if it is all GSM 7 bit, or not. If all GSM 7 bit then the maximum message length allowed is 160 characters, else 70 CSU-2 symbols.
3. Return value. This is the length of the PDU and is used in the GSM modem command +CGMS when sending an SMS. **Note** ths is not the length of the entire message so can be confusing to one that has not read the documentation. To learm the structure of a PDU read [here](https://bluesecblog.wordpress.com/2016/11/16/sms-submit-tpdu-structure/)  
A return value of -1 indicates that the message is longer than the maximum allowed.
## setSCAnumber
<b>void setSCAnumber(const char *)</b>  
Before one can encode and send a PDU the number of the Service Centre must be known.  
Typically this can be discovered in a GSM modem by issuing the command  
AT+CSCA?  
## getSMS  
<b>const char *getSMS()</b>  
This returns the address of the buffer created by **encodePDU**. The buffer already contains the termination character CTRL/Z so can be used as is.  
# Development and Debugging
The code was developed in VS Code and Ubuntu desktop environment.  
There are a few differences between the VS Code environment and the Arduino IDE which is the default mode for many Arduino developers. The main difference is the file name of an Arduino sketch. In VS Code this is a classical C++ file with the extension **cpp** e.g. **anyName.cpp**. In Arduino IDE the extension is **ino** and the leading part of the name **must** be the same as that of the folder enclosing the sketch e.g. for a sketch called **blah** the sketch folder is **blah** and the sketch file name **blah.ino**.  
I have constructed the source tree so that it is valid for both Arduino IDE and PlatformIO environments. For example the files **pdulib/examples/Decode/Decode.ino** and **pdulib/examples/Decode/src/Decode.c++** are identical but with different names and location.<br>
I use the extension c++ to prevent the Arduino IDE compiling both ino and any cpp files it finds. PlatformIO will happily compile c++ files.<br> 
Arduino has its own peculiarities for the location of library header files. It folds many standard libraries into **Arduino.h** whereas the Desktop user has to name them specifically. This is the reason behind the **ARDUINO_BASE** macro. 
```
#ifdef ARDUINO_BASE
#include <Arduino.h>     
#else
#include <math.h>
#include <string.h>
#endif
```
The library, as released, is configured for the Arduino user i.e. in **pdulib.cpp** the macro **ARDUINO_BASE** is enabled. However, if you clone this library for further development and intend to use the Desktop environment, do as follows.  
1. Comment out the macro ARDUINO_BASE. You can now compile for the desktop.   
2. For Arduino sketches, add the line **build_flags=-DARDUINO_BASE** to the platformio.ini configuration file. You can now compile for Arduino.<br>

When developing a new Arduino sketch you must also show the sketch where pdulib is located. In a classical PlatformIO layout, library files are located in the pdulib/examples/sketch/lib/pdulib folder. In reality they are in the pdulib/src folder. To overcome this, create the folder pdulib/examples/sketch/lib/pdulib and create soft links from there to the actual source files.<br>
The script **createSoftLinks.sh** does this automagically for all the examples.
```
cd pdulib/examples/sketch/lib
mkdir pdulib
cd pdulib
ln -s ../../../../src/pdulib.cpp pdulib.cpp
ln -s ../../../../src/pdulib.h pdulib.h
```
## Desktop
My GSM modem is an SIM900 Arduino breakout board connected to an FTDI USB-Serial device, thus it appears as an /dev/ttyUSB* device. On Windows it will be COMnn where nn is a number asigned by the OS.    
The modem needs its own power supply as the current supplied by the FTDI is insufficient.  
Debugging in desktop mode is more convenient as it allows one to set breakpoints, watch variables etc. Something not available to the Arduino developer.  
### Serial port
It is essential to configure the serial port correctly as some drivers edit incoming data in an annoying way e.g. converting carriage returns to line feeds.  
Read the main() code in phonetester.cpp.
### Typical Usage Sending an SMS
This is C++ pseudo code. Note that I have ignored the issue of parsing data coming from the modem via the serial port, which is outside the scope of this document.
```
#include <pdulib.h>
int main(int argc,char *argv[]) {  
    PDU mypdu = PDU(); 
    sp = open(argv[1], O_RDWR);  
    // ensure that the modem is in PDU mode
    write(sp,"AT+CMGF=0\r",10); 
    // discover and save SCA number  
    write(sp,"AT+CSCA\r",11);  
    --> responds +CSCA: "nnnnnn",129  where nnnnnn is a phone number , extract the field 
    mypdu.setSCAnumber("nnnnnn");  
    // create an SMS buffer  
    //int len = myPDU.encodePDU("+12121234567","שלום");  
    int len = myPDU.encodePDU("+12121234567","hi there");  
    char temp[20];  
    sprintf(temp,"AT+CMGS=%d\r",len); // create the command 
    write(sp,temp,strlen(temp));    // write command to modem  
    sleep(1);   // wait for > response to pass  
    write(sp,mypdu.getSMS(),strlen(mypdu.getSMS));  // write the whole buffer to the serial port  
}  
```
### Typical Usage Receiving an SMS
Assuming the data
```
+CMT: "",24
0791xxxx140230F2040C91xxxxxxxxxxxx00001290813175212105C8329BFD06
```
arrived from the GSM modem via the serial port. The code below would print out the senders phone number and the body of the message.  
Note it is not necessary to save the SCA number if just handling incoming messages.
```
#include <pdulib.h>
int main(int argc,char *argv[]) {
    PDU mypdu = PDU(); 
    sp = open(argv[1], O_RDWR);  
    // ensure that the modem is in PDU mode
    write(sp,"AT+CMGF=0\r",10); 
    mypdu.decodePDU("07917952140230F2040C917952xxxxxxxx00001290813175212105C8329BFD06");
    std::cout << mypdu.getSender() << std::endl;  // prints "+nnnnnnnnn"
    std::cout << mypdu.getText() << std::endl;  // prints "Hello"
    std::cout << mypdu.getTimeStamp() << std::endl;  // prints "210918135712"
}  
```
## buildUtf
<b>void  buildUtf(unsigned long codepoint, char *target)</b>  
This is a helper method for those environments where inserting characters e.g. an emoji is just not possible. The Arduino IDE is a good example.  
<b>Supercedes builtUtf16.</b>  
1. codepoint. The code position of a character e.g. smiley emoji, ace of spades. For
examples see https://en.wikipedia.org/wiki/List_of_Unicode_characters .  
2. target. A buffer to receive the string.   
### Example
```
#include <pdulib.h>
PDU mypdu = PDU();
char finalMsg[50]; // assemble final message here
char tempbuf[10];  // temporary workspace

// some Unicode codepoints
#define POO 0x1F4a9
#define SPADE 0x2660
#define YEN 165
  
int main() {
  // build up the message
  strcpy(finalMsg,"Here is some poo ");
  mypdu.buildUtf(POO,tempbuf);
  strcat(finalMsg,tempbuf);
  mypdu.buildUtf(SPADE,tempbuf);
  strcat(finalMsg,tempbuf);
  mypdu.buildUtf(YEN,tempbuf);
  strcat(finalMsg,tempbuf);
  // now carry on as normal
  mypdu.setSCAnumber("+12125557777");
  int len = mypdu.encodePDU("+12125556666",finalMsg);
  ....
  ....
}
```   

## DesktopExample
### phonetester.cpp
This is the main module. The main function expects 1 parameter, the serial port of the modem. The value of this parameter is defined in .vscode/launch.json/args.  

After opening the serial port and configuring it correctly, two threads are started up.  
**serialHandler** reads all incoming data from the modem, packages up complete lines and places the lines into a queue.  
**startup** configures the modem e.g. by setting SMS PDU mode and then exits.  
Once **startup** finishes two more threads are started up.  
**unsolicited** reads discrete lines from the queue created by **serialHandler** and processes each one as needed. I have provided some examples, feel free to add more.  
**consoleHandler** is a crude mechanism to kick off actions from the keyboard. I have implemented a simple menu where the command 's' sends an SMS. Feel free to customise the example and add more.
## Arduino Examples
When compiling for Arduino AVR, uncomment the line **#define PM** at the beginning of pdulib.h.  
This transfers some static tables to progmem and frees up 128 bytes of RAM.  
My development environment was an SIM900 shield plugged into an Arduino Uno, configured to use software serial.  
Note that I have ignored all of the actions required to power up the shield and analyze incoming data from the shield. That is outside the scope of this document.  
### Decode
```
#include <Arduino.h>
#include <pdulib.h>

PDU mypdu = PDU();
void setup() {
  char temp[20];
  Serial.begin(9600);
  mypdu.decodePDU("07917777140230F2040C9188885419999900001280018153832106D17B594ECF03");
  Serial.println(mypdu.getSCAnumber());
  Serial.println(mypdu.getSender());
  Serial.println(mypdu.getTimeStamp());
  Serial.println(mypdu.getText());
}

void loop() {
}
```
### Encode
```
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <pdulib.h>

SoftwareSerial GSM(2,3);

// example receiver phone numbers
const char *nat = "*********";   // national phone number
const char *intl = "+*********";   // international phone number
// example messages
const char *allascii = "Hello";
const char *notascii = "שלום";

PDU mypdu = PDU();
void setup() {
  char temp[20];
  Serial.begin(9600);
  GSM.begin(9600);
  GSM.print("AT+CMGF=0\r");  // put modem into PDU mode
  delay(1000);
  mypdu.SetSCAnumber("+xxxxxxxx");   // insert your networks SCA number here
  int len = mypdu.encodePDU(nat,notascii); 
  Serial.print("SMS length ");Serial.println(len);
  Serial.println(mypdu.getSMS());
  sprintf(temp,"AT+CMGS=%d\r",len); // create SMS submit command
  GSM.print(temp);           // issue SMS submit command
  delay(1000);               // wait for > prompt to pass
  GSM.print(mypdu.getSMS()); // send the message
}

void loop() {
  if (GSM.available())
    Serial.write(GSM.read());
}
```

# How many characters can I send?
The common misconception is that a single SMS can contain up to 160 characters. This does not take into account
the definition of a character.  
The space available for a message in a PDU is 140 bytes or 1120 bits. For a 7 bit character, or septet, there is thus
space for 1120/7 = 160 characters. For 16 bits characters (anything not GSM 7) there is space for 1120/16 = 70 characters.  
Furthermore, in GSM7, there are a number of escaped characters e.g. [ and ]. These take up 2 septets out of the allowed 160.
