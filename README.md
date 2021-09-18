# PDUlib
Encode/Decode PDU for sending/receiving SMS.
## Alphabets
Both the GSM 7 bit alphabet and UCS-2 16 bit alphabets are supported which means that you can, in practice, send and receive in any language you want.
## Target audience
The code is written in plain C++ so it should be usable by both desktop and Arduino coders.
## Work in progress
The size of buffers needed will be a problem for the Arduino user. I am working on a solution that moves as much static data to flash as possible.
# API
## decodePDU
**bool decodePDU(const char *pdu)**<br>
If using a GSM modem, in PDU mode <b>not TEXT mode</b> an incoming message is displayed as<br>
+CMT: nn    where nn is length<br>
XXXXXXXXX   where XXXXXX is a string of hexadecimal character. It is this line that should be decoded.<br>
After decodeing the PDU its constituent parts can be recovered with the following methods.
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
1. recipient. The phone number of the recipient. It must conform to the E-164 format i.e. numeric only, no embedded white space. An international number mey, or may not be preceded by '+'.
2. addressType. Must be either INTERNATIONAL_NUMERIC or NATIONAL_NUMERIC. In the case of INTERNATIONAL_NUMERIC preceding '+' in the recipient field can be implicit or explicit.
3. message. The body of the message, in UTF-8 format. This is typically what gets typed in from any keyboard driver.
4. dcs. Data coding scheme. Must be ALPHABET_7BIT or ALPHABET_16BIT. If the message contains only characters in 7 bit ASCII, choose ALPHABET_7BIT, else choose ALPHABET_16BIT. Note that the presence of even just 1 not ASCII 7 bit makes the whole message ALPHABET_16BIT.
## setSCAnumber
**void setSCAnumber(const char *)**<br>
Before one can encode an PDU the number of the Service Centre must be known.  
Typically this can be discovered in a GSM modem by issuing the command  
AT+CSCA?
