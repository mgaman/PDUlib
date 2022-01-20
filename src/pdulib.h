/**
 * @file pdulib.h
 * @author David Henry (mgadriver@gmail.com)
 * @brief Encode/Decode PDU data 
 * @version 0.1
 * @date 2021-09-23
 * 
 * @copyright Copyright (c) 2021
 * @
 */


#ifdef PDU_LIB_INCLUDE
#else
//#define PM

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

// UDH bits
#define UDH_EXIST 64

// IEI
#define IEI_CSM_8 0x00
#define IEI_CSM_16 0x08

#define EXT_MASK 0x80   // bit 7
#define TON_MASK 0x70   // bits 4-6
#define TON_OFFSET 4
#define NPI_MASK 0x0f   // bits 0-3

#define MAX_SMS_LENGTH_7BIT 160 // GSM 3.4
#define MAX_NUMBER_LENGTH 20    // gets packed into BCD or packed 7 bit

//SCA (12) + type + mref + address(12) + pid + dcs + length + data(140) -- no valtime
#define PDU_BINARY_MAX_LENGTH 170

 /* Define Non-Printable Characters as a question mark */
#define NPC7    63
#define NPC8    '?'

enum eDCS { ALPHABET_7BIT, ALPHABET_8BIT, ALPHABET_16BIT };
enum eAddressType {INTERNATIONAL_NUMERIC,NATIONAL_NUMERIC,ALPHABETIC};
enum eLengthType {OCTETS,NIBBLES};  // SCA is in octets, sender/recipient nibbles

// wiki: https://en.wikipedia.org/wiki/Concatenated_SMS
struct IED {
  unsigned short number;
  unsigned char total;
  unsigned char part;
};

// wiki: https://en.wikipedia.org/wiki/User_Data_Header
struct UDH {
  unsigned char iei;
  IED ied;
};

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
  //const char *getSCA();
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
   * @brief Get the user data header.
   * 
   * @return const UDH* The user data header.
   */
  const UDH *getUDH();
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
private:
  // following for storing decode fields of incoming messages
  int scalength;
  char scabuff[MAX_NUMBER_LENGTH];
  int addressLength;  // in octets
  char addressBuff[MAX_NUMBER_LENGTH];  // ample for any phone number
  int meslength;
  char mesbuff[MAX_SMS_LENGTH_7BIT];  // actually packed 7 bit is 140
  unsigned char pduType;
  UDH udh;
  int tslength;
  char tsbuff[20];    // big enough for timestamp
  char scanumber[MAX_NUMBER_LENGTH];  // for outgoing SMS
  // following for buiulding an SMS-SUBMIT message - Binary not ASCII
  int addressType;    // GSM 3.04     for building address part of SMS SUBMIT
  int smsOffset;
  char smsSubmit[PDU_BINARY_MAX_LENGTH*2];  // big enough for largest message
  // helper methods
  //bool setMessage(const char *message,eDCS);

  void stringToBCD(const char *number, char *pdu);
  void BCDtoString(char *number, const char *pdu,int length);
  void digitSwap(const char *number, char *pdu);
  
  int utf8_to_packed7bit(const char *utf8, char *pdu);
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
  int utf8_to_ucs2_single(const char *utf8, short *ucs2);  // translate to a single uds2
  int utf8_to_ucs2(const char *utf8, char *ucs2);  // translate an utf8 zero terminated string
  // get length of next utf8
  int utf8Length(const char *);
  int decodeAddress(const char *,char *, eLengthType);  // pdu to readable starts with length octet
  int decodeUDH(const char *);
  bool setAddress(const char *,eAddressType,eLengthType);
//  //  Get SCA number for outgoing SMS
//  const char *getMySCAnumber();
};

/****************************************************************************
This lookup table converts from ISO-8859-1 8-bit ASCII to the
7 bit "default alphabet" as defined in ETSI GSM 03.38

ISO-characters that don't have any corresponding character in the
7-bit alphabet is replaced with the NPC7-character.  If there's
a close match between the ISO-char and a 7-bit character (for example
the letter i with a circumflex and the plain i-character) a substitution
is done. These "close-matches" are marked in the lookup table by
having its value negated.

There are some character (for example the curly brace "}") that must
be converted into a 2 uint8_t 7-bit sequence.  These characters are
marked in the table by having 256 added to its value.
****************************************************************************/
const
#ifdef PM
      PROGMEM
#endif
short lookup_ascii8to7[] = {
  NPC7,       /*     0      null [NUL]                              */
  NPC7,       /*     1      start of heading [SOH]                  */
  NPC7,       /*     2      start of text [STX]                     */
  NPC7,       /*     3      end of text [ETX]                       */
  NPC7,       /*     4      end of transmission [EOT]               */
  NPC7,       /*     5      enquiry [ENQ]                           */
  NPC7,       /*     6      acknowledge [ACK]                       */
  NPC7,       /*     7      bell [BEL]                              */
  NPC7,       /*     8      backspace [BS]                          */
  NPC7,       /*     9      horizontal tab [HT]                     */
  10,         /*    10      line feed [LF]                          */
  NPC7,       /*    11      vertical tab [VT]                       */
  10 + 256,     /*    12      form feed [FF]                          */
  13,         /*    13      carriage return [CR]                    */
  NPC7,       /*    14      shift out [SO]                          */
  NPC7,       /*    15      shift in [SI]                           */
  NPC7,       /*    16      data link escape [DLE]                  */
  NPC7,       /*    17      device control 1 [DC1]                  */
  NPC7,       /*    18      device control 2 [DC2]                  */
  NPC7,       /*    19      device control 3 [DC3]                  */
  NPC7,       /*    20      device control 4 [DC4]                  */
  NPC7,       /*    21      negative acknowledge [NAK]              */
  NPC7,       /*    22      synchronous idle [SYN]                  */
  NPC7,       /*    23      end of trans. block [ETB]               */
  NPC7,       /*    24      cancel [CAN]                            */
  NPC7,       /*    25      end of medium [EM]                      */
  NPC7,       /*    26      substitute [SUB]                        */
  NPC7,       /*    27      escape [ESC]                            */
  NPC7,       /*    28      file separator [FS]                     */
  NPC7,       /*    29      group separator [GS]                    */
  NPC7,       /*    30      record separator [RS]                   */
  NPC7,       /*    31      unit separator [US]                     */
  32,         /*    32      space                                   */
  33,         /*    33    ! exclamation mark                        */
  34,         /*    34    " double quotation mark                   */
  35,         /*    35    # number sign                             */
  2,          /*    36    $ dollar sign                             */
  37,         /*    37    % percent sign                            */
  38,         /*    38    & ampersand                               */
  39,         /*    39    ' apostrophe                              */
  40,         /*    40    ( left parenthesis                        */
  41,         /*    41    ) right parenthesis                       */
  42,         /*    42    * asterisk                                */
  43,         /*    43    + plus sign                               */
  44,         /*    44    , comma                                   */
  45,         /*    45    - hyphen                                  */
  46,         /*    46    . period                                  */
  47,         /*    47    / slash,                                  */
  48,         /*    48    0 digit 0                                 */
  49,         /*    49    1 digit 1                                 */
  50,         /*    50    2 digit 2                                 */
  51,         /*    51    3 digit 3                                 */
  52,         /*    52    4 digit 4                                 */
  53,         /*    53    5 digit 5                                 */
  54,         /*    54    6 digit 6                                 */
  55,         /*    55    7 digit 7                                 */
  56,         /*    56    8 digit 8                                 */
  57,         /*    57    9 digit 9                                 */
  58,         /*    58    : colon                                   */
  59,         /*    59    ; semicolon                               */
  60,         /*    60    < less-than sign                          */
  61,         /*    61    = equal sign                              */
  62,         /*    62    > greater-than sign                       */
  63,         /*    63    ? question mark                           */
  0,          /*    64    @ commercial at sign                      */
  65,         /*    65    A uppercase A                             */
  66,         /*    66    B uppercase B                             */
  67,         /*    67    C uppercase C                             */
  68,         /*    68    D uppercase D                             */
  69,         /*    69    E uppercase E                             */
  70,         /*    70    F uppercase F                             */
  71,         /*    71    G uppercase G                             */
  72,         /*    72    H uppercase H                             */
  73,         /*    73    I uppercase I                             */
  74,         /*    74    J uppercase J                             */
  75,         /*    75    K uppercase K                             */
  76,         /*    76    L uppercase L                             */
  77,         /*    77    M uppercase M                             */
  78,         /*    78    N uppercase N                             */
  79,         /*    79    O uppercase O                             */
  80,         /*    80    P uppercase P                             */
  81,         /*    81    Q uppercase Q                             */
  82,         /*    82    R uppercase R                             */
  83,         /*    83    S uppercase S                             */
  84,         /*    84    T uppercase T                             */
  85,         /*    85    U uppercase U                             */
  86,         /*    86    V uppercase V                             */
  87,         /*    87    W uppercase W                             */
  88,         /*    88    X uppercase X                             */
  89,         /*    89    Y uppercase Y                             */
  90,         /*    90    Z uppercase Z                             */
  60 + 256,     /*    91    [ left square bracket                     */
  47 + 256,     /*    92    \ backslash                               */
  62 + 256,     /*    93    ] right square bracket                    */
  20 + 256,     /*    94    ^ circumflex accent                       */
  17,         /*    95    _ underscore                              */
  -39,        /*    96    ` back apostrophe                         */
  97,         /*    97    a lowercase a                             */
  98,         /*    98    b lowercase b                             */
  99,         /*    99    c lowercase c                             */
  100,        /*   100    d lowercase d                             */
  101,        /*   101    e lowercase e                             */
  102,        /*   102    f lowercase f                             */
  103,        /*   103    g lowercase g                             */
  104,        /*   104    h lowercase h                             */
  105,        /*   105    i lowercase i                             */
  106,        /*   106    j lowercase j                             */
  107,        /*   107    k lowercase k                             */
  108,        /*   108    l lowercase l                             */
  109,        /*   109    m lowercase m                             */
  110,        /*   110    n lowercase n                             */
  111,        /*   111    o lowercase o                             */
  112,        /*   112    p lowercase p                             */
  113,        /*   113    q lowercase q                             */
  114,        /*   114    r lowercase r                             */
  115,        /*   115    s lowercase s                             */
  116,        /*   116    t lowercase t                             */
  117,        /*   117    u lowercase u                             */
  118,        /*   118    v lowercase v                             */
  119,        /*   119    w lowercase w                             */
  120,        /*   120    x lowercase x                             */
  121,        /*   121    y lowercase y                             */
  122,        /*   122    z lowercase z                             */
  40 + 256,     /*   123    { left brace                              */
  64 + 256,     /*   124    | vertical bar                            */
  41 + 256,     /*   125    } right brace                             */
  61 + 256,     /*   126    ~ tilde accent                            */
  NPC7,       /*   127      delete [DEL]                            */
  NPC7,       /*   128                                              */
  NPC7,       /*   129                                              */
  -39,        /*   130      low left rising single quote            */
  -102,       /*   131      lowercase italic f                      */
  -34,        /*   132      low left rising double quote            */
  NPC7,       /*   133      low horizontal ellipsis                 */
  NPC7,       /*   134      dagger mark                             */
  NPC7,       /*   135      double dagger mark                      */
  NPC7,       /*   136      letter modifying circumflex             */
  NPC7,       /*   137      per thousand (mille) sign               */
  -83,        /*   138      uppercase S caron or hacek              */
  -39,        /*   139      left single angle quote mark            */
  -214,       /*   140      uppercase OE ligature                   */
  NPC7,       /*   141                                              */
  NPC7,       /*   142                                              */
  NPC7,       /*   143                                              */
  NPC7,       /*   144                                              */
  -39,        /*   145      left single quotation mark              */
  -39,        /*   146      right single quote mark                 */
  -34,        /*   147      left double quotation mark              */
  -34,        /*   148      right double quote mark                 */
  -42,        /*   149      round filled bullet                     */
  -45,        /*   150      en dash                                 */
  -45,        /*   151      em dash                                 */
  -39,        /*   152      small spacing tilde accent              */
  NPC7,       /*   153      trademark sign                          */
  -115,       /*   154      lowercase s caron or hacek              */
  -39,        /*   155      right single angle quote mark           */
  -111,       /*   156      lowercase oe ligature                   */
  NPC7,       /*   157                                              */
  NPC7,       /*   158                                              */
  -89,        /*   159      uppercase Y dieresis or umlaut          */
  -32,        /*   160      non-breaking space                      */
  64,         /*   161    ¡ inverted exclamation mark               */
  -99,        /*   162    ¢ cent sign                               */
  1,          /*   163    £ pound sterling sign                     */
  36,         /*   164    ? general currency sign                   */
  3,          /*   165    ¥ yen sign                                */
  -33,        /*   166    ¦ broken vertical bar                     */
  95,         /*   167    § section sign                            */
  -34,        /*   168    ¨ spacing dieresis or umlaut              */
  NPC7,       /*   169    © copyright sign                          */
  NPC7,       /*   170    × feminine ordinal indicator              */
  -60,        /*   171    « left (double) angle quote               */
  NPC7,       /*   172    ¬ logical not sign                        */
  -45,        /*   173    ­ soft hyphen                             */
  NPC7,       /*   174    ® registered trademark sign               */
  NPC7,       /*   175    ¯ spacing macron (long) accent            */
  NPC7,       /*   176    ° degree sign                             */
  NPC7,       /*   177    ± plus-or-minus sign                      */
  -50,        /*   178    ² superscript 2                           */
  -51,        /*   179    ³ superscript 3                           */
  -39,        /*   180    ´ spacing acute accent                    */
  -117,       /*   181    µ micro sign                              */
  NPC7,       /*   182    ¶ paragraph sign, pilcrow sign            */
  NPC7,       /*   183    · middle dot, centered dot                */
  NPC7,       /*   184    ¸ spacing cedilla                         */
  -49,        /*   185    ¹ superscript 1                           */
  NPC7,       /*   186    ÷ masculine ordinal indicator             */
  -62,        /*   187    » right (double) angle quote (guillemet)  */
  NPC7,       /*   188    ¼ fraction 1/4                            */
  NPC7,       /*   189    ½ fraction 1/2                            */
  NPC7,       /*   190    ¾ fraction 3/4                            */
  96,         /*   191    ¿ inverted question mark                  */
  -65,        /*   192    ? uppercase A grave                       */
  -65,        /*   193    ? uppercase A acute                       */
  -65,        /*   194    ? uppercase A circumflex                  */
  -65,        /*   195    ? uppercase A tilde                       */
  91,         /*   196    ? uppercase A dieresis or umlaut          */
  14,         /*   197    ? uppercase A ring                        */
  28,         /*   198    ? uppercase AE ligature                   */
  9,          /*   199    ? uppercase C cedilla                     */
  -31,        /*   200    ? uppercase E grave                       */
  31,         /*   201    ? uppercase E acute                       */
  -31,        /*   202    ? uppercase E circumflex                  */
  -31,        /*   203    ? uppercase E dieresis or umlaut          */
  -73,        /*   204    ? uppercase I grave                       */
  -73,        /*   205    ? uppercase I acute                       */
  -73,        /*   206    ? uppercase I circumflex                  */
  -73,        /*   207    ? uppercase I dieresis or umlaut          */
  -68,        /*   208    ? uppercase ETH                           */
  93,         /*   209    ? uppercase N tilde                       */
  -79,        /*   210    ? uppercase O grave                       */
  -79,        /*   211    ? uppercase O acute                       */
  -79,        /*   212    ? uppercase O circumflex                  */
  -79,        /*   213    ? uppercase O tilde                       */
  92,         /*   214    ? uppercase O dieresis or umlaut          */
  -42,        /*   215    ? multiplication sign                     */
  11,         /*   216    ? uppercase O slash                       */
  -85,        /*   217    ? uppercase U grave                       */
  -85,        /*   218    ? uppercase U acute                       */
  -85,        /*   219    ? uppercase U circumflex                  */
  94,         /*   220    ? uppercase U dieresis or umlaut          */
  -89,        /*   221    ? uppercase Y acute                       */
  NPC7,       /*   222    ? uppercase THORN                         */
  30,         /*   223    ? lowercase sharp s, sz ligature          */
  127,        /*   224    ? lowercase a grave                       */
  -97,        /*   225    ? lowercase a acute                       */
  -97,        /*   226    ? lowercase a circumflex                  */
  -97,        /*   227    ? lowercase a tilde                       */
  123,        /*   228    ? lowercase a dieresis or umlaut          */
  15,         /*   229    ? lowercase a ring                        */
  29,         /*   230    ? lowercase ae ligature                   */
  -9,         /*   231    ? lowercase c cedilla                     */
  4,          /*   232    ? lowercase e grave                       */
  5,          /*   233    ? lowercase e acute                       */
  -101,       /*   234    ? lowercase e circumflex                  */
  -101,       /*   235    ? lowercase e dieresis or umlaut          */
  7,          /*   236    ? lowercase i grave                       */
  7,          /*   237    ? lowercase i acute                       */
  -105,       /*   238    ? lowercase i circumflex                  */
  -105,       /*   239    ? lowercase i dieresis or umlaut          */
  NPC7,       /*   240    ? lowercase eth                           */
  125,        /*   241    ? lowercase n tilde                       */
  8,          /*   242    ? lowercase o grave                       */
  -111,       /*   243    ? lowercase o acute                       */
  -111,       /*   244    ? lowercase o circumflex                  */
  -111,       /*   245    ? lowercase o tilde                       */
  124,        /*   246    ? lowercase o dieresis or umlaut          */
  -47,        /*   247    ? division sign                           */
  12,         /*   248    ? lowercase o slash                       */
  6,          /*   249    ? lowercase u grave                       */
  -117,       /*   250    ? lowercase u acute                       */
  -117,       /*   251    ? lowercase u circumflex                  */
  126,        /*   252    ? lowercase u dieresis or umlaut          */
  -121,       /*   253    ? lowercase y acute                       */
  NPC7,       /*   254    ? lowercase thorn                         */
  -121        /*   255    ? lowercase y dieresis or umlaut          */
};

/****************************************************************************
This lookup table converts from the 7 bit "default alphabet" as
defined in ETSI GSM 03.38 to a standard ISO-8859-1 8-bit ASCII.

Some characters in the 7-bit alphabet does not exist in the ISO
character set, they are replaced by the NPC8-character.

If the character is decimal 27 (ESC) the following character have
a special meaning and must be handled separately.
****************************************************************************/

const
#ifdef PM
      PROGMEM
#endif
        unsigned char lookup_ascii7to8[] = {
  64,         /*  0      @  COMMERCIAL AT                           */
  163,        /*  1      £  POUND SIGN                              */
  36,         /*  2      $  DOLLAR SIGN                             */
  165,        /*  3      ¥  YEN SIGN                                */
  232,        /*  4      ?  LATIN SMALL LETTER E WITH GRAVE         */
  233,        /*  5      ?  LATIN SMALL LETTER E WITH ACUTE         */
  249,        /*  6      ?  LATIN SMALL LETTER U WITH GRAVE         */
  236,        /*  7      ?  LATIN SMALL LETTER I WITH GRAVE         */
  242,        /*  8      ?  LATIN SMALL LETTER O WITH GRAVE         */
  199,        /*  9      ?  LATIN CAPITAL LETTER C WITH CEDILLA     */
  10,         /*  10        LINE FEED                               */
  216,        /*  11     ?  LATIN CAPITAL LETTER O WITH STROKE      */
  248,        /*  12     ?  LATIN SMALL LETTER O WITH STROKE        */
  13,         /*  13        CARRIAGE RETURN                         */
  197,        /*  14     ?  LATIN CAPITAL LETTER A WITH RING ABOVE  */
  229,        /*  15     ?  LATIN SMALL LETTER A WITH RING ABOVE    */
  NPC8,       /*  16        GREEK CAPITAL LETTER DELTA              */
  95,         /*  17     _  LOW LINE                                */
  NPC8,       /*  18        GREEK CAPITAL LETTER PHI                */
  NPC8,       /*  19        GREEK CAPITAL LETTER GAMMA              */
  NPC8,       /*  20        GREEK CAPITAL LETTER LAMBDA             */
  NPC8,       /*  21        GREEK CAPITAL LETTER OMEGA              */
  NPC8,       /*  22        GREEK CAPITAL LETTER PI                 */
  NPC8,       /*  23        GREEK CAPITAL LETTER PSI                */
  NPC8,       /*  24        GREEK CAPITAL LETTER SIGMA              */
  NPC8,       /*  25        GREEK CAPITAL LETTER THETA              */
  NPC8,       /*  26        GREEK CAPITAL LETTER XI                 */
  27,         /*  27        ESCAPE TO EXTENSION TABLE               */
  198,        /*  28     ?  LATIN CAPITAL LETTER AE                 */
  230,        /*  29     ?  LATIN SMALL LETTER AE                   */
  223,        /*  30     ?  LATIN SMALL LETTER SHARP S (German)     */
  201,        /*  31     ?  LATIN CAPITAL LETTER E WITH ACUTE       */
  32,         /*  32        SPACE                                   */
  33,         /*  33     !  EXCLAMATION MARK                        */
  34,         /*  34     "  QUOTATION MARK                          */
  35,         /*  35     #  NUMBER SIGN                             */
  164,        /*  36     ?  CURRENCY SIGN                           */
  37,         /*  37     %  PERCENT SIGN                            */
  38,         /*  38     &  AMPERSAND                               */
  39,         /*  39     '  APOSTROPHE                              */
  40,         /*  40     (  LEFT PARENTHESIS                        */
  41,         /*  41     )  RIGHT PARENTHESIS                       */
  42,         /*  42     *  ASTERISK                                */
  43,         /*  43     +  PLUS SIGN                               */
  44,         /*  44     ,  COMMA                                   */
  45,         /*  45     -  HYPHEN-MINUS                            */
  46,         /*  46     .  FULL STOP                               */
  47,         /*  47     /  SOLIDUS (SLASH)                         */
  48,         /*  48     0  DIGIT ZERO                              */
  49,         /*  49     1  DIGIT ONE                               */
  50,         /*  50     2  DIGIT TWO                               */
  51,         /*  51     3  DIGIT THREE                             */
  52,         /*  52     4  DIGIT FOUR                              */
  53,         /*  53     5  DIGIT FIVE                              */
  54,         /*  54     6  DIGIT SIX                               */
  55,         /*  55     7  DIGIT SEVEN                             */
  56,         /*  56     8  DIGIT EIGHT                             */
  57,         /*  57     9  DIGIT NINE                              */
  58,         /*  58     :  COLON                                   */
  59,         /*  59     ;  SEMICOLON                               */
  60,         /*  60     <  LESS-THAN SIGN                          */
  61,         /*  61     =  EQUALS SIGN                             */
  62,         /*  62     >  GREATER-THAN SIGN                       */
  63,         /*  63     ?  QUESTION MARK                           */
  161,        /*  64     ¡  INVERTED EXCLAMATION MARK               */
  65,         /*  65     A  LATIN CAPITAL LETTER A                  */
  66,         /*  66     B  LATIN CAPITAL LETTER B                  */
  67,         /*  67     C  LATIN CAPITAL LETTER C                  */
  68,         /*  68     D  LATIN CAPITAL LETTER D                  */
  69,         /*  69     E  LATIN CAPITAL LETTER E                  */
  70,         /*  70     F  LATIN CAPITAL LETTER F                  */
  71,         /*  71     G  LATIN CAPITAL LETTER G                  */
  72,         /*  72     H  LATIN CAPITAL LETTER H                  */
  73,         /*  73     I  LATIN CAPITAL LETTER I                  */
  74,         /*  74     J  LATIN CAPITAL LETTER J                  */
  75,         /*  75     K  LATIN CAPITAL LETTER K                  */
  76,         /*  76     L  LATIN CAPITAL LETTER L                  */
  77,         /*  77     M  LATIN CAPITAL LETTER M                  */
  78,         /*  78     N  LATIN CAPITAL LETTER N                  */
  79,         /*  79     O  LATIN CAPITAL LETTER O                  */
  80,         /*  80     P  LATIN CAPITAL LETTER P                  */
  81,         /*  81     Q  LATIN CAPITAL LETTER Q                  */
  82,         /*  82     R  LATIN CAPITAL LETTER R                  */
  83,         /*  83     S  LATIN CAPITAL LETTER S                  */
  84,         /*  84     T  LATIN CAPITAL LETTER T                  */
  85,         /*  85     U  LATIN CAPITAL LETTER U                  */
  86,         /*  86     V  LATIN CAPITAL LETTER V                  */
  87,         /*  87     W  LATIN CAPITAL LETTER W                  */
  88,         /*  88     X  LATIN CAPITAL LETTER X                  */
  89,         /*  89     Y  LATIN CAPITAL LETTER Y                  */
  90,         /*  90     Z  LATIN CAPITAL LETTER Z                  */
  196,        /*  91     ?  LATIN CAPITAL LETTER A WITH DIAERESIS   */
  214,        /*  92     ?  LATIN CAPITAL LETTER O WITH DIAERESIS   */
  209,        /*  93     ?  LATIN CAPITAL LETTER N WITH TILDE       */
  220,        /*  94     ?  LATIN CAPITAL LETTER U WITH DIAERESIS   */
  167,        /*  95     §  SECTION SIGN                            */
  191,        /*  96     ¿  INVERTED QUESTION MARK                  */
  97,         /*  97     a  LATIN SMALL LETTER A                    */
  98,         /*  98     b  LATIN SMALL LETTER B                    */
  99,         /*  99     c  LATIN SMALL LETTER C                    */
  100,        /*  100    d  LATIN SMALL LETTER D                    */
  101,        /*  101    e  LATIN SMALL LETTER E                    */
  102,        /*  102    f  LATIN SMALL LETTER F                    */
  103,        /*  103    g  LATIN SMALL LETTER G                    */
  104,        /*  104    h  LATIN SMALL LETTER H                    */
  105,        /*  105    i  LATIN SMALL LETTER I                    */
  106,        /*  106    j  LATIN SMALL LETTER J                    */
  107,        /*  107    k  LATIN SMALL LETTER K                    */
  108,        /*  108    l  LATIN SMALL LETTER L                    */
  109,        /*  109    m  LATIN SMALL LETTER M                    */
  110,        /*  110    n  LATIN SMALL LETTER N                    */
  111,        /*  111    o  LATIN SMALL LETTER O                    */
  112,        /*  112    p  LATIN SMALL LETTER P                    */
  113,        /*  113    q  LATIN SMALL LETTER Q                    */
  114,        /*  114    r  LATIN SMALL LETTER R                    */
  115,        /*  115    s  LATIN SMALL LETTER S                    */
  116,        /*  116    t  LATIN SMALL LETTER T                    */
  117,        /*  117    u  LATIN SMALL LETTER U                    */
  118,        /*  118    v  LATIN SMALL LETTER V                    */
  119,        /*  119    w  LATIN SMALL LETTER W                    */
  120,        /*  120    x  LATIN SMALL LETTER X                    */
  121,        /*  121    y  LATIN SMALL LETTER Y                    */
  122,        /*  122    z  LATIN SMALL LETTER Z                    */
  228,        /*  123    ?  LATIN SMALL LETTER A WITH DIAERESIS     */
  246,        /*  124    ?  LATIN SMALL LETTER O WITH DIAERESIS     */
  241,        /*  125    ?  LATIN SMALL LETTER N WITH TILDE         */
  252,        /*  126    ?  LATIN SMALL LETTER U WITH DIAERESIS     */
  224         /*  127    ?  LATIN SMALL LETTER A WITH GRAVE         */

        /*  The double bytes below must be handled separately after the
        table lookup.

        12             27 10      FORM FEED
        94             27 20   ^  CIRCUMFLEX ACCENT
        123            27 40   {  LEFT CURLY BRACKET
        125            27 41   }  RIGHT CURLY BRACKET
        92             27 47   \  REVERSE SOLIDUS (BACKSLASH)
        91             27 60   [  LEFT SQUARE BRACKET
        126            27 61   ~  TILDE
        93             27 62   ]  RIGHT SQUARE BRACKET
        124            27 64   |  VERTICAL BAR                             */

};
#endif
