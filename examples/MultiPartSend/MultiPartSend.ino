#include <Arduino.h>
#include <SoftwareSerial.h>
#include <pdulib.h>

/*
*  Update credentials before running this test
*
*/
const char *SCAnumber = "*******";
const char *Target = "*******";

bool once = false;
bool haveSCA = false;

//SoftwareSerial gsm(2,3);
SoftwareSerial gsm(10,11);

// adjust BUFFER_SIZE until buffer overlow message goes away
#define BUFFER_SIZE 150
PDU mypdu = PDU(BUFFER_SIZE);

#define SEND_MULTI_PART // uncomment to send multipart, comment to send standalone messages
char newMessage[30];

const char *messages[] = {
#ifdef PM
  "Using PM\r\n",
#else
  "Not using PM\r\n",
#endif
  "Line 1\r\nplus more stuff",  // 7 bit alphabet
  "שורה 2",  // 16 bit alphabet
  "😃",  // Surrogate pair
  newMessage    // filled in later
};

unsigned short refNumber = 99;

void setup() {
  Serial.begin(9600);
#ifdef PM
  Serial.println("Using PM");
#else
  Serial.println("Not using PM");
#endif
  Serial.print("Buffer Size ");Serial.println(BUFFER_SIZE);
  gsm.begin(9600);
}

bool waitOn(const char *str,unsigned long timeout) {
  int n = strlen(str);
  const char *pC = str;
  unsigned long done = millis() + timeout;
  while (n && (millis() < done)) {
    if (gsm.available()) {
      if ((char)gsm.read() == *pC) { // character match
        pC++;  // move on to next
        n--;
      }
    }
  }
  return (n == 0);
}

void SendSMS(const char *sms, int len) {
  char temp[20];
  sprintf(temp,"AT+CMGS=%d\r\n",len);
  Serial.print("SMS length ");Serial.println(len);
  Serial.println(sms);
  gsm.print(temp);
  bool wait = waitOn(">",10000);
  if (wait) {
    Serial.print(">");
    gsm.flush();
    gsm.print(sms);
    wait = waitOn("+CMGS",15000);
  }
  if (wait)
    Serial.println("Message send success");
  else {
    Serial.println("Message send fail");
    gsm.flush();
  }
}

void loop() {
  int len;
  int numMessages = sizeof(messages)/sizeof(const char *);
  if (once)
    return;
  else
    once = true;
  sprintf(newMessage,"\r\nMessage %d",refNumber);  // update message
  gsm.flush();
  mypdu.setSCAnumber(SCAnumber);
  for (int n = 0; n<numMessages;n++) {
    Serial.print("Message ");Serial.println(n);
#ifdef SEND_MULTI_PART
    len = mypdu.encodePDU(Target,messages[n],refNumber, numMessages,n+1);
#else
    len = mypdu.encodePDU(Target,messages[n]);
#endif
    if (len < 0) {
      switch (len) {
        case mypdu.UCS2_TOO_LONG:
        case mypdu.GSM7_TOO_LONG:
          Serial.println("Message too long to send as a single message, change to multipart");
          break;
        case mypdu.WORK_BUFFER_TOO_SMALL:
          Serial.println("Work buffer too small, change PDU constructor");
          break;
        case mypdu.ADDRESS_FORMAT:
          Serial.println("SCA or Target address illegal characters or too long");
          break;
        case mypdu.MULTIPART_NUMBERS:
          Serial.println("Multipart numbers illogical or nit numeric");
          break;
        case mypdu.ALPHABET_8BIT_NOT_SUPPORTED:
          Serial.println("How did we get here?");
          break;
      }
    }
    else {
      SendSMS(mypdu.getSMS(),len);
      delay(1000);
      gsm.flush();
    }
  }
  refNumber++;
}