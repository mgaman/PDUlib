#include <Arduino.h>
#define NPC7 63
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

char buffer[50];

void setup() {
  Serial.begin(9600);
  int source = 252;
#ifdef PM
  Serial.println("Using PM");
  short x = pgm_read_word(lookup_ascii8to7 + source);
#else
  Serial.println("Not using PM");
  short x = lookup_ascii8to7[source];
#endif
  sprintf(buffer,"%d %d",source,x);
  Serial.println(buffer);
}

void loop() {
  // put your main code here, to run repeatedly:
}
