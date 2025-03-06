/**
 * @file pdulib.cpp
 * @author David Henry (mgadriver@gmail.com)
 * @brief A general purpose libray for encoding/decoding PDU data for GSM modems
 * @version 0.5.5
 * @date 2022-07-11
 *
 * @copyright Copyright (c) 2021
 *
 * Release History
 * 0.1.1 Original release
 * 0.2.1 Fix odd length Sender Address bug
 *       Add support for emojis
 * 0.2.2 Added correct surrogate pair handling for UTF16
 * 0.3.3 Fixed getSCAnumber bug
 *       Added buildUtf16 helper
 * 0.4.6 Make source tree Arduino/PlatformIO compatible (no source changes)
 * 0.4.7 Fixed issue with PM macro/Arduino
 * 0.5.1 Add sanity checks to Encode and helpers to avoid overflow of PDU buffer
         Implement fully GSM 7 encoding
         Fix GSM 7 decode bug that ignored Greek letters, issue #11
         Avoid duplication of static translation tables
  * 0.5.4 Fix uninitiated data warning
          Fix switch case fallthrough warning
          Fix issue #13 CR/LF missing from GSM7 default alphabet
          Fix issue #16 bug, alphanumeric origin address, translated too many characters
          Issue #14 Add handling for User Data Header/Concatenated messages
  * 0.5.5 Change default compilation model from Desktop to Arduino. Necessitated by
        confusion as how to manipulate Arduino IDE preferences
        Reduce RAM by replacing 2 static buffers by 1 user defined buffer. Default size 100 bytes can be over-ridden
        in the PDU constructor.
        Add the getOverflow method
  * 0.5.7 Fix issues #27, #28, #30, #32, #33
 */

#ifndef DESKTOP_PDU
#include <Arduino.h>
#else
#include <math.h>
#include <string.h>
#endif
#include <ctype.h>
#include <pdulib.h>
#include <stdint.h>

PDU::PDU(int worksize ) {
  generalWorkBuff.allocate(worksize+2); // dynamically allocate buffer
}
PDU::~PDU() {
}

// array must be defined before its use in sizeof statement
const
#ifdef PM
    PROGMEM
#endif
    struct sRange gsm7_legal[] = {
        {10, 10}, {13, 13}, {32, 95}, {97, 126}, {161, 161}, {163, 167}, {191, 191}, {196, 201}, {209, 209}, {214, 214}, {216, 216}, {220, 220}, {223, 224}, {228, 230}, {232, 233}, {236, 236}, {241, 242}, {246, 246}, {248, 249}, {252, 252}, {915, 916}, {920, 920}, {923, 923}, {926, 926}, {928, 928}, {931, 931}, {934, 934}, {936, 937}, // greek
        {8364, 8364}                                                                                                                                                                                                                                                                                                                             // euro
};

/*
   Sanity check on phone number, only numeric or +
*/
bool PDU::phoneNumberLegal(const char *number) const {
  bool rc = true;
  int j = strlen(number);
  int i =0;
  while (i++ < j) {
    char c = *number++;
    if (!(isdigit(c) || c == '+')) {
      rc = false;
      break;
    }
  }
  return rc;
}

/*
  Save recipient phone number, check that it is numeric
  return true if valid
  Save in generalWorkBuff
  byte 0 length in nibbles
*/
bool PDU::setAddress(const char *address, /*eAddressType at,*/ eLengthType lt, char *buffer)
{
  // sanity check on number format numeric and optional +
  bool rc = phoneNumberLegal(address);
  if (rc) {
    eAddressType at = NATIONAL_NUMERIC;
    if (*address == '+') {
      address++; // ignore leading +
      at = INTERNATIONAL_NUMERIC;
    }
    addressLength = strlen(address);
    if (addressLength < MAX_NUMBER_LENGTH)
    {
      if (lt == NIBBLES)
        buffer[smsOffset++] = addressLength;
      else
        buffer[smsOffset++] = ((addressLength + 1) / 2) + 1; // add 1 for length
      switch (at)
      {
        case INTERNATIONAL_NUMERIC:
          buffer[smsOffset++] = INTERNATIONAL_NUMBER;
          stringToBCD(address, &buffer[smsOffset]);
          smsOffset += (strlen(address) + 1) / 2;
          rc = true;
          break;
        case NATIONAL_NUMERIC:
          buffer[smsOffset++] = NATIONAL_NUMBER;
          stringToBCD(address, &buffer[smsOffset]);
          smsOffset += (strlen(address) + 1) / 2;
          rc = true;
          break;
        default:
          ;
      }
    }
    else
      rc = false;
  }
  return rc;
}

// convert 2 printable digits to 1 BCD byte
void PDU::stringToBCD(const char *number, char *pdu)
{
  int j, targetindex = 0;
  if (*number == '+') // ignore leading +
    number++;
  for (j = 0; j < addressLength; j++)
  {
    if ((j & 1) == 1) // odd, upper
    {
      pdu[targetindex] &= 0x0f;
      pdu[targetindex] += (*number++ - '0') << 4;
      targetindex++;
    }
    else
    {
      // prime in case this is the last byte
      pdu[targetindex] = 0xf0;
      //      pdu[targetindex] &= 0xf0;  // clear lower
      pdu[targetindex] += *number++ - '0';
    }
  }
}

void PDU::digitSwap(const char *number, char *pdu)
{
  int j, targetindex = 0;
  if (*number == '+') // ignore leading +
    number++;
  for (j = 0; j < addressLength; j++)
  {
    if ((j & 1) == 1) // odd, upper
    {
      pdu[targetindex] = *number++;
      targetindex += 2;
    }
    else
    { // even lower
      pdu[targetindex + 1] = *number++;
    }
  }
  if ((addressLength & 1) == 1)
  {
    pdu[targetindex] = 'F';
    targetindex += 2;
  }
  pdu[targetindex++] = 0;
}

/*
    Input is ISO-8859 8 bit ASCII, 0 to 255  -- WRONG
    Input is UTF8

    Need to know UDH size in septets (0 or 8) to calculate when
    message is too long
    Return GSM7_TOO_LONG is message is longer than MAX_SMS_LENGTH_7BIT
*/
int PDU::convert_utf8_to_gsm7bit(const char *message, char *gsm7bit, int udhsize, int bufferSize)
{

  int w = 0;

  while (*message)
  {
    // sanity check against overflow
    int length = utf8Length(message);
    unsigned short ucs2[2], target; // allow for surrogate pair
    utf8_to_ucs2_single(message, ucs2);
    target = (ucs2[0] << 8) | ((ucs2[0] & 0xff00) >> 8); // swap endian
    /*
      Handle special cases, code a bit convoluted caused by the need to
      keep translate tables as small as possible
    */
    if (target == 0x20AC)
    { // euro ucs2
      *gsm7bit++ = 27;
      *gsm7bit++ = 0x65;
      w += 2;
    }
    else if (target >= GREEK_UCS_MINIMUM)
    {
#ifdef PM
      *gsm7bit++ = pgm_read_word(lookup_UnicodeToGreek7 + target - GREEK_UCS_MINIMUM);
#else
      *gsm7bit++ = lookup_UnicodeToGreek7[target - GREEK_UCS_MINIMUM];
#endif
      w++;
    }
    else
    {
#ifdef PM
      short x = pgm_read_word(lookup_ascii8to7 + target);
#else
      short x = lookup_ascii8to7[target];
#endif
      //     Serial.print(target);Serial.print(" ");Serial.println(x);
      if (x > 256)
      { // this is an escaped character
        *gsm7bit++ = 27;
        *gsm7bit++ = x - 256;
        w += 2;
      }
      else
      {
        *gsm7bit++ = x;
        w++;
      }
    }
    message += length; // bump to next character
    // check not exceeding max 160 characters for GSM7
  //  if (w > bufferSize - udhsize)
    //  break;
    if (w > MAX_SMS_LENGTH_7BIT)
      break;
  }
//  return w > (bufferSize - udhsize) ? WORK_BUFFER_TOO_SMALL : w;
  return w > MAX_SMS_LENGTH_7BIT ? GSM7_TOO_LONG : w;
}
/*
    UTF8 string may contain characters that need to be changed from 8 bit ISO-8859
    to GSM 7 bit e.g. Pound Sterling from 0xA3 to 0x01 or escaped characters e.g.
    Left Square 0x5B to ESC/0x3C, Euro 0x20AC to ESC/0x65
    Special treatment for Greek

    Return GSM7_TOO_LONG is the message is longer than MAX_SMS_LENGTH_7BIT
*/
int PDU::utf8_to_packed7bit(const char *utf8, char *pdu, int *septets, int udhsize, int bufferSize)
{
  int r;
  int w;
  int len7bit;
  char gsm7bit[MAX_SMS_LENGTH_7BIT + 2]; // allow for overflow

  /* Start by converting the UTF8-string to a GSM7 string */
  len7bit = convert_utf8_to_gsm7bit(utf8, gsm7bit, udhsize, bufferSize);
  // check if workbuffer exceeded
  if (len7bit < 0)
    return len7bit;

  /* Now, we can create a PDU string by packing the 7bit-string */
  r = 0;
  w = 0;
  while (r < len7bit)
  {
    pdu[w] = ((gsm7bit[r] >> (w % 7)) & 0x7F) | ((gsm7bit[r + 1] << (7 - (w % 7))) & 0xFF);
    if ((w % 7) == 6)
      r++;
    r++;
    w++;
  }
  *septets = len7bit;
  return w;
}

int PDU::buildUDH(unsigned short csms, unsigned numparts, unsigned partnumber)
{
  int offset = 0;
  // Issue 36  extra padding byte prepended to all messages
  // caused by following check that tries to be clever and save a byte
  // temporary fix, cancel the check
#if 0
  if (csms <= 255)
  {
    udhbuffer[offset++] = 5; // length
    udhbuffer[offset++] = 0; // IEI
    udhbuffer[offset++] = 3; // len IEL
    udhbuffer[offset++] = csms;
  }
  else
  {
#endif
    udhbuffer[offset++] = 6;           // length
    udhbuffer[offset++] = 8;           // IEI
    udhbuffer[offset++] = 4;           // len IEL
    udhbuffer[offset++] = csms >> 8;   // hi byte
    udhbuffer[offset++] = csms & 0xff; // lo byte
#if 0
  }
#endif
  udhbuffer[offset++] = numparts;
  udhbuffer[offset++] = partnumber;
  return offset;
}

/* creates an buffer in SMS SUBMIT format and returns length, < 0 if invalid in anyway
    https://bluesecblog.wordpress.com/2016/11/16/sms-submit-tpdu-structure/
*/
int PDU::encodePDU(const char *recipient, const char *message, unsigned short csms, unsigned char numparts, unsigned char partnumber)
{
  int length = -1;
  int delta = -1;
  char tempbuf[PDU_BINARY_MAX_LENGTH];
  smsOffset = 0;
  int beginning = 0;
 // bool intl = *recipient == '+';
  enum eDCS dcs = ALPHABET_7BIT;
  overFlow = false;
  /* sanity check that concatenation parameters all zero or all not zero */
  if ((csms + numparts + partnumber) == 0)
    ; // OK so far
  else
  {
    if (csms == 0 || numparts == 0 || partnumber == 0)
      return (int)MULTIPART_NUMBERS;
    else if (partnumber > numparts)
      return (int)MULTIPART_NUMBERS;
  }
  /* proper way to check if entire message default GSM 7 bit
    scan UTF8 string, check each UCS2, bail out on 1st non-GSM7 value
  */
  bool gsm7bit = true;
  const char *savem = message;
  while (*message && gsm7bit)
  {
    unsigned short ucs2[2], target; // allow for surrogate pair
    int length = utf8Length(message);
    utf8_to_ucs2_single(message, ucs2); // translate to a single ucs2
    // ucs2 is bigendian, swap to little endian
    target = (ucs2[0] << 8) | ((ucs2[0] & 0xff00) >> 8);
    gsm7bit = isGSM7(&target);
    message += length; // bump to next
  }
  if (!gsm7bit)
    dcs = ALPHABET_16BIT;
  if (!setAddress(scabuffout, OCTETS, tempbuf)) { // set SCSA address
    return ADDRESS_FORMAT; // bail out now
  }
  else 
    beginning = smsOffset;
  int pdutype = PDU_SMS_SUBMIT;                                                                // SMS-SUBMIT
  if (csms != 0)
    pdutype |= (1 << PDU_UDHI); // set UDH bit for concatenated message
  tempbuf[smsOffset++] = pdutype;
  tempbuf[smsOffset++] = 0; // message reference
  if (!setAddress(recipient, NIBBLES, tempbuf))
  {
    return ADDRESS_FORMAT;
  }
  tempbuf[smsOffset++] = 0; // PID
  int udhsize;
  if (csms == 0)
    udhsize = 0;
  else
    udhsize = buildUDH(csms, numparts, partnumber);
  int pduLengthPlaceHolder = 0;
  int septetcount = 0;
  switch (dcs)
  {
  case ALPHABET_8BIT:
    delta = ALPHABET_8BIT_NOT_SUPPORTED; // not handled yet, cause failure
    break;
  case ALPHABET_7BIT:
    tempbuf[smsOffset++] = DCS_7BIT_ALPHABET_MASK;
    pduLengthPlaceHolder = smsOffset;
    tempbuf[smsOffset++] = 1; // placeholder for length in septets
    // insert UDH if any
#if 0
    if (udhsize == 6)    // Issue 36 This test now redundant 
    { // 1 byte ref number, need to pad UDH to 7 octets
      udhbuffer[6] = 0x40;
      udhsize++;
    }
#endif
    if (udhsize != 0)
    {
      memcpy(&tempbuf[smsOffset], udhbuffer, udhsize);
      smsOffset += udhsize;
    }
    delta = utf8_to_packed7bit(savem, &tempbuf[smsOffset], &septetcount, udhsize == 0 ? 0 : 8, /*generalWorkBuffLength - smsOffset*/MAX_NUMBER_OCTETS);
    if (delta < 0) {
      overFlow = delta == WORK_BUFFER_TOO_SMALL;
//      return delta;
    }
    else {
      tempbuf[pduLengthPlaceHolder] = septetcount;
      if (udhsize != 0)
        tempbuf[pduLengthPlaceHolder] += 8; // 7 octets of udh = 8 septets
      length = smsOffset + delta;             // allow for length byte
    }
    break;
  case ALPHABET_16BIT:
    tempbuf[smsOffset++] = DCS_16BIT_ALPHABET_MASK;
    pduLengthPlaceHolder = smsOffset;
    tempbuf[smsOffset++] = 1; // placeholder for length in octets
    // insert UDH if any
    if (udhsize != 0)
    {
      memcpy(&tempbuf[smsOffset], udhbuffer, udhsize);
      smsOffset += udhsize;
    }
    delta = utf8_to_ucs2(savem, (char *)&tempbuf[smsOffset]);
    if (delta > 0) {
      tempbuf[pduLengthPlaceHolder] = delta + udhsize; // correct message length
      length = smsOffset + delta;                        // allow for length byte
    }
    break;
  default:
    break;
  }
  // sanity check
  if (delta < 0)
    return delta;

  // now convert from binary to printable
//  memcpy(tempbuf, generalWorkBuff, length);
  // each byte in tempbuf converts to 2 bytes in generalworkbuff
  if (generalWorkBuff.size() < (length*2)) {
    overFlow = true;
    return WORK_BUFFER_TOO_SMALL;
  }
  int newoffset = 0;
  for (int i = 0; i < length; i++)
  {
    putHex(tempbuf[i], &generalWorkBuff[newoffset]);
    newoffset += 2;
  }
  generalWorkBuff[length * 2] = 0x1a;    // add ctrl z
  generalWorkBuff[(length * 2) + 1] = 0; // add end marker

  return length - beginning;
}

// convert 2 printable characters to 1 byte
unsigned char PDU::gethex(const char *pc)
{
  int i;
  // bug Issue 27, some modems present HEX characters as lowercase a-f
  // convert all to uppercase
  //if (islower(*pc)) {
  //  char LCC = *pc;
 // }
  char PC = toupper(*pc);
  if (isdigit(PC))
    i = ((unsigned char)(PC) - '0') * 16;
  else
    i = ((unsigned char)(PC) - 'A' + 10) * 16;
  PC = toupper(*++pc);
  if (isdigit(PC))
    i += (unsigned char)(PC) - '0';
  else
    i += (unsigned char)(PC) - 'A' + 10;
  return i;
}

// convert 1 byte to 2 printable characters in hex
void PDU::putHex(unsigned char b, char *target)
{
  // upper nibble
  if ((b >> 4) <= 9)
    *target++ = (b >> 4) + '0';
  else
    *target++ = (b >> 4) + 'A' - 10;
  // lower nibble
  if ((b & 0xf) <= 9)
    *target++ = (b & 0xf) + '0';
  else
    *target++ = (b & 0xf) + 'A' - 10;
}
/*
    length is in octets, output buffer ucs2 must be big enough to receive the results
*/
int PDU::pdu_to_ucs2(const char *pdu, int length, unsigned short *ucs2)
{
  unsigned short temp;
  int indexOut = 0;
  int octet = 0;
  unsigned char X;
  while (octet < length)
  {
    temp = 0;
    X = gethex(pdu);
    pdu += 2; // skip 2 chars
    octet++;
    temp = X << 8; // BE or LE ?
    X = gethex(pdu);
    pdu += 2;
    octet++;
    temp |= X; // BE or LE ?
    ucs2[indexOut++] = temp;
  }
  return indexOut;
}

int PDU::convert_7bit_to_unicode(unsigned char *gsm7bit, int length, char *unicode)
{
  int r;
  int w;

  w = 0;
  for (r = 0; r < length; r++)
  {
    // check for buffer overflow, check space for additional 4 chars buildUtf may write
    if (w+4 >= generalWorkBuff.size()) {
      overFlow = true;
      unicode[w] = 0;  // add end marker
      return w;
    }
#ifdef PM
    if ((pgm_read_byte(lookup_gsm7toUnicode + gsm7bit[r]) != 27))
    {
      const unsigned char C = pgm_read_byte(lookup_gsm7toUnicode + (unsigned char)gsm7bit[r]);
#else
    if ((lookup_gsm7toUnicode[(unsigned char)gsm7bit[r]]) != 27)
    {
      const unsigned char C = lookup_gsm7toUnicode[(unsigned char)gsm7bit[r]];
#endif
      /* Greek 7 bit was initially not handled as the lookup table was just 8 bit, presumably
         to save space.
         Now add a 7 bit to 16 bit just for those unhgandled greek characters
         Could have changed lookup_gsm7toUnicode to lookup_ascii7to16 but would waste space
      */
      // problem !! distinguish between genuine ? and non-printable character
      if (gsm7bit[r] == '?' || C != NPC8)
        w += buildUtf(C, &unicode[w]);
      else
      {
#ifdef PM
        unsigned short S = pgm_read_word(lookup_Greek7ToUnicode + (gsm7bit[r] - 16));
#else
        unsigned short S = lookup_Greek7ToUnicode[gsm7bit[r] - 16];
#endif
        w += buildUtf(S, &unicode[w]);
      }
    }
    else
    {
      /* If we're escaped then the next uint8_t have a special meaning. */
      r++;
      switch (gsm7bit[r])
      {
      case 10:
        unicode[w++] = 12;
        break;
      case 20:
        unicode[w++] = '^';
        break;
      case 40:
        unicode[w++] = '{';
        break;
      case 41:
        unicode[w++] = '}';
        break;
      case 47:
        unicode[w++] = '\\';
        break;
      case 60:
        unicode[w++] = '[';
        break;
      case 61:
        unicode[w++] = '~';
        break;
      case 62:
        unicode[w++] = ']';
        break;
      case 64:
        unicode[w++] = '|';
        break;
      case 0x65:
        // unicode[w++] = '€';  // euro
        w += buildUtf(0x20AC, &unicode[w]);
        break;
      default:
        unicode[w++] = NPC8;
        break;
      }
    }
  }

  /* Terminate the result string */
  unicode[w] = 0;

  return w;
}

int PDU::pduGsm7_to_unicode(const char *pdu, int numSeptets, char *unicode, int fillBits)
{
    unsigned char *buf = new unsigned char[numSeptets+1];
    uint16_t bits = 0;
    uint8_t bitsLeft = 0;

    for (int sept = 0, i = 0; sept < numSeptets; sept++) {
        if (bitsLeft < 7) {
            uint8_t c = gethex(&pdu[i]);
            i += 2;
            bits |= c << bitsLeft;
            bitsLeft += 8;
            if (fillBits) {
                int erase = (fillBits > 8) ? 8 : fillBits;
                fillBits -= erase;
                bits >>= erase;
                bitsLeft -= erase;
            }
        }
        if (bitsLeft >= 7) {
            buf[sept] = bits & 0x7F;
            bits >>= 7;
            bitsLeft -= 7;
        }
    }
    buf[numSeptets] = 0;


  int length = convert_7bit_to_unicode(buf, numSeptets, unicode);
  delete[] buf;
  return length;
}

/*
  Decode a complete message
  returns true for success else false
*/
bool PDU::decodePDU(const char *pdu)
{
  bool rc = true;
  int index = 0;
  int outindex = 0;
  int i, dcs, /*pid,*/ tpdu;
  bool udhPresent;

  unsigned char X;
  overFlow = false;
  i = decodeAddress(pdu, scabuffin, OCTETS);
  if (i == 0)
  {
    return false;
  }
  index = i + 4; // skip over SCA length and atn
  tpdu = gethex(&pdu[index]);
  index += 2; // skip over tpdu
  udhPresent = tpdu & (1 << PDU_UDHI);
  i = decodeAddress(&pdu[index], addressBuff, NIBBLES);
  if (i == 0)
    return false;
  index += i + 4; // skip over sender number length & atn
  // pid = gethex(&pdu[index]); // TP-PID
  index += 2;                // skip over PID
  dcs = gethex(&pdu[index]); // data coding system
  index += 2;
  // decode SCTS timestamp
  outindex = 0;
  for (i = 0; i < 7; i++)
  {
    X = gethex(&pdu[index]);
    index += 2;
    tsbuff[outindex++] = (X & 0xf) + 0x30;
    tsbuff[outindex++] = (X >> 4) + 0x30;
  }
  tsbuff[outindex] = 0;

  // decode the actual data
  int dulength = gethex(&pdu[index]);
  index += 2;
  int utflength = 0, utfoffset;
  unsigned short ucs2;
  *generalWorkBuff = 0;
  int fillBits = 0;

  memset(concatInfo, 0, sizeof(concatInfo));

  if (udhPresent)
  {
    int udhlength = gethex(&pdu[index]); // udh length is always in octets regardless of data coding
    index += 2;
    int udhidx = index; //index for header processing
    index += udhlength * 2; //skip index to SM actual beginning

    //set dubitlen to the SM actual length
    if ((dcs & DCS_ALPHABET_MASK) == DCS_7BIT_ALPHABET_MASK) {
        fillBits = (7 - ((udhlength + 1) * 8 % 7)) % 7; // header is in octets, there are fill bits aligning the SM to a 7th bit
        int udh7len = (udhlength + 1)*8 / 7 + (fillBits > 0); //lentgh in 7bit characters occupied by user data header
        dulength -= udh7len; // set to actual SM length
    }
    else {
        dulength -= udhlength + 1;
    }

    //user data header processing -------------
    while (1) {
        int iei = gethex(&pdu[udhidx]); // information element identifier
        udhidx += 2;
        int iedlength = gethex(&pdu[udhidx]); // information element data length
        udhidx += 2;

        switch (iei)
        {
        case 0: { // concatenated short messages
            if (iedlength != 3)
                return false;
            concatInfo[0] = gethex(&pdu[udhidx]); // get concatenated short message reference number
            udhidx += 2;
            concatInfo[2] = gethex(&pdu[udhidx]); // get total number of parts
            udhidx += 2;
            concatInfo[1] = gethex(&pdu[udhidx]); // get part number
            udhidx += 2;
        } break;
        case 8: { // NOTE: GSM 03.40 Version 5.3.0 documentation at page 57 lists all possible information element identifiers 
                  // it doesn't mention this one, this part should probably be deleted
            if (iedlength != 4)
                return false;
            concatInfo[0] = gethex(&pdu[udhidx]);
            udhidx += 2;
            concatInfo[0] <<= 8;
            concatInfo[0] += gethex(&pdu[udhidx]);
            udhidx += 2;
            concatInfo[2] = gethex(&pdu[udhidx]); // get total number of parts
            udhidx += 2;
            concatInfo[1] = gethex(&pdu[udhidx]); // get part number
            udhidx += 2;
        } break;
        default:
            udhidx += iedlength;
            break;
        }
        if (udhidx >= index)
            break;
    }
    //end of user data header processing -------------
    if (udhidx != index)
        return false;
  }

  switch (dcs & DCS_ALPHABET_MASK)
  {
  case DCS_7BIT_ALPHABET_MASK:
    i = pduGsm7_to_unicode(&pdu[index], dulength, generalWorkBuff.get(), fillBits);
    generalWorkBuff[i] = 0;
  //  utf8length = i;
    rc = true;
    break;
  case DCS_8BIT_ALPHABET_MASK:
    rc = false;
    break;
  case DCS_16BIT_ALPHABET_MASK:
    // loop on all ucs2 words until done
    utfoffset = 0;
    while (dulength)
    {
      pdu_to_ucs2(&pdu[index], 2, &ucs2); // treat 2 octets
      index += 4;
      dulength -= 2;
      utflength = ucs2_to_utf8(ucs2, generalWorkBuff + utfoffset);
      // check for overflow, ucs2_to_utf8 tries to write up to 4 chars check space for +3
      if (utfoffset + utflength + 3 >= generalWorkBuff.size()) {
        overFlow = true;
        break;
      }
      utfoffset += utflength;
    }
  //  utf8length = utfoffset;
    generalWorkBuff[utfoffset] = 0; // end marker
    rc = true;
    break;
  default:
    rc = false;
  }
  return rc;
}

/*
    Utilities to convert between UTF-8 and UCS-2
    ANSII-C can be used anywhere

    Author David Henry mgadriver@gmail.com
*/

#define BITS7654ON 0B11110000
#define BITS765ON 0B11100000
#define BITS76ON 0B11000000
#define BIT7ON6OFF 0B10000000
#define BITS0TO5ON 0B00111111
bool SPstart = false;
unsigned short spair[2]; // save surrogate pair

int PDU::ucs2_to_utf8(unsigned short ucs2, char *outbuf)
{
  if (/*ucs2>=0 and*/ ucs2 <= 0x7f) // 7F(16) = 127(10)
  {
    outbuf[0] = ucs2;
    return 1;
  }
  else if (ucs2 <= 0x7ff) // 7FF(16) = 2047(10)
  {
    unsigned char c1 = BITS76ON, c2 = BIT7ON6OFF;

    for (int k = 0; k < 11; ++k)
    {
      if (k < 6)
        c2 |= (ucs2 % 64) & (1 << k);
      else
        c1 |= (ucs2 >> 6) & (1 << (k - 6));
    }

    outbuf[0] = c1;
    outbuf[1] = c2;

    return 2;
  }
  else if ((ucs2 & 0xff00) >= 0xD800 && ((ucs2 & 0xff00) <= 0xDB00))
  { // start of surrogate pair
    SPstart = true;
    spair[0] = ucs2;
  }
  else if (SPstart)
  {
    SPstart = false;
    spair[1] = ucs2;
    // extract code point from pair
    unsigned long utf16 = ((spair[0] & ~0xd800) << 10) + (spair[1] & 0x03ff);
    unsigned char c1 = BITS7654ON, c2 = BIT7ON6OFF, c3 = BIT7ON6OFF, c4 = BIT7ON6OFF;
    utf16 += 0x10000;
    for (int k = 0; k < 22; ++k) // 22 bits in pack
    {
      if (k < 6) // bits 0-6
        c4 |= (utf16 % 64) & (1 << k);
      else if (k < 12) // bits 6-11
        c3 |= (utf16 >> 6) & (1 << (k - 6));
      else if (k < 18) // bits 7-18
        c2 |= (utf16 >> 12) & (1 << (k - 12));
      else // bits 19-22
        c1 |= (utf16 >> 18) & (1 << (k - 18));
    }
    outbuf[0] = c1;
    outbuf[1] = c2;
    outbuf[2] = c3;
    outbuf[3] = c4;

    return 4;
  }
  else // if (ucs2 <= 0xffff)  // FFFF(16) = 65535(10)
  {
    unsigned char c1 = BITS765ON, c2 = BIT7ON6OFF, c3 = BIT7ON6OFF;

    for (int k = 0; k < 16; ++k) // 16 bits in pack
    {
      if (k < 6)
        c3 |= (ucs2 % 64) & (1 << k);
      else if (k < 12)
        c2 |= (ucs2 >> 6) & (1 << (k - 6));
      else
        c1 |= (ucs2 >> 12) & (1 << (k - 12));
    }
    outbuf[0] = c1;
    outbuf[1] = c2;
    outbuf[2] = c3;

    return 3;
  }

  return 0;
}
/*
   return number of bytes used by this UTF8 unicode character
*/
int PDU::utf8Length(const char *utf8) const
{
  int length = 1;
  unsigned char mask = BITS76ON;
  // look for ascii 7 on 1st byte
  if ((*utf8 & BIT7ON6OFF) == 0)
    ;
  else
  {
    // look for length pattern on first byte - 2 r more continuous 1's
    while ((*utf8 & mask) == mask)
    {
      length++;
      mask = (mask >> 1 | BIT7ON6OFF);
    }
    if (length > 1)
    { // validate continuation bytes
      int LEN = length - 1;
      utf8++;
      while (LEN)
      {
        if ((*utf8++ & BITS76ON) == BIT7ON6OFF)
          LEN--;
        else
          break;
      }
      if (LEN != 0)
        length = -1;
    }
    else
      length = -1; //
  }
  return length;
}
/*
    convert an utf8 string to a single ucs2
    return number of octets
    Correction: Allow for the creation of surrogate pairs
    If the input character is in the range 0x10000 to 0x10ffff, convert into a pair of UCS2 words
*/
int PDU::utf8_to_ucs2_single(const char *utf8, unsigned short *target)
{
  unsigned short ucs2[2];
  int numbytes = 0;
  int cont = utf8Length(utf8) - 1; // number of continuation bytes
  unsigned long utf16;
  if (cont < 0)
    return 0;
  if (cont == 0)
  { // ascii 7 bit
    ucs2[0] = *utf8;
    ucs2[1] = 0; // was missing before
    numbytes = 2;
  }
  else
  {
    // read n bits of first byte then 6 bits of each continuation
    unsigned char mask = BITS0TO5ON;
    int temp = cont;
    while (temp-- > 0)
      mask >>= 1;
    utf16 = *utf8++ & mask;
    // add continuation bytes
    while (cont-- > 0)
    {
      utf16 = (utf16 << 6) | (*(utf8++) & BITS0TO5ON);
    }
    // check if we need to make a surrogate pair
    if (utf16 < 0x10000)
    {
      ucs2[0] = utf16;
      numbytes = 2;
    }
    else
    {
      utf16 -= 0x10000;
      ucs2[0] = 0xD800 | (utf16 >> 10);
      ucs2[1] = 0xDC00 | (utf16 & 0x3ff);
      numbytes = 4;
    }
  }
  *target = (ucs2[0] >> 8) | ((ucs2[0] & 0x0ff) << 8); // swap bytes
  if (numbytes > 2)
  {
    target++;
    *target = (ucs2[1] >> 8) | ((ucs2[1] & 0x0ff) << 8); // swap bytes
  }
  return numbytes;
}

const char *PDU::getSender() const
{
  return addressBuff;
}
const char *PDU::getTimeStamp() const
{
  return tsbuff;
}
const char *PDU::getText() const
{
  return generalWorkBuff.get();
}

void PDU::BCDtoString(char *output, const char *input, int length)
{
  unsigned char X;
  for (int i = 0; i < length; i += 2)
  {
    X = gethex(input);
    input += 2;
    *output++ = (X & 0xf) + 0x30;
    if ((X & 0xf0) == 0xf0) // end filler
      break;
    *output++ = (X >> 4) + 0x30;
  }
  *output = 0; // add end of string
}

/*
    returns number of characters to occupied by number part (after length and atn)
    returns 0 if number cannot be decoded
*/
int PDU::decodeAddress(const char *pdu, char *output, eLengthType et)
{                           // pdu to readable starts with length octet
  int length = gethex(pdu); // could be nibbles or octets
  // if octets, length include TON so reduce by 1
  // if nibbles length is just the number
  if (et == NIBBLES)
    addressLength = length;
  else
    addressLength = --length * 2;
  pdu += 2; // gethex reads 2 bytes
  // now analyse address type
  int adt = gethex(pdu);
  pdu += 2;
  if ((adt & EXT_MASK) != 0)
  {
    switch ((adt & TON_MASK) >> TON_OFFSET)
    {
    case 1:            // international number
      *output++ = '+'; // add prefix and fall through
     // [[fallthrough]]
    case 0: // issue #39
    //  [[fallthrough]]
    case 2: // national number
    //  [[fallthrough]];
    //case 3: // network specific number, Issue #26
      BCDtoString(output, pdu, addressLength);
      if ((addressLength & 1) == 1) // if odd, bump 1
        addressLength++;            // we could do this before calling BCDtoString
      break;
    case 5: // alphabetic, convert  nibble length to septets
      pduGsm7_to_unicode(pdu, (addressLength * 4) / 7, output);
      if ((addressLength & 1) == 1) // if odd, bump 1
        addressLength++;            // we could do NOT this before calling pduGsm7_to_unicode
      break;
    default:
      addressLength = 0;
      break;
    }
  }
  else
  {
    addressLength = 0; // dont know how to handle EXT
  }
  return addressLength;
}
/*
    Translate a UTF-8 string to UCS2 octets
    Return number of octets, else -2 if too large to fit in a PDU
*/
int PDU::utf8_to_ucs2(const char *utf8, char *ucs2)
{ // translate an utf8 zero terminated string
  int octets = 0, ucslength;
  unsigned short tempucs2[2]; // allow space for surrogate pair
  while (*utf8 && octets <= MAX_NUMBER_OCTETS)
  {
    int inputlen = utf8Length(utf8);
    //    int ucslength = utf8_to_ucs2_single(utf8,(short *)ucs2);
    ucslength = utf8_to_ucs2_single(utf8, tempucs2);
    // sanity check against overflowing the buffer
    if (octets + ucslength > MAX_NUMBER_OCTETS)
      return UCS2_TOO_LONG;
    // ok to continue
    memcpy(ucs2, tempucs2, ucslength);
    utf8 += inputlen;    // bump input pointer
    ucs2 += ucslength;   // bump output pointer
    octets += ucslength; // bump total number of octets created
  }
  return octets;
}

const char *PDU::getSMS() const
{
  return generalWorkBuff.get();
}

void PDU::setSCAnumber(const char *n)
{
  strcpy(scabuffout, n);
}

const char *PDU::getSCAnumber() const
{
  return scabuffin; // from INCOMING SMS
}

void PDU::buildUtf16(unsigned long cp, char *target)
{
  buildUtf(cp, target); // for backward compatibility
}

int PDU::buildUtf(unsigned long cp, char *target)
{
  unsigned char buf[5];
  int length;
  if (cp <= 0x7f) // ASCII
  {
    length = 1;
    buf[0] = cp;
    buf[length] = 0;
  }
  else if (cp <= 0x7ff)
  { // Extended latin, greek, hebrew, arabic, cyrillic
    length = 2;
    buf[0] = BITS76ON;
    buf[1] = BIT7ON6OFF;
    buf[length] = 0;
    for (int k = 0; k < 11; ++k) // 11 bits in pack
    {
      if (k < 6)
        buf[1] |= (cp % 64) & (1 << k);
      else
        buf[0] |= (cp >> 6) & (1 << (k - 6));
    }
  }
  else if (cp <= 0xffff)
  { // many Asian languages
    length = 3;
    buf[0] = BITS765ON;
    buf[1] = BIT7ON6OFF;
    buf[2] = BIT7ON6OFF;
    buf[length] = 0;
    for (int k = 0; k < 16; ++k) // 16 bits in pack
    {
      if (k < 6)
        buf[2] |= (cp % 64) & (1 << k);
      else if (k < 12)
        buf[1] |= (cp >> 6) & (1 << (k - 6));
      else
        buf[0] |= (cp >> 12) & (1 << (k - 12));
    }
  }
  else if (cp > 0x10000)
  { // emojis, drawings, chinese
    length = 4;
    buf[0] = BITS7654ON;
    buf[1] = BIT7ON6OFF;
    buf[2] = BIT7ON6OFF;
    buf[3] = BIT7ON6OFF;
    buf[length] = 0;
    for (int k = 0; k < 22; ++k) // 22 bits in pack
    {
      if (k < 6) // bits 0-6
        buf[3] |= (cp % 64) & (1 << k);
      else if (k < 12) // bits 6-11
        buf[2] |= (cp >> 6) & (1 << (k - 6));
      else if (k < 18) // bits 7-18
        buf[1] |= (cp >> 12) & (1 << (k - 12));
      else // bits 19-22
        buf[0] |= (cp >> 18) & (1 << (k - 18));
    }
  }
  strcpy(target, (char *)buf);
  return strlen(target);
}

bool PDU::isGSM7(unsigned short *pucs) const
{
  for (unsigned int i = 0; i < sizeof(gsm7_legal) / sizeof(sRange); i++)
  {
#ifdef PM
    if (*pucs >= pgm_read_word(&gsm7_legal[i].min) && *pucs <= pgm_read_word(&gsm7_legal[i].max))
#else
    if (*pucs >= gsm7_legal[i].min && *pucs <= gsm7_legal[i].max)
#endif
      return true;
  }
  return false;
}

const int *PDU::getConcatInfo() const
{
  return concatInfo;
}

bool PDU::getOverflow() const {
  return overFlow;
}
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
/*
    The source to be scanned is actually Unicode in UTF8 format.
    While this table may be badly named and contains values irrelevant to
    this project, it is still good enough.
    The tables is indexed by a unicode value and returns the GSM7 value for that symbol
    TBD change name to reflect lookup Unicode to GSM7
*/
const
#ifdef PM
    PROGMEM
#endif
    short lookup_ascii8to7[] = {
        NPC7,     /*     0      null [NUL]                              */
        NPC7,     /*     1      start of heading [SOH]                  */
        NPC7,     /*     2      start of text [STX]                     */
        NPC7,     /*     3      end of text [ETX]                       */
        NPC7,     /*     4      end of transmission [EOT]               */
        NPC7,     /*     5      enquiry [ENQ]                           */
        NPC7,     /*     6      acknowledge [ACK]                       */
        NPC7,     /*     7      bell [BEL]                              */
        NPC7,     /*     8      backspace [BS]                          */
        NPC7,     /*     9      horizontal tab [HT]                     */
        10,       /*    10      line feed [LF]                          */
        NPC7,     /*    11      vertical tab [VT]                       */
        10 + 256, /*    12      form feed [FF]                          */
        13,       /*    13      carriage return [CR]                    */
        NPC7,     /*    14      shift out [SO]                          */
        NPC7,     /*    15      shift in [SI]                           */
        NPC7,     /*    16      data link escape [DLE]                  */
        NPC7,     /*    17      device control 1 [DC1]                  */
        NPC7,     /*    18      device control 2 [DC2]                  */
        NPC7,     /*    19      device control 3 [DC3]                  */
        NPC7,     /*    20      device control 4 [DC4]                  */
        NPC7,     /*    21      negative acknowledge [NAK]              */
        NPC7,     /*    22      synchronous idle [SYN]                  */
        NPC7,     /*    23      end of trans. block [ETB]               */
        NPC7,     /*    24      cancel [CAN]                            */
        NPC7,     /*    25      end of medium [EM]                      */
        NPC7,     /*    26      substitute [SUB]                        */
        NPC7,     /*    27      escape [ESC]                            */
        NPC7,     /*    28      file separator [FS]                     */
        NPC7,     /*    29      group separator [GS]                    */
        NPC7,     /*    30      record separator [RS]                   */
        NPC7,     /*    31      unit separator [US]                     */
        32,       /*    32      space                                   */
        33,       /*    33    ! exclamation mark                        */
        34,       /*    34    " double quotation mark                   */
        35,       /*    35    # number sign                             */
        2,        /*    36    $ dollar sign                             */
        37,       /*    37    % percent sign                            */
        38,       /*    38    & ampersand                               */
        39,       /*    39    ' apostrophe                              */
        40,       /*    40    ( left parenthesis                        */
        41,       /*    41    ) right parenthesis                       */
        42,       /*    42    * asterisk                                */
        43,       /*    43    + plus sign                               */
        44,       /*    44    , comma                                   */
        45,       /*    45    - hyphen                                  */
        46,       /*    46    . period                                  */
        47,       /*    47    / slash,                                  */
        48,       /*    48    0 digit 0                                 */
        49,       /*    49    1 digit 1                                 */
        50,       /*    50    2 digit 2                                 */
        51,       /*    51    3 digit 3                                 */
        52,       /*    52    4 digit 4                                 */
        53,       /*    53    5 digit 5                                 */
        54,       /*    54    6 digit 6                                 */
        55,       /*    55    7 digit 7                                 */
        56,       /*    56    8 digit 8                                 */
        57,       /*    57    9 digit 9                                 */
        58,       /*    58    : colon                                   */
        59,       /*    59    ; semicolon                               */
        60,       /*    60    < less-than sign                          */
        61,       /*    61    = equal sign                              */
        62,       /*    62    > greater-than sign                       */
        63,       /*    63    ? question mark                           */
        0,        /*    64    @ commercial at sign                      */
        65,       /*    65    A uppercase A                             */
        66,       /*    66    B uppercase B                             */
        67,       /*    67    C uppercase C                             */
        68,       /*    68    D uppercase D                             */
        69,       /*    69    E uppercase E                             */
        70,       /*    70    F uppercase F                             */
        71,       /*    71    G uppercase G                             */
        72,       /*    72    H uppercase H                             */
        73,       /*    73    I uppercase I                             */
        74,       /*    74    J uppercase J                             */
        75,       /*    75    K uppercase K                             */
        76,       /*    76    L uppercase L                             */
        77,       /*    77    M uppercase M                             */
        78,       /*    78    N uppercase N                             */
        79,       /*    79    O uppercase O                             */
        80,       /*    80    P uppercase P                             */
        81,       /*    81    Q uppercase Q                             */
        82,       /*    82    R uppercase R                             */
        83,       /*    83    S uppercase S                             */
        84,       /*    84    T uppercase T                             */
        85,       /*    85    U uppercase U                             */
        86,       /*    86    V uppercase V                             */
        87,       /*    87    W uppercase W                             */
        88,       /*    88    X uppercase X                             */
        89,       /*    89    Y uppercase Y                             */
        90,       /*    90    Z uppercase Z                             */
        60 + 256, /*    91    [ left square bracket                     */
        47 + 256, /*    92    \ backslash                               */
        62 + 256, /*    93    ] right square bracket                    */
        20 + 256, /*    94    ^ circumflex accent                       */
        17,       /*    95    _ underscore                              */
        -39,      /*    96    ` back apostrophe                         */
        97,       /*    97    a lowercase a                             */
        98,       /*    98    b lowercase b                             */
        99,       /*    99    c lowercase c                             */
        100,      /*   100    d lowercase d                             */
        101,      /*   101    e lowercase e                             */
        102,      /*   102    f lowercase f                             */
        103,      /*   103    g lowercase g                             */
        104,      /*   104    h lowercase h                             */
        105,      /*   105    i lowercase i                             */
        106,      /*   106    j lowercase j                             */
        107,      /*   107    k lowercase k                             */
        108,      /*   108    l lowercase l                             */
        109,      /*   109    m lowercase m                             */
        110,      /*   110    n lowercase n                             */
        111,      /*   111    o lowercase o                             */
        112,      /*   112    p lowercase p                             */
        113,      /*   113    q lowercase q                             */
        114,      /*   114    r lowercase r                             */
        115,      /*   115    s lowercase s                             */
        116,      /*   116    t lowercase t                             */
        117,      /*   117    u lowercase u                             */
        118,      /*   118    v lowercase v                             */
        119,      /*   119    w lowercase w                             */
        120,      /*   120    x lowercase x                             */
        121,      /*   121    y lowercase y                             */
        122,      /*   122    z lowercase z                             */
        40 + 256, /*   123    { left brace                              */
        64 + 256, /*   124    | vertical bar                            */
        41 + 256, /*   125    } right brace                             */
        61 + 256, /*   126    ~ tilde accent                            */
        NPC7,     /*   127      delete [DEL]                            */
        NPC7,     /*   128                                              */
        NPC7,     /*   129                                              */
        -39,      /*   130      low left rising single quote            */
        -102,     /*   131      lowercase italic f                      */
        -34,      /*   132      low left rising double quote            */
        NPC7,     /*   133      low horizontal ellipsis                 */
        NPC7,     /*   134      dagger mark                             */
        NPC7,     /*   135      double dagger mark                      */
        NPC7,     /*   136      letter modifying circumflex             */
        NPC7,     /*   137      per thousand (mille) sign               */
        -83,      /*   138      uppercase S caron or hacek              */
        -39,      /*   139      left single angle quote mark            */
        -214,     /*   140      uppercase OE ligature                   */
        NPC7,     /*   141                                              */
        NPC7,     /*   142                                              */
        NPC7,     /*   143                                              */
        NPC7,     /*   144                                              */
        -39,      /*   145      left single quotation mark              */
        -39,      /*   146      right single quote mark                 */
        -34,      /*   147      left double quotation mark              */
        -34,      /*   148      right double quote mark                 */
        -42,      /*   149      round filled bullet                     */
        -45,      /*   150      en dash                                 */
        -45,      /*   151      em dash                                 */
        -39,      /*   152      small spacing tilde accent              */
        NPC7,     /*   153      trademark sign                          */
        -115,     /*   154      lowercase s caron or hacek              */
        -39,      /*   155      right single angle quote mark           */
        -111,     /*   156      lowercase oe ligature                   */
        NPC7,     /*   157                                              */
        NPC7,     /*   158                                              */
        -89,      /*   159      uppercase Y dieresis or umlaut          */
        -32,      /*   160      non-breaking space                      */
        64,       /*   161    ¡ inverted exclamation mark               */
        -99,      /*   162    ¢ cent sign                               */
        1,        /*   163    £ pound sterling sign                     */
        36,       /*   164    ? general currency sign                   */
        3,        /*   165    ¥ yen sign                                */
        -33,      /*   166    ¦ broken vertical bar                     */
        95,       /*   167    § section sign                            */
        -34,      /*   168    ¨ spacing dieresis or umlaut              */
        NPC7,     /*   169    © copyright sign                          */
        NPC7,     /*   170    × feminine ordinal indicator              */
        -60,      /*   171    « left (double) angle quote               */
        NPC7,     /*   172    ¬ logical not sign                        */
        -45,      /*   173    ­ soft hyphen                             */
        NPC7,     /*   174    ® registered trademark sign               */
        NPC7,     /*   175    ¯ spacing macron (long) accent            */
        NPC7,     /*   176    ° degree sign                             */
        NPC7,     /*   177    ± plus-or-minus sign                      */
        -50,      /*   178    ² superscript 2                           */
        -51,      /*   179    ³ superscript 3                           */
        -39,      /*   180    ´ spacing acute accent                    */
        -117,     /*   181    µ micro sign                              */
        NPC7,     /*   182    ¶ paragraph sign, pilcrow sign            */
        NPC7,     /*   183    · middle dot, centered dot                */
        NPC7,     /*   184    ¸ spacing cedilla                         */
        -49,      /*   185    ¹ superscript 1                           */
        NPC7,     /*   186    ÷ masculine ordinal indicator             */
        -62,      /*   187    » right (double) angle quote (guillemet)  */
        NPC7,     /*   188    ¼ fraction 1/4                            */
        NPC7,     /*   189    ½ fraction 1/2                            */
        NPC7,     /*   190    ¾ fraction 3/4                            */
        96,       /*   191    ¿ inverted question mark                  */
        -65,      /*   192    ? uppercase A grave                       */
        -65,      /*   193    ? uppercase A acute                       */
        -65,      /*   194    ? uppercase A circumflex                  */
        -65,      /*   195    ? uppercase A tilde                       */
        91,       /*   196    ? uppercase A dieresis or umlaut          */
        14,       /*   197    ? uppercase A ring                        */
        28,       /*   198    ? uppercase AE ligature                   */
        9,        /*   199    ? uppercase C cedilla                     */
        -31,      /*   200    ? uppercase E grave                       */
        31,       /*   201    ? uppercase E acute                       */
        -31,      /*   202    ? uppercase E circumflex                  */
        -31,      /*   203    ? uppercase E dieresis or umlaut          */
        -73,      /*   204    ? uppercase I grave                       */
        -73,      /*   205    ? uppercase I acute                       */
        -73,      /*   206    ? uppercase I circumflex                  */
        -73,      /*   207    ? uppercase I dieresis or umlaut          */
        -68,      /*   208    ? uppercase ETH                           */
        93,       /*   209    ? uppercase N tilde                       */
        -79,      /*   210    ? uppercase O grave                       */
        -79,      /*   211    ? uppercase O acute                       */
        -79,      /*   212    ? uppercase O circumflex                  */
        -79,      /*   213    ? uppercase O tilde                       */
        92,       /*   214    ? uppercase O dieresis or umlaut          */
        -42,      /*   215    ? multiplication sign                     */
        11,       /*   216    ? uppercase O slash                       */
        -85,      /*   217    ? uppercase U grave                       */
        -85,      /*   218    ? uppercase U acute                       */
        -85,      /*   219    ? uppercase U circumflex                  */
        94,       /*   220    ? uppercase U dieresis or umlaut          */
        -89,      /*   221    ? uppercase Y acute                       */
        NPC7,     /*   222    ? uppercase THORN                         */
        30,       /*   223    ? lowercase sharp s, sz ligature          */
        127,      /*   224    ? lowercase a grave                       */
        -97,      /*   225    ? lowercase a acute                       */
        -97,      /*   226    ? lowercase a circumflex                  */
        -97,      /*   227    ? lowercase a tilde                       */
        123,      /*   228    ? lowercase a dieresis or umlaut          */
        15,       /*   229    ? lowercase a ring                        */
        29,       /*   230    ? lowercase ae ligature                   */
        -9,       /*   231    ? lowercase c cedilla                     */
        4,        /*   232    ? lowercase e grave                       */
        5,        /*   233    ? lowercase e acute                       */
        -101,     /*   234    ? lowercase e circumflex                  */
        -101,     /*   235    ? lowercase e dieresis or umlaut          */
        7,        /*   236    ? lowercase i grave                       */
        7,        /*   237    ? lowercase i acute                       */
        -105,     /*   238    ? lowercase i circumflex                  */
        -105,     /*   239    ? lowercase i dieresis or umlaut          */
        NPC7,     /*   240    ? lowercase eth                           */
        125,      /*   241    ? lowercase n tilde                       */
        8,        /*   242    ? lowercase o grave                       */
        -111,     /*   243    ? lowercase o acute                       */
        -111,     /*   244    ? lowercase o circumflex                  */
        -111,     /*   245    ? lowercase o tilde                       */
        124,      /*   246    ? lowercase o dieresis or umlaut          */
        -47,      /*   247    ? division sign                           */
        12,       /*   248    ? lowercase o slash                       */
        6,        /*   249    ? lowercase u grave                       */
        -117,     /*   250    ? lowercase u acute                       */
        -117,     /*   251    ? lowercase u circumflex                  */
        126,      /*   252    ? lowercase u dieresis or umlaut          */
        -121,     /*   253    ? lowercase y acute                       */
        NPC7,     /*   254    ? lowercase thorn                         */
        -121      /*   255    ? lowercase y dieresis or umlaut          */
    };

/****************************************************************************
This lookup table converts from the 7 bit "default alphabet" as
defined in ETSI GSM 03.38 to a standard ISO-8859-1 8-bit ASCII.

Some characters in the 7-bit alphabet does not exist in the ISO
character set, they are replaced by the NPC8-character.

If the character is decimal 27 (ESC) the following character have
a special meaning and must be handled separately.
****************************************************************************/
/*
   This table is index by a GSM 7 value to yield the Unicode value for that symbol
   Unicode values > 256 (Greek uppercase are returned as NPC8 and handled separately)
   THis could be avoided by making a table of unsigned short but that would be double the size
   and problematic for Arduino
   TBD change name to relect lookup GSM7 to Unicode
*/

const
#ifdef PM
    PROGMEM
#endif
    unsigned char lookup_gsm7toUnicode[] = {
        64,   /*  0      @  COMMERCIAL AT                           */
        163,  /*  1      £  POUND SIGN                              */
        36,   /*  2      $  DOLLAR SIGN                             */
        165,  /*  3      ¥  YEN SIGN                                */
        232,  /*  4      ?  LATIN SMALL LETTER E WITH GRAVE         */
        233,  /*  5      ?  LATIN SMALL LETTER E WITH ACUTE         */
        249,  /*  6      ?  LATIN SMALL LETTER U WITH GRAVE         */
        236,  /*  7      ?  LATIN SMALL LETTER I WITH GRAVE         */
        242,  /*  8      ?  LATIN SMALL LETTER O WITH GRAVE         */
        199,  /*  9      ?  LATIN CAPITAL LETTER C WITH CEDILLA     */
        10,   /*  10        LINE FEED                               */
        216,  /*  11     ?  LATIN CAPITAL LETTER O WITH STROKE      */
        248,  /*  12     ?  LATIN SMALL LETTER O WITH STROKE        */
        13,   /*  13        CARRIAGE RETURN                         */
        197,  /*  14     ?  LATIN CAPITAL LETTER A WITH RING ABOVE  */
        229,  /*  15     ?  LATIN SMALL LETTER A WITH RING ABOVE    */
        NPC8, /*  16        GREEK CAPITAL LETTER DELTA              */
        95,   /*  17     _  LOW LINE                                */
        NPC8, /*  18        GREEK CAPITAL LETTER PHI                */
        NPC8, /*  19        GREEK CAPITAL LETTER GAMMA              */
        NPC8, /*  20        GREEK CAPITAL LETTER LAMBDA             */
        NPC8, /*  21        GREEK CAPITAL LETTER OMEGA              */
        NPC8, /*  22        GREEK CAPITAL LETTER PI                 */
        NPC8, /*  23        GREEK CAPITAL LETTER PSI                */
        NPC8, /*  24        GREEK CAPITAL LETTER SIGMA              */
        NPC8, /*  25        GREEK CAPITAL LETTER THETA              */
        NPC8, /*  26        GREEK CAPITAL LETTER XI                 */
        27,   /*  27        ESCAPE TO EXTENSION TABLE               */
        198,  /*  28     ?  LATIN CAPITAL LETTER AE                 */
        230,  /*  29     ?  LATIN SMALL LETTER AE                   */
        223,  /*  30     ?  LATIN SMALL LETTER SHARP S (German)     */
        201,  /*  31     ?  LATIN CAPITAL LETTER E WITH ACUTE       */
        32,   /*  32        SPACE                                   */
        33,   /*  33     !  EXCLAMATION MARK                        */
        34,   /*  34     "  QUOTATION MARK                          */
        35,   /*  35     #  NUMBER SIGN                             */
        164,  /*  36     ?  CURRENCY SIGN                           */
        37,   /*  37     %  PERCENT SIGN                            */
        38,   /*  38     &  AMPERSAND                               */
        39,   /*  39     '  APOSTROPHE                              */
        40,   /*  40     (  LEFT PARENTHESIS                        */
        41,   /*  41     )  RIGHT PARENTHESIS                       */
        42,   /*  42     *  ASTERISK                                */
        43,   /*  43     +  PLUS SIGN                               */
        44,   /*  44     ,  COMMA                                   */
        45,   /*  45     -  HYPHEN-MINUS                            */
        46,   /*  46     .  FULL STOP                               */
        47,   /*  47     /  SOLIDUS (SLASH)                         */
        48,   /*  48     0  DIGIT ZERO                              */
        49,   /*  49     1  DIGIT ONE                               */
        50,   /*  50     2  DIGIT TWO                               */
        51,   /*  51     3  DIGIT THREE                             */
        52,   /*  52     4  DIGIT FOUR                              */
        53,   /*  53     5  DIGIT FIVE                              */
        54,   /*  54     6  DIGIT SIX                               */
        55,   /*  55     7  DIGIT SEVEN                             */
        56,   /*  56     8  DIGIT EIGHT                             */
        57,   /*  57     9  DIGIT NINE                              */
        58,   /*  58     :  COLON                                   */
        59,   /*  59     ;  SEMICOLON                               */
        60,   /*  60     <  LESS-THAN SIGN                          */
        61,   /*  61     =  EQUALS SIGN                             */
        62,   /*  62     >  GREATER-THAN SIGN                       */
        63,   /*  63     ?  QUESTION MARK                           */
        161,  /*  64     ¡  INVERTED EXCLAMATION MARK               */
        65,   /*  65     A  LATIN CAPITAL LETTER A                  */
        66,   /*  66     B  LATIN CAPITAL LETTER B                  */
        67,   /*  67     C  LATIN CAPITAL LETTER C                  */
        68,   /*  68     D  LATIN CAPITAL LETTER D                  */
        69,   /*  69     E  LATIN CAPITAL LETTER E                  */
        70,   /*  70     F  LATIN CAPITAL LETTER F                  */
        71,   /*  71     G  LATIN CAPITAL LETTER G                  */
        72,   /*  72     H  LATIN CAPITAL LETTER H                  */
        73,   /*  73     I  LATIN CAPITAL LETTER I                  */
        74,   /*  74     J  LATIN CAPITAL LETTER J                  */
        75,   /*  75     K  LATIN CAPITAL LETTER K                  */
        76,   /*  76     L  LATIN CAPITAL LETTER L                  */
        77,   /*  77     M  LATIN CAPITAL LETTER M                  */
        78,   /*  78     N  LATIN CAPITAL LETTER N                  */
        79,   /*  79     O  LATIN CAPITAL LETTER O                  */
        80,   /*  80     P  LATIN CAPITAL LETTER P                  */
        81,   /*  81     Q  LATIN CAPITAL LETTER Q                  */
        82,   /*  82     R  LATIN CAPITAL LETTER R                  */
        83,   /*  83     S  LATIN CAPITAL LETTER S                  */
        84,   /*  84     T  LATIN CAPITAL LETTER T                  */
        85,   /*  85     U  LATIN CAPITAL LETTER U                  */
        86,   /*  86     V  LATIN CAPITAL LETTER V                  */
        87,   /*  87     W  LATIN CAPITAL LETTER W                  */
        88,   /*  88     X  LATIN CAPITAL LETTER X                  */
        89,   /*  89     Y  LATIN CAPITAL LETTER Y                  */
        90,   /*  90     Z  LATIN CAPITAL LETTER Z                  */
        196,  /*  91     ?  LATIN CAPITAL LETTER A WITH DIAERESIS   */
        214,  /*  92     ?  LATIN CAPITAL LETTER O WITH DIAERESIS   */
        209,  /*  93     ?  LATIN CAPITAL LETTER N WITH TILDE       */
        220,  /*  94     ?  LATIN CAPITAL LETTER U WITH DIAERESIS   */
        167,  /*  95     §  SECTION SIGN                            */
        191,  /*  96     ¿  INVERTED QUESTION MARK                  */
        97,   /*  97     a  LATIN SMALL LETTER A                    */
        98,   /*  98     b  LATIN SMALL LETTER B                    */
        99,   /*  99     c  LATIN SMALL LETTER C                    */
        100,  /*  100    d  LATIN SMALL LETTER D                    */
        101,  /*  101    e  LATIN SMALL LETTER E                    */
        102,  /*  102    f  LATIN SMALL LETTER F                    */
        103,  /*  103    g  LATIN SMALL LETTER G                    */
        104,  /*  104    h  LATIN SMALL LETTER H                    */
        105,  /*  105    i  LATIN SMALL LETTER I                    */
        106,  /*  106    j  LATIN SMALL LETTER J                    */
        107,  /*  107    k  LATIN SMALL LETTER K                    */
        108,  /*  108    l  LATIN SMALL LETTER L                    */
        109,  /*  109    m  LATIN SMALL LETTER M                    */
        110,  /*  110    n  LATIN SMALL LETTER N                    */
        111,  /*  111    o  LATIN SMALL LETTER O                    */
        112,  /*  112    p  LATIN SMALL LETTER P                    */
        113,  /*  113    q  LATIN SMALL LETTER Q                    */
        114,  /*  114    r  LATIN SMALL LETTER R                    */
        115,  /*  115    s  LATIN SMALL LETTER S                    */
        116,  /*  116    t  LATIN SMALL LETTER T                    */
        117,  /*  117    u  LATIN SMALL LETTER U                    */
        118,  /*  118    v  LATIN SMALL LETTER V                    */
        119,  /*  119    w  LATIN SMALL LETTER W                    */
        120,  /*  120    x  LATIN SMALL LETTER X                    */
        121,  /*  121    y  LATIN SMALL LETTER Y                    */
        122,  /*  122    z  LATIN SMALL LETTER Z                    */
        228,  /*  123    ?  LATIN SMALL LETTER A WITH DIAERESIS     */
        246,  /*  124    ?  LATIN SMALL LETTER O WITH DIAERESIS     */
        241,  /*  125    ?  LATIN SMALL LETTER N WITH TILDE         */
        252,  /*  126    ?  LATIN SMALL LETTER U WITH DIAERESIS     */
        224   /*  127    ?  LATIN SMALL LETTER A WITH GRAVE         */

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

/*
   Decode Greek marked as NPC8 in lookup_gsm7toUnicode
*/
const
#ifdef PM
    PROGMEM
#endif
    unsigned short lookup_Greek7ToUnicode[] = {
        0x394, /*  16        GREEK CAPITAL LETTER DELTA              */
        95,    /*  17     _  LOW LINE                                */
        0x3a6, /*  18        GREEK CAPITAL LETTER PHI                */
        0x393, /*  19        GREEK CAPITAL LETTER GAMMA              */
        0x39b, /*  20        GREEK CAPITAL LETTER LAMBDA             */
        0x3a9, /*  21        GREEK CAPITAL LETTER OMEGA              */
        0x3a0, /*  22        GREEK CAPITAL LETTER PI                 */
        0x3a8, /*  23        GREEK CAPITAL LETTER PSI                */
        0x3a3, /*  24        GREEK CAPITAL LETTER SIGMA              */
        0x398, /*  25        GREEK CAPITAL LETTER THETA              */
        0x39e, /*  26        GREEK CAPITAL LETTER XI                 */
    };

/*
   Decode Greek marked as NPC8 in lookup_gsm7toUnicode
*/
const
#ifdef PM
    PROGMEM
#endif
    unsigned short lookup_UnicodeToGreek7[] = {
        19, /* GREEK CAPITAL LETTER GAMMA  0x393            */
        16, /* GREEK CAPITAL LETTER DELTA  0x394          */
        0,  /* 0x395 */
        0,  /* 0x396 */
        0,  /* 0x397 */
        25, /*  GREEK CAPITAL LETTER THETA  0x398            */
        0,  /* 0x399 */
        0,  /* 0x39a */
        20, /*  GREEK CAPITAL LETTER LAMBDA 0x39b            */
        0,  /* 0x39c */
        0,  /* 0x39d */
        26, /* GREEK CAPITAL LETTER XI   0x39e              */
        0,  /* 0x39f */
        22, /* GREEK CAPITAL LETTER PI 0x3a0                */
        0,  /* 0x3a1 */
        0,  /* 0x3a2 */
        24, /* GREEK CAPITAL LETTER SIGMA  0x3a3            */
        0,  /* 0x3a4 */
        0,  /* 0x3a5 */
        18, /* GREEK CAPITAL LETTER PHI 0x3a6               */
        0,  /* 0x3a7 */
        23, /* GREEK CAPITAL LETTER PSI  0x3a8              */
        21  /* GREEK CAPITAL LETTER OMEGA 0x3a9,             */
    };
