
//#include <Arduino.h>
#include <iostream>
#include <string.h>
//#include <stdio.h>
#include <ctype.h>
#include <stdint-gcc.h>
#include <math.h>
#include "pdulib.h"
//#define PM  // forces tables into PROGMEM

PDU::PDU()
{
  // initialize all local variables
  scavalid = recvalid = mesvalid = false;
  csvalid = false;
  dcs = ALPHABET_16BIT;
}

PDU::~PDU() {}

#if 0
bool PDU::setSCA(char *sca)
{
  bool rc = false;
  if (strlen(sca) < MAX_NUMBER_LENGTH)
  {
    strcpy(scabuff,sca);
    scalength = strlen(sca);
    rc = true;
  }
  else if (sca == 0)   // sca is optional null string
  {
    scalength = 0;
    rc = true;
  }
  scavalid = rc;
  return rc;
}
#endif

/*
  Save recipient phone number, check that it is numeric
  return true if valid
*/
bool PDU::setAddress(const char *address,eAddressType at)
{
  bool rc = false;
  if (strlen(address) < MAX_NUMBER_LENGTH)
  {
    strcpy(addressBuff, address);
    // recipient length is from 1st digit to last decimal digit
    // first character must be a decimal digit or +
    if (*address == '+')
      address++;
    if (isdigit(*address))
    {
      addressLength = 0;
      while (isdigit(*address++))
        addressLength++;
      rc = true;
    }
  }
  recvalid = rc;
  return rc;
}

bool PDU::setMessage(const char *mes)
{
  bool rc = false;
  mesvalid = rc;
  return rc;
}

bool PDU::setCharSet(enum eDCS DCS)
{
  bool rc =  (DCS == ALPHABET_7BIT) || (DCS == ALPHABET_16BIT);  // 8 bit not handled
  csvalid = rc;
  if (rc)
    dcs = DCS;
  return rc;
}

void PDU::stringToBCD(const char *number, uint8_t *pdu)
{
  int j, targetindex=0;
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
      pdu[targetindex] &= 0xf0;  // clear lower
      pdu[targetindex] += *number++ - '0';
    }
  }
}

int PDU::convert_ascii_to_7bit(const char *ascii, char *a7bit) {
  int r;
  int w;

  r = 0;
  w = 0;
  while (ascii[r] != 0) {
    //    lu = pgm_read_word_near(lookup_ascii8to7+(uint8_t)ascii[r]);
    //    Serial.print("lu: " );
    //    Serial.println(lu,HEX);
#ifdef PM
    if (pgm_read_word_near(lookup_ascii8to7 + (uint8_t)ascii[r])<256)
#else
    if (lookup_ascii8to7[(uint8_t)ascii[r]]<256)
#endif
    {
#ifdef PM
      int16_t x = (int16_t)pgm_read_word_near(lookup_ascii8to7 + (uint8_t)ascii[r]);
#else
      int16_t x = (int16_t)lookup_ascii8to7[(uint8_t)ascii[r]];
#endif
      a7bit[w++] = abs(x);
    }
    else
    {
      a7bit[w++] = 27;
#ifdef PM
      a7bit[w++] = pgm_read_word_near(lookup_ascii8to7 + (uint8_t)ascii[r]) - 256;
#else
      a7bit[w++] = lookup_ascii8to7[(uint8_t)ascii[r]] - 256;
#endif
    }
    r++;
  }
  return w;
}

int PDU::ascii_to_pdu(const char *ascii, uint8_t *pdu)
{
  int r;
  int w;
  char ascii7bit[70];
  int  len7bit;

  /* Start by converting the ISO-string to a 7bit-string */
  len7bit = convert_ascii_to_7bit(ascii, ascii7bit);

  /* Now, we can create a PDU string by packing the 7bit-string */
  r = 0;
  w = 0;
  while (r<len7bit) {
    pdu[w] = ((ascii7bit[r] >> (w % 7)) & 0x7F) | ((ascii7bit[r + 1] << (7 - (w % 7))) & 0xFF);
    if ((w % 7) == 6) r++;
    r++;
    w++;
  }
  return w;
}

int PDU::encodePDU(unsigned char *pdubuffer)
{
  int sourceIndex = 0, index = 0, length;
  uint8_t tempbuffer[200]; // more than enough for larget message
  // sanity check that all items were set
  if (recvalid /*&& mesvalid*/ && csvalid)
  {
    // start building the sms-submit structure
    pdubuffer[index++] = 0;  // no sca info
    pdubuffer[index++] = PSU_SMS_SUBMIT;
    pdubuffer[index++] = 0; // MR
    pdubuffer[index++] = addressLength;
    pdubuffer[index++] = *addressBuff == '+' ? INTERNATIONAL_NUMBER : NATIONAL_NUMBER;
    // now convert recipient to nibble swapped BCD format
    if (*addressBuff == '+')
      sourceIndex++;
    stringToBCD(&addressBuff[sourceIndex], &pdubuffer[index]);
    index += (addressLength + 1) / 2;
    // PID
    pdubuffer[index++] = 0;
    // DCS
    pdubuffer[index++] = dcs << DCS_ALPHABET_OFFSET; // all other bits zero
    // No VP, now endcode message
    switch (dcs)
    {
      case ALPHABET_7BIT:
//        length = ascii_to_pdu(mesbuff, tempbuffer);
//        pdubuffer[index++] = strlen(mesbuff);
        length = ascii_to_pdu("7 bit", tempbuffer);
        pdubuffer[index++] = strlen("7 bit");
        break;
      case ALPHABET_8BIT:
        length = ascii_to_pdu("8 bit", tempbuffer);
        pdubuffer[index++] = strlen("8 bit");
        break;
      case ALPHABET_16BIT:
        length = ascii_to_pdu("16 bit", tempbuffer);
        pdubuffer[index++] = strlen("16 bit");
        break;
    }
    memcpy(&pdubuffer[index], tempbuffer, length);
    index += length;
  }
  return index;
}

// creates an buffer in SMS SUBMIT format and returns length, -1 if invalid in anyway
int PDU::encodePDU(unsigned char *pdubuffer,const char *recipient, const char *message, enum eDCS dcs)
{
  int length = -1;
  bool rc = setAddress(recipient,NUMERIC);
 // rc &= setMessage(message);
  rc &= setCharSet(dcs);
  if (rc)
    length = encodePDU(pdubuffer);
  return length;
}

int PDU::encodePDU(unsigned char *pdubuffer, const char *recipient,const char *message)
{
  return encodePDU(pdubuffer, recipient, message, ALPHABET_7BIT);
}

uint8_t PDU::gethex(const char *pc)
{
  int i;
  if (isdigit(*pc))
    i = (uint8_t(*pc) - '0') * 16;
  else
    i = (uint8_t(*pc) - 'A' + 10) * 16;
  pc++;
  if (isdigit(*pc))
    i += (uint8_t(*pc) - '0');
  else
    i += (uint8_t(*pc) - 'A' + 10);
  return i;
}
/*
    length is in octets, output buffer ucs2 must be big enough to receive the results
*/
int PDU::pdu_to_ucs2(const char *pdu, int length, uint16_t *ucs2) {
  uint16_t temp;
  int indexOut = 0;
  int octet = 0;
  unsigned char X;
  while (octet < length) {
    temp = 0;
    X = gethex(pdu);
    pdu+=2;  // skip 2 chars
    octet++;
    temp = X<<8;   // BE or LE ?
    X = gethex(pdu);
    pdu+=2;
    octet++;
    temp |= X;   // BE or LE ?
    ucs2[indexOut++] = temp;
  }
  return indexOut;
}


int PDU::convert_7bit_to_ascii(uint8_t *a7bit, int length, char *ascii) {
  int     r;
  int     w;

  w = 0;
  for (r = 0; r<length; r++) {
#ifdef PM
      if ((pgm_read_byte(lookup_ascii7to8 + a7bit[r]) != 27)) {
        ascii[w++] = pgm_read_byte(lookup_ascii7to8 + (unsigned char)a7bit[r]);
#else
      if ((lookup_ascii7to8[(unsigned char)a7bit[r]]) != 27) {
        ascii[w++] = lookup_ascii7to8[(unsigned char)a7bit[r]];
#endif
    }
    else {
      /* If we're escaped then the next uint8_t have a special meaning. */
      r++;
      switch (a7bit[r]) {
      case    10:
        ascii[w++] = 12;
        break;
      case    20:
        ascii[w++] = '^';
        break;
      case    40:
        ascii[w++] = '{';
        break;
      case    41:
        ascii[w++] = '}';
        break;
      case    47:
        ascii[w++] = '\\';
        break;
      case    60:
        ascii[w++] = '[';
        break;
      case    61:
        ascii[w++] = '~';
        break;
      case    62:
        ascii[w++] = ']';
        break;
      case    64:
        ascii[w++] = '|';
        break;
      default:
        ascii[w++] = NPC8;
        break;
      }
    }
  }

  /* Terminate the result string */
  ascii[w] = 0;

  return w;
}

int PDU::pdu_to_ascii(const char *pdu, int pdulength, char *ascii) {
  int   r;
  int   w;
  int   length;
  unsigned char ascii7bit[40];

  w = 0;
  int index = 0;   // index into the string
  int ovflow = 0;
  for (r = 0; r<pdulength; r++) {
    index = r * 2;
    if (r % 7 == 0) {
      ascii7bit[w++] = (gethex(&pdu[index]) << 0) & 0x7F;
    }
    else if (r % 7 == 6) {
      ascii7bit[w++] = ((gethex(&pdu[index]) << 6) | (gethex(&pdu[index - 2]) >> 2)) & 0x7F;
      ascii7bit[w++] = (gethex(&pdu[index]) >> 1) & 0x7F;
      ovflow++;
    }
    else {
      ascii7bit[w++] = ((uint8_t)(gethex(&pdu[index]) << (r % 7)) | (uint8_t)(gethex(&pdu[index - 2]) >> (7 + 1 - (r % 7)))) & 0x7F;
    }
  }

  length = convert_7bit_to_ascii(ascii7bit, w - ovflow, ascii);

  return length;
}

bool PDU::decodePDU(const char *pdu)
{
  bool rc = true;
  //char buff[20];
  int index = 0;
  int outindex = 0;
  int i, pid, dcs;
  unsigned char X;
  X = gethex(pdu);
  decodeAddress(pdu,scabuff,OCTETS);
  index = (X*2)+2;  // skip over SCA
  X = gethex(&pdu[index]);
  index += 2;
//  std::cout << "SMS Deliver " << X;
  X = gethex(&pdu[index]);
  decodeAddress(&pdu[index],senderbuff,NIBBLES);
  index += X + 4; // skip over sender number
  pid = gethex(&pdu[index]);
  index += 2;
//  std::cout << "PID " << pid;
  dcs = gethex(&pdu[index]);
  index += 2;
//  std::cout << "DCS " << dcs;
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
  X = gethex(&pdu[index]);
  index += 2;
  int dulength = X;   // in octets
//  std::cout << "msg length " << dulength;
  int utflength = 0,utfoffset;
  unsigned short ucs2;
  *mesbuff = 0;
  switch (dcs & DCS_ALPHABET_MASK)
  {
    case DCS_7BIT_ALPHABET_MASK:
      outindex = 0;
      i = pdu_to_ascii(&pdu[index], dulength, (char *)mesbuff);
      mesbuff[i] = 0;
      meslength = i;
      rc = true;
      break;
    case DCS_8BIT_ALPHABET_MASK:
      std::cout << "8 bit not supported\n";
      rc = false;
      break;
    case DCS_16BIT_ALPHABET_MASK:
      // loop on all ucs2 words until done
      utfoffset = 0;
      while (dulength) {
        pdu_to_ucs2(&pdu[index],2,&ucs2); // treat 2 octets
  //      Serial.println(ucs2,HEX);
        index += 4;
        dulength -=2;
        utflength = ucs2_to_utf8(ucs2,mesbuff + utfoffset);
        utfoffset += utflength;
      }
      mesbuff[++utfoffset] = 0;  // end marker
      meslength = utfoffset;
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

#define BITS765ON   0B11100000
#define BITS76ON    0B11000000
#define BIT7ON6OFF  0B10000000
#define BITS0TO5ON  0B00111111

int PDU::ucs2_to_utf8(unsigned short ucs2, unsigned char *outbuf)
{
    if (/*ucs2>=0 and*/ ucs2 <= 0x7f)  // 7F(16) = 127(10)
    {
        outbuf[0] = ucs2;
        return 1;
    }
    else if (ucs2 <= 0x7ff)  // 7FF(16) = 2047(10)
    {
        unsigned char c1 = BITS76ON, c2 = BIT7ON6OFF;

        for (int k=0; k<11; ++k)
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
    else // if (ucs2 <= 0xffff)  // FFFF(16) = 65535(10)
    {
        unsigned char c1 = BITS765ON, c2 = BIT7ON6OFF, c3 = BIT7ON6OFF;

        for (int k=0; k<16; ++k)
        {
            if (k < 6)  c3 |= (ucs2 % 64) & (1 << k);
            else if (k < 12) c2 |= (ucs2 >> 6) & (1 << (k - 6));
            else c1 |= (ucs2 >> 12) & (1 << (k - 12));
        }
        outbuf[0] = c1;
        outbuf[1] = c2;
        outbuf[2] = c3;

        return 3;
    }

    return 0;
}

int PDU::utf8Length(unsigned char *utf8) {
    int length = 1;
    unsigned char mask = BITS76ON;
    // look for ascii 7 on 1st byte
    if ((*utf8 & BIT7ON6OFF) == 0)
        ;
    else {
        // look for length pattern on first byte - 2 r more continuous 1's
        while ((*utf8 & mask) == mask) {
                length++;
                mask = (mask>>1 | BIT7ON6OFF);
        }
        if (length > 1) { // validate continuation bytes
            int LEN = length-1;  
            utf8++;
            while (LEN) {
                if ((*utf8++ & BITS76ON) == BIT7ON6OFF)
                    LEN--;
                else
                    break;
            }
            if (LEN != 0)
                length = -1;
        }
        else
            length = -1;    //
    }
    return length;
}

unsigned short PDU::utf8_to_ucs2(unsigned char *utf8) {
    unsigned short ucs2 = 0;
    int cont = utf8Length(utf8)-1; // number of continuation bytes
    if (cont < 0)
        return ucs2;
    if (cont == 0) {       // ascii 7 bit
        ucs2 = *utf8;
    }
    else {
        // read n bits of first byte then 6 bits of each continuation
        unsigned char mask = BITS0TO5ON;
        int temp = cont;
        while (temp-- > 0)
            mask >>= 1;
        ucs2 = *utf8++ & mask;
        // add continuation bytes
        while (cont-- > 0) {
            ucs2 = (ucs2<<6) | (*(utf8++) & BITS0TO5ON);
        }
    }
    return ucs2;
}

const char *PDU::getSCA() {
  return scabuff;
}
const char *PDU::getSender() {
  return senderbuff;
}
const char *PDU::getTimeStamp() {
  return tsbuff;
}
const unsigned char *PDU::getText() {
  return mesbuff;
}

int PDU::getUtf8Length() {
  return meslength;
}

void PDU::BCDtoString(char *output, const char *input,int length) {
  uint8_t X;
  for (int i = 0; i < length; i += 2)
  {
    X = gethex(input);
    input += 2;
    *output++ = (X & 0xf) + 0x30;
    if ((X & 0xf0) == 0xf0)  // end filler
      break;
    *output++ = (X >> 4) + 0x30;
  }
  *output = 0;  // add end of string
}

bool PDU::decodeAddress(const char *pdu,char *output,eLengthType et) {  // pdu to readable starts with length octet
  bool rc = false;
  int length = gethex(pdu);   // could be nibbles or octets
  // if octets, length include TON so reduce by 1
  // if nibbles length is just the number
  if (et == NIBBLES)
    addressLength = length;
  else
    addressLength = --length * 2;
  pdu += 2;   // gethex reads 2 bytes
  // now analyse address type
  int adt = gethex(pdu);
  pdu += 2;
  if (adt & EXT_MASK == 0) {
    // dont know how to handle this
    return rc;
  }
  switch ((adt & TON_MASK) >> TON_OFFSET) {
    case 1:  // international number
      *output++ = '+';  // add prefix and fall through
    case 2:  // national number
      BCDtoString(output,pdu,addressLength);
      break;
    case 5: // alphabetic
      break;
    default:
      return rc;
  }
}