# PDUlib
Encode/Decode PDU for sending/receiving SMS.
## Alphabets
Both the GSM 7 bit alphabet and UCS-2 16 bit alphabets are supported which means that you can, in practice, send and receive in any language you want.
## Target audience
The code is written in plain C++ so it should be usable by both desktop and Arduino coders.
## Work in progress
The size of buffers needed will be a problem for the Arduino user. I am working on a solution that moves as much static data to flash as possible.
