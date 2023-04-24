#include "g711.h"

//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************
static unsigned char alaw_encode(short pcm16);
static int alaw_decode(unsigned char alaw);
static unsigned char ulaw_encode(short pcm16);
static int ulaw_decode(unsigned char alaw);
static unsigned char alaw_to_ulaw(unsigned char alaw);
static unsigned char ulaw_to_alaw(unsigned char ulaw);


//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************
unsigned int audio_alaw_encode(unsigned char *dst, short *src, unsigned int srcSize)
{
    srcSize >>= 1;
    unsigned char* end = dst+srcSize;
    while(dst<end)
        *dst++ = alaw_encode(*src++);

    return srcSize;
}

unsigned int audio_alaw_decode(short *dst, const unsigned char *src, unsigned int srcSize)
{
    short* end = dst+srcSize;
    while(dst<end)
        *dst++ = alaw_decode(*src++);
    return srcSize<<1;
}

unsigned int audio_ulaw_encode(unsigned char *dst, short *src, unsigned int srcSize)
{
    srcSize >>= 1;
    unsigned char* end = dst+srcSize;
    while(dst<end)
        *dst++ = ulaw_encode(*src++);
    return srcSize;
}


unsigned int audio_ulaw_decode(short *dst, const unsigned char *src, unsigned int srcSize)
{
    short* end = dst+srcSize;
    while(dst<end)
        *dst++ = ulaw_decode(*src++);
    return srcSize<<1;
}

unsigned int audio_alaw_to_ulaw(unsigned char *dst, const unsigned char *src, unsigned int srcSize)
{
    unsigned char* end = dst+srcSize;
    while(dst<end)
        *dst++ = alaw_to_ulaw(*src++);
    return srcSize;
}


unsigned int audio_ulaw_to_alaw(unsigned char *dst, const unsigned char *src, unsigned int srcSize)
{
    unsigned char* end = dst+srcSize;
    while(dst<end)
        *dst++ = ulaw_to_alaw(*src++);
    return srcSize;
}


//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************
static unsigned char alaw_encode(short pcm16)
{
    int p = pcm16;
    unsigned a;  // A-law value we are forming
    if(p<0)
    {
        // -ve value
        // Note, ones compliment is used here as this keeps encoding symetrical
        // and equal spaced around zero cross-over, (it also matches the standard).
        p = ~p;
        a = 0x00; // sign = 0
    }
    else
    {
        // +ve value
        a = 0x80; // sign = 1
    }

    // Calculate segment and interval numbers
    p >>= 4;
    if(p>=0x20)
    {
        if(p>=0x100)
        {
            p >>= 4;
            a += 0x40;
        }
        if(p>=0x40)
        {
            p >>= 2;
            a += 0x20;
        }
        if(p>=0x20)
        {
            p >>= 1;
            a += 0x10;
        }
    }
    // a&0x70 now holds segment value and 'p' the interval number

    a += p;  // a now equal to encoded A-law value

    return a^0x55;  // A-law has alternate bits inverted for transmission
}

static int alaw_decode(unsigned char alaw)
{
    alaw ^= 0x55;  // A-law has alternate bits inverted for transmission

    unsigned sign = alaw&0x80;

    int linear = alaw&0x1f;
    linear <<= 4;
    linear += 8;  // Add a 'half' bit (0x08) to place PCM value in middle of range

    alaw &= 0x7f;
    if(alaw>=0x20)
    {
        linear |= 0x100;  // Put in MSB
        unsigned shift = (alaw>>4)-1;
        linear <<= shift;
    }

    if(!sign)
        return -linear;
    else
        return linear;
}

static unsigned char ulaw_encode(short pcm16)
{
    int p = pcm16;
    unsigned u;  // u-law value we are forming

    if(p<0)
    {
        // -ve value
        // Note, ones compliment is used here as this keeps encoding symetrical
        // and equal spaced around zero cross-over, (it also matches the standard).
        p = ~p;
        u = 0x80^0x10^0xff;  // Sign bit = 1 (^0x10 because this will get inverted later) ^0xff to invert final u-Law code
    }
    else
    {
        // +ve value
        u = 0x00^0x10^0xff;  // Sign bit = 0 (-0x10 because this amount extra will get added later) ^0xff to invert final u-Law code
    }

    p += 0x84; // Add uLaw bias

    if(p>0x7f00)
        p = 0x7f00;  // Clip to 15 bits

    // Calculate segment and interval numbers
    p >>= 3;    // Shift down to 13bit
    if(p>=0x100)
    {
        p >>= 4;
        u ^= 0x40;
    }
    if(p>=0x40)
    {
        p >>= 2;
        u ^= 0x20;
    }
    if(p>=0x20)
    {
        p >>= 1;
        u ^= 0x10;
    }
    // (u^0x10)&0x70 now equal to the segment value and 'p' the interval number (^0x10)

    u ^= p; // u now equal to encoded u-law value (with all bits inverted)

    return u;
}


static int ulaw_decode(unsigned char ulaw)
{
    ulaw ^= 0xff;  // u-law has all bits inverted for transmission

    int linear = ulaw&0x0f;
    linear <<= 3;
    linear |= 0x84;  // Set MSB (0x80) and a 'half' bit (0x04) to place PCM value in middle of range

    unsigned shift = ulaw>>4;
    shift &= 7;
    linear <<= shift;

    linear -= 0x84; // Subract uLaw bias

    if(ulaw&0x80)
        return -linear;
    else
        return linear;
}

static unsigned char alaw_to_ulaw(unsigned char alaw)
{
    unsigned char sign=alaw&0x80;
    alaw ^= sign;
    alaw ^= 0x55;
    unsigned ulaw;
    if(alaw<45)
    {
        if(alaw<24)
            ulaw = (alaw<8) ? (alaw<<1)+1 : alaw+8;
        else
            ulaw = (alaw<32) ? (alaw>>1)+20 : alaw+4;
    }
    else
    {
        if(alaw<63)
            ulaw = (alaw<47) ? alaw+3 : alaw+2;
        else
            ulaw = (alaw<79) ? alaw+1 : alaw;
    }
    ulaw ^= sign;
    return ulaw^0x7f;
}


static unsigned char ulaw_to_alaw(unsigned char ulaw)
{
    unsigned char sign=ulaw&0x80;
    ulaw ^= sign;
    ulaw ^= 0x7f;
    unsigned alaw;
    if(ulaw<48)
    {
        if(ulaw<=32)
            alaw = (ulaw<=15) ? ulaw>>1 : ulaw-8;
        else
            alaw = (ulaw<=35) ? (ulaw<<1)-40 : ulaw-4;
    }
    else
    {
        if(ulaw<=63)
            alaw = (ulaw==48) ? ulaw-3 : ulaw-2;
        else
            alaw = (ulaw<=79) ? ulaw-1 : ulaw;
    }
    alaw ^= sign;
    return alaw^0x55;
}


