<p align="center"><img src="images/modem.png"></p>  

# PDUlib
Encode/Decode PDU for sending/receiving SMS.
## Alphabets
Both the GSM 7 bit alphabet and UCS-2 16 bit alphabets are supported which means that you can, in practice, send and receive in any language you want.  
BTW Emojis can also be sent.  The Arduino IDE does not support inserting emojis into text. The VS Code user should install the Emoji plugin.
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
Returns the number of the SCA, i.e. the Service Centre that delivered the message.  
## getSender
<b>const char *getSender()</b>  
Returns the phone number of the sender.  
## getTimeStamp
<b>const char *getTimeStamp()</b>    
Returns the timestamp of the message in the format YYMMDDHHMMSS.  
## getText
<b>const char *getText()</b>  
Returns the body of the message. Note that it is a UTF-8 string so should be displayable, as is.  
## encodePDU
<b>int encodePDU(const char *recipient,const char *message)</b>  
1. recipient. The phone number of the recipient. It must conform to the following format, numeric only, no embedded white space. An international number must be preceded by '+'.
2. message. The body of the message, in UTF-8 format. This is typically what gets typed in from any keyboard driver. The code will scan the message to deduce if it is all 7 bit ASCII, or not. If all 7 bit ASCII then the maximum message length allowed is 160 characters, else 70 CSU-2 symbols.
3. Return value. This is the length of the PDU and is used in the GSM modem command +CGMS when sending an SMS. **Note** ths is not the length of the entire message so can be confusing to one that has not read the documentation. To learm the structure of a PDU read [here](https://bluesecblog.wordpress.com/2016/11/16/sms-submit-tpdu-structure/) 
## setSCAnumber
<b>void setSCAnumber(const char *)</b>  
Before one can encode an PDU the number of the Service Centre must be known.  
Typically this can be discovered in a GSM modem by issuing the command  
AT+CSCA?  
## getSMS  
<b>const char *getSMS()</b>  
This returns the address of the buffer created by **encodePDU**. The buffer already contains the termination character CTRL/Z so can be used as is.  
# Development and Debugging
The code was developed in VS Code and Ubuntu desktop environment.
## Desktop
My GSM modem is an SIM900 Arduino breakout board connected to an FTDI USB-Serial device, thus it appears as an /dev/ttyUSB* device.  
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
## buildUtf16
<b>void  buildUtf16(unsigned long codepoint, char *target)</b>
1. codepoint. The code position of a character whose value is > 0xFFFF e.g. emojis. For
examples see https://unicode.org/emoji/charts/full-emoji-list.html .  
2. target. A buffer to receive the string.   
This is a helper method for those environments where inserting characters e.g. an emoji is just not possible. The Arduino IDE is a good example.
### Example
```
#include <pdulib.h>
PDU mypdu = PDU();
char finalMsg[50]; // assemble final message here
char tempbuf[10];  // temporary workspace
#define POO 0x1F4A9

int main() {
  // build up the message
  strcpy(finalMsg,"Here is some poo ");
  mypdu.buildUtf16(POO,tempbuf);
  strcat(finalMsg,temp);
  // now carry on as normal
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
