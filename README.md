# PDUlib
Encode/Decode PDU for sending/receiving SMS.
## Alphabets
Both the GSM 7 bit alphabet and UCS-2 16 bit alphabets are supported which means that you can, in practice, send and receive in any language you want.
## Target audience
The code is written in plain C++ so it should be usable by both desktop and Arduino coders.
## Work in progress
The size of RAM needed will be a problem for the Arduino user. I am working on a solution that moves as much static data as possible to flash.  
# API
## decodePDU
**bool decodePDU(const char *pdu)**<br>
If using a GSM modem, in PDU mode <b>not TEXT mode</b> an incoming message is displayed as<br>
+CMT: nn    where nn is length<br>
XXXXXXXXX   where XXXXXX is a string of hexadecimal character. It is this line that should be decoded.<br>
After decoding the PDU its constituent parts can be recovered with the following methods.
## getSCAnumber
**char *getSCAnumber()**<br>
Returns the number of the SCA, i.e. the Service Centre that delivered the message.<br>
## getSender
**const char *getSender()**<br>
Returns the phone number of the sender
## getTimeStamp
**const char *getTimeStamp()**<br>
Returns the timestamp of the message in the format YYMMDDHHMMSS<br>
## getText
**const unsigned char *getText()**<br>
Returns the body of the message. Note that it is a UTF-8 string so should be displayable, as is.<br>
## encodePDU
**int encodePDU(const char *recipient, eAddressType,const char *message, eDCS dcs)**<br>
1. recipient. The phone number of the recipient. It must conform to the E-164 format i.e. numeric only, no embedded white space. An international number may, or may not be preceded by '+'.
2. addressType. Must be either INTERNATIONAL_NUMERIC or NATIONAL_NUMERIC. In the case of INTERNATIONAL_NUMERIC preceding '+' in the recipient field can be implicit or explicit.
3. message. The body of the message, in UTF-8 format. This is typically what gets typed in from any keyboard driver.
4. dcs. Data coding scheme. Must be ALPHABET_7BIT or ALPHABET_16BIT. If the message contains only characters in 7 bit ASCII, choose ALPHABET_7BIT, else choose ALPHABET_16BIT. Note that the presence of even just 1 not ASCII 7 bit makes the whole message ALPHABET_16BIT.
4. Return value. This is the length of the PDU and is used in the GSM modem command +CGMS when sending an SMS. **Note** ths is not the length of the entire message so can be confusing to one that has not read the documentation. To learm the structure of a PDU read [here](https://bluesecblog.wordpress.com/2016/11/16/sms-submit-tpdu-structure/) 
## setSCAnumber
**void setSCAnumber(const char *)**<br>
Before one can encode an PDU the number of the Service Centre must be known.  
Typically this can be discovered in a GSM modem by issuing the command  
AT+CSCA?
## getSMS
**const char *getSMS()**  
This returns the address of the buffer created by **encodePDU**. The buffer already contains CTRL/Z as its last character so can be used as is
# Development and Debugging
The code was developed in VS Code and Ubuntu desktop environment.
## Desktop
My GSM modem is an SIM900 Arduino breakout board connected to an FTDI USB-Serial device, thus it appears as an /dev/ttyUSB* device.  
Debugging in desktop mode is more convenient as it allows one to set breakpoints, watch variables etc. Something not available to the Arduino developer.  
### Serial port
It is essential to configure the serial port correctly as some drivers edit incoming data in an annoying way e.g. converting carriage returns to line feeds.
### Typical Usage Sending an SMS
This is C++ pseudo code. Note that I have ignored the issue of parsing data coming from the modem via the serial port, which is outside the scope here.
```
#include <pdulib.h>
PDU mypdu = PDU(); 
sp = open("/dev/ttyUSB0", O_RDWR);  
// ensure that the modem is in PDU mode
write(sp,"AT+CMGF=0\r",10); 
// discover and save SCA number  
write(sp,"AT+CSCA\r",11);  
--> responds +CSCA: "nnnnnn",129  where nnnnnn is a phone number , extract the field 
mypdu.setSCAnumber("nnnnnn");  
// create an SMS buffer  
int len = myPDU.encodePDU("+12121234567",INTERNATIONAL_NUMERIC,"hi there",ALPHABET_7BIT);  
char temp[20];  
sprintf(temp,"AT+CMGS=%d\r",len); // create the command 
write(sp,temp,strlen(temp));    // write command to modem  
sleep(1);   // wait for > response to pass  
write(sp,mypdu.getSMS(),strlen(mypdu.getSMS));  // write the whole buffer to the serial port  
```
If you wanted to send something not ASCII the encodePDU line would look like
```
int len = myPDU.encodePDU("+12121234567",INTERNATIONAL_NUMERIC,"שלום",ALPHABET_16BIT);  
```
### Typical Usage Receiving an SMS
Assuming the data
```
+CMT: "",24
07917952140230F2040C917952xxxxxxxx00001290813175212105C8329BFD06
```
arrived from the GSM modem via the serial port. The code below would print out the senders phone number and the body of the message.  
Note it is not necessary to save the SCA number if just handling incoming messages.
```
#include <pdulib.h>
PDU mypdu = PDU(); 
sp = open("/dev/ttyUSB0", O_RDWR);  
// ensure that the modem is in PDU mode
write(sp,"AT+CMGF=0\r",10); 
mypdu.decodePDU("07917952140230F2040C917952xxxxxxxx00001290813175212105C8329BFD06");
std::cout << mypdu.getSender() << std::endl;  // prints "+972nnnnnnnnn"
std::cout << mypdu.getText() << std::endl;  // prints "Hello"
std::cout << mypdu.getTimeStamp() << std::endl;  // prints "210918135712"
```
