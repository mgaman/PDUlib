/**
 * @file pdulib.h
 * @author David Henry (mgadriver@gmail.com)
 * @brief Encode/Decode PDU data 
 * @version 0.1.2
 * @date 2021-09-23
 * 
 * @copyright Copyright (c) 2021
 * 
 * Release History
 * 0.1.1 Original release
 * 0.5.1 Fixed bug where Greek characters in a GSM7 message were ignored
 *       Fixed bad behaviour where static lookup tables were duplicated (moved to pdulib.cpp)
 *       Default GSM7 now fully supported
 */


#ifdef PDU_LIB_INCLUDE
#else

#define PDU_LIB_INCLUDE
#define BITMASK_7BITS 0x7F

// DCS bit masks
#define DCS_COMPRESSED (5<<1)
#define DCS_CLASS_MEANING (4<<1)
#define DCS_ALPHABET_MASK (3<<2)
#define DCS_ALPHABET_OFFSET 2
#define DCS_7BIT_ALPHABET_MASK  0B0000 //(0<<2)
#define DCS_8BIT_ALPHABET_MASK  0B0100 //1<<2)
#define DCS_16BIT_ALPHABET_MASK 0B1000 //(2<<2)
#define DCS_CLASS_MASK 3
#define DCS_IMMEDIATE_DISPLAY 3
#define DCS_ME_SPECIFIC_MASK 1
#define DCS_SIM_SPECIFIC_MASK 2
#define DCS_TE_SPECIFIC_MASK 3

 // PDU bits
#define PDU_VALIDITY_MASK_OFFSET 3
#define PDU_VALIDITY_NOT_PRESENT 0
#define PDU_VALIDITY_PRESENT_RELATIVE 2
#define PDU_VALIDITY_PRESENT_ENHANCED 1
#define PDU_VALIDITY_PRESENT_ABSOLUTE 3
#define PSU_SMS_DELIVER 0
#define PSU_SMS_SUBMIT  1

// type of address
#define INTERNATIONAL_NUMBER 0x91
#define NATIONAL_NUMBER 0xA1

#define EXT_MASK 0x80   // bit 7
#define TON_MASK 0x70   // bits 4-6
#define TON_OFFSET 4
#define NPI_MASK 0x0f   // bits 0-3

#define MAX_SMS_LENGTH_7BIT 160 // GSM 3.4
#define MAX_NUMBER_OCTETS 140
#define MAX_NUMBER_LENGTH 20    // gets packed into BCD or packed 7 bit

//SCA (12) + type + mref + address(12) + pid + dcs + length + data(140) -- no valtime
#define PDU_BINARY_MAX_LENGTH 170

 /* Define Non-Printable Characters as a question mark */
#define NPC7    63
#define NPC8    '?'

#define EURO_UCS 0x20AC
//#define SURROGATE_PAIR 0xD800


enum eDCS { ALPHABET_7BIT, ALPHABET_8BIT, ALPHABET_16BIT };
enum eAddressType {INTERNATIONAL_NUMERIC,NATIONAL_NUMERIC,ALPHABETIC};
enum eLengthType {OCTETS,NIBBLES};  // SCA is in octets, sender/recipient nibbles
/**
 * @brief PDU class, provides methods to decode a PDU message or encode a new one
 * @param None There are no parameters for the constructor
 * 
 */
class PDU
{
public:
  PDU();
  ~PDU();
/**
 * @brief Encode a PDU block for sending to an GSM modem
 * 
 * @param recipient Phone number, must be numeric, no whitespace. International numbers prefixed by '+'
 * @param message The message in UTF-8 format
 * @return int The length of the message, need for the GSM command <b>AT+CSMG=nn</b>
 */
  int encodePDU(const char *recipient,const char *message);

/**
   * @brief Get the address of the PDU message created by <b>encodePDU</b>
   * 
   * @return const char* The pointer to the message. It already contained the CTRL/Z delimiter byte.
   */
  const char *getSMS();

/**
 * @brief Before encoding a PDU, you must supply the SCA phone number.
 * Typically this can be retrieved from a GSM modem with the AT+CSCA? command.
 * 
 * @param number The number as retrieved from the AT+CSCA? command
 * 
 */
  void setSCAnumber(const char *number);

  /**
   * @brief Decode a PDU, typically received from a GSM modem when in PDU mode.
   * After a successful decoding you can retrieve the components parts, described below.
   * 
   * @param pdu A pointer to the PDU
   * @return true If the decoding succeeded.
   * @return false If the decoding did not succeed.
   */
  bool decodePDU(const char *pdu);

  /**
   * @brief Get the SCA number from a decoded PDU
   * 
   * @return char* Pointer to the number
   */
  const char *getSCAnumber();

  /**
   * @brief Get the senders phone number from a decoded PDU
   * 
   * @return const char* Pointer to the number
   */
  const char *getSender();

  /**
   * @brief Get the Timestamp from a decoded PDU
   * 
   * @return const char* The tomestamp formatted as YYYMMDDHHMMSS
   */
  const char *getTimeStamp();

  /**
   * @brief Get the text froma a decoded PDU.
   * 
   * @return const unsigned char* The message in UTF-8 format.
   */
  const char *getText();

  /**
   * @brief Create a UTF16 string from a codepoint > 0xffff
   * 
   * @param codepoint Examples https://unicode.org/emoji/charts/full-emoji-list.html
   * @param target Where to place the string
   */
  void buildUtf16(unsigned long codepoint, char *target); // build a string from a codepoint

  /**
   * @brief Create a UTF string from a codepoint. Handles practically anything
   * 
   * @param codepoint Examples https://en.wikipedia.org/wiki/List_of_Unicode_characters
   * @param target Where to place the string
   */
  int buildUtf(unsigned long codepoint, char *target); // build a string from a codepoint

  /**
   * @brief Convert a UTF8 string into an array of UCS2 octets
   * 
   * @param utf8 The UTF8 string
   * @param ucs2 Pointer to UCS2 array
   * @return int Return the number of octets, -1 if the message is greater than the maximum allowed
   */
  int utf8_to_ucs2(const char *utf8, char *ucs2);  // translate an utf8 zero terminated string 

  /**
   * @brief Examine an array of UCS2 to determine if this is a default GSM7 character
   * 
   * @param pucs A UCS2 array e.g. as created by the utf8_to_ucs2_single method
   * @return true or false
   */
  bool isGSM7(unsigned short *pucs);  // UCS may be a surrogate pair

  /**
   * @brief Examine a UTF8 encoded Unicode character
   * 
   * @return The number of bytes (octets) occupied by the character
   */
  int utf8Length(const char *);

  /**
   * @brief Encode a single UTF8 encoded Unicode character into UCS2 format. 
   * 
   * @param utf8 Pointer to an UTF8 stream
   * @param pucs2 An array of unsigned short to receive the UCS2 data. Surrogate pair characters need an array size of 2.
   * @return int The length (in bytes) of the converted character. This will be 4 for a Surrogate Pair (e.g an Emoji) else 2.
   */
  int utf8_to_ucs2_single(const char *utf8, unsigned short *pucs2);  // translate to a single ucs2
private:
  // following for storing decode fields of incoming messages
  int scalength;
  char scabuff[MAX_NUMBER_LENGTH];
  int addressLength;  // in octets
  char addressBuff[MAX_NUMBER_LENGTH];  // ample for any phone number
  int meslength;
  char mesbuff[MAX_SMS_LENGTH_7BIT];  // actually packed 7 bit is 140
  int tslength;
  char tsbuff[20];    // big enough for timestamp
  char scanumber[MAX_NUMBER_LENGTH];  // for outgoing SMS
  // following for buiulding an SMS-SUBMIT message - Binary not ASCII
  int addressType;    // GSM 3.04     for building address part of SMS SUBMIT
  int smsOffset;
  char smsSubmit[PDU_BINARY_MAX_LENGTH*2];  // big enough for largest message
  // helper methods

  void stringToBCD(const char *number, char *pdu);
  void BCDtoString(char *number, const char *pdu,int length);
  void digitSwap(const char *number, char *pdu);
  
  int utf8_to_packed7bit(const char *utf8, char *pdu, int *septets);
  int pdu_to_ascii(const char *pdu, int pdulength, char *ascii);

  int convert_utf8_to_gsm7bit(const char *ascii, char *a7bit);
  int convert_7bit_to_ascii(unsigned char *a7bit, int length, char *ascii);

  unsigned char gethex(const char *pc);
  void putHex(unsigned char b, char *target);
  // return number of ucs2 octets in output array
  int pdu_to_ucs2(const char *pdu, int length, unsigned short *ucs2);
  // callers responsibilty that utf8 array is big enough
  int ucs2_to_utf8(unsigned short ucs2, char *utf8);
  // callers responsibilty that ucs2 array is big enough
 // int utf8_to_ucs2_single(const char *utf8, unsigned short *ucs2);  // translate to a single ucs2
//  int utf8_to_ucs2(const char *utf8, char *ucs2);  // translate an utf8 zero terminated string
  // get length of next utf8
//  int utf8Length(const char *);
  int decodeAddress(const char *,char *, eLengthType);  // pdu to readable starts with length octet
  bool setAddress(const char *,eAddressType,eLengthType);
//  bool isGSM7(unsigned short *pucs);  // UCS may be a surrogate pair

//  //  Get SCA number for outgoing SMS
//  const char *getMySCAnumber();
};

extern const
#ifdef PM
      PROGMEM
#endif
              short lookup_ascii8to7[];

extern const
#ifdef PM
      PROGMEM
#endif
              unsigned char lookup_ascii7to8[];

extern const
#ifdef PM
      PROGMEM
#endif
              unsigned short lookup_Greek7ToUnicode[];
#endif

#define GREEK_UCS_MINIMUM 0x393
extern const
#ifdef PM
      PROGMEM
#endif
        unsigned short lookup_UnicodeToGreek7[];

struct sRange {
  unsigned short min,max;
};
extern const
#ifdef PM
      PROGMEM
#endif
        struct sRange gsm7_legal[];
