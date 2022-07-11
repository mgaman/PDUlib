#include <Arduino.h>
#include <pdulib.h>

// adjust BUFFER_SIZE until the overflow message goes away
#define BUFFER_SIZE 100
PDU mypdu = PDU(BUFFER_SIZE);
// Not enough RAM to do all examples in one go, so just uncomment one of the following
#define PART1
//#define PART2
//#define PART3
//#define PART4
//#define PART5
char const *inpdu[] = {

#ifdef PART1
  "07917952140230F2040C917952777777770008120170016131212200680065006C006C006F003000A505D02660D83CDCA1D83DDE0005E905DC05D505DD",
  "07917952140230F2040C9179527777777700001201216123732106CA405B8D6000", //    GSM 7 bit
  "07917952939899F9240C917952777777770000120151113404210A814D79C3DBF8C2E231",  // includes escape euro
  "07917952140230F2040C91795277777777000812012161238121180061006200630064D83CDF56D83DDE0305D005D105D205D3",  // hebrew emojis
  "07917952140230F2040C91795277777777000012012161335221A061F1985C369FD169F59ADD76BFE171F99C5EB7DFF1797D503824168D476452B964369D4F68543AA556AD576C561B168FC965F3199D56AFD96DF71B1E97CFE975FB1D9FD707854362D1784426954B66D3F98446A5536AD57AC566B561F1985C369FD169F59ADD76BFE171F99C5EB7DFF1797D503824168D476452B964369D4F68543AA556AD576C561B93CD68", // full length
#elif defined(PART2)
   // greek in GSM 7 bit
  "06910379010023040DD05774983EAFC20100002230603023928046456C318901D960B013C8027D62404FCA13447D3A41CB27A6F904519F59D01519A49EA6A02A146479664114E4E4997582A8482708167BC168A0984C078301", 
  "06910379010023040DD05774983EAFC20100002230604054558046456C318901D960B013C8027D62404FCA13447D3A41CB27A6F904519F59D01519A49EA6A02A146479664114E4E4997582A8482708167BC168A0984C078301", 
  "06910379010023040DD05774983EAFC20100002230608044508047456C31890161B2CE2725B90C8262B09BED04B2409F18D0931204518320487249CD064196671644414E9E5927888A748262B5176CF692C16432504CA683C100", 
  "06910379010023040DD05774983EAFC2010000223060804401803C456C318901C564301868DA9C822CD0270654CC3893CB2008925C52B34190E5990551909367D609A2229DA058ED059B8164341D0C06",
  "06910379010023040DD05774983EAFC20100002230608044518049456C318901C570B51808528116834E5053089A2483200BE5899C2031481086FA04259DD4A2D459A4829A452C3409A226312098EC05A3BD6430990C04B3E96630"
#elif defined(PART3)
  /* 7 bit concatenated */
    "07917952140280F2440C91795211111111000022408201643121A00500035D030140C170381C0E87C3E170301C0E87C3E17038AC0886C3E170381C0E8783E170381C0E87C3E160381C0E87C38A60381C0E87C3E17038180E87C3E170381C0605C3E170381C0E8715C170381C0E87C3E170301C0E87C3E17038AC0886C3E170381C0E8783E170381C0E87C3E160381C0E87C38A60381C0E87C3E17038180E87C3E170381C0605C3",
    "07917952140230F2440C91795211111111000022408201643121300500035D0303C2E170381C0E07C3E170381C0E87C3A060381C0E87C3E1B04228168BC56231482C168B15",
    "07917952140280F1440C91795211111111000022408201643121A00500035D0302C2E170381C0E2B8462B1582C1683C462B158110C87C3E170381C0E07C3E170381C0E87C38A60381C0E87C3E17038180E87C3E170381C0E86C3E17038AC0886C3E170381C0E8783E170381C0E87C36150301C0E87C3E17058110C87C3E170381C0E07C3E170381C0E87C38A60381C0E87C3E17038180E87C3E170381C0E86C3E17038AC0886C3",
#elif defined(PART4)
    /* 16 bit concatenated */
  "07917952140230F2440C917952111111110008224031013445218C05000361080105D005DC002005EA05D205D905D305D5002005DC05D0002005D905D305E205E005D5002005DC05D0002005E905DE05E205E005D5000A05DE05D105E605E205D9002005D705D2002005D405D005D105D905D1002005D105DC05D905E705E8002005DE05E805E705D800200078006F000A05DE05D705DB05D905DD002005DC05DB05DD002005DC",
  "07917952140230F2440C917952111111110008224031013455218C05000361080205E705E805D005EA002005D405E405E105D7002005D105DB05DC002005D405DE05D705DC05E705D505EA002005E905DC05E005D5000A05D105DC05D905E705E8002005DE05E805E705D80020006200690067002005E705E805D905D505EA000A05E605DC05E605DC05D5002005D005DC05D905E005D5003A00300034002D0038003700320034",
  "07917952140230F2440C917952111111110008224031013455218C050003610803003300330033000A05DE05D105D705E8002005DE05D105E605E205D905DD002005DC05D705D105E805D9002005DE05D505E205D305D505DF000A05E805DE05EA002005D705D105E805D505DF002005D005E805DE05D205D305D505DF0020003400330039000A05E405D905D905D1002005E105D805D505E005E1002005D505D905E805D805D5",
#elif defined(PART5)
    /*  MMS should be rejected*/
  "07917952939899F94406D1CDE61400F522507011138121750B05040B8423F000031902010006291F226170706C69636174696F6E2F766E642E7761702E6D6D732D6D657373616765008184AF848D01008C8298317469643937323533323235343538315F307337717365008D918919802B3937323534353931393838362F545950453D504C4D4E00964D4D5300",
  "07917952939899F94406D1CDE61400F522507011130221680B05040B8423F0000319020286818A808E030115FF8805810303F48083687474703A2F2F6D6D732E686F746D6F62696C652E636F2E696C2F6D6D732F776170656E633F6C6F636174696F6E3D3937323533323235343538315F307337717365267269643D30393600"
#endif
};
void setup() {
  Serial.begin(9600);
#ifdef PM
  Serial.println("Using PM");
#else
  Serial.println("Not using PM");
#endif
  Serial.print("Buffer Size "); Serial.println(BUFFER_SIZE);

  for (unsigned long i=0; i< sizeof(inpdu)/sizeof(const char *); i++) {
    if (mypdu.decodePDU(inpdu[i])) {
      Serial.println("-------------------------------");
      if (mypdu.getOverflow()) {
        Serial.println("Buffer Overflow, partial message only");
      }
      Serial.print("SCA ");Serial.println(mypdu.getSCAnumber());
      Serial.print("Sender ");Serial.println(mypdu.getSender());
      Serial.print("Timestamp ");Serial.println(mypdu.getTimeStamp());
      Serial.println(mypdu.getText());
      // check if concatenated
      int *concat = mypdu.getConcatInfo();
      if (concat[0] != 0) {
        Serial.print("CSMS ");Serial.print(concat[0]);
        Serial.print(" Part ");Serial.print(concat[1]);Serial.print(" of ");Serial.println(concat[2]);
      }
      else
        Serial.println("Not concatenated");
    }
    else
    {
      Serial.println("Decode failed");
//      Serial.print("Sender ");Serial.println(mypdu.getSender());
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}