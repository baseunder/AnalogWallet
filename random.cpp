#include "random.h"
byte lastByte = 0;
byte leftStack = 0;
byte rightStack = 0;
byte rotate(byte b, int r)
{
  return (b << r) | (b >> (8 - r));
}
void pushLeftStack(byte bitToPush)
{
  leftStack = (leftStack << 1) ^ bitToPush ^ leftStack;
}
void pushRightStackRight(byte bitToPush)
{
  rightStack = (rightStack >> 1) ^ (bitToPush << 7) ^ rightStack;
}
int getRead()
{
  // Enable the ADC
  ADCSRA |= (1 << ADEN);
  ADMUX = (ADMUX & 0xF0) | (0 & 0x0F); // select A0
  ADCSRA |= (1 << ADSC);               // Start the ADC conversion
  while (ADCSRA & (1 << ADSC))
    ; // Wait for the conversion to complete
  ADCSRA &= ~(1 << ADEN);
  return ADC;
}

void flickrTest()
{
  int vonneumann = 0;
  for (int i = 0; i < 1000; i++)
  {
    int leftBits = getRead();
    int rightBits = getRead();
    if (leftBits != rightBits)
    {
      vonneumann++;
      digitalWrite(LED_BUILTIN, ((bool)(leftBits & 0x01)));
    }
  }
  digitalWrite(LED_BUILTIN, LOW);
  if (vonneumann < 100)
  {
    while (1)
    {
      digitalWrite(LED_BUILTIN, LOW);
      delay(500);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(500);
    }
  }
}
byte lastStack;
byte finalByte;
int leftBits;
int rightBits;
byte getTrueRotateRandomByte()
{
  finalByte = 0;
  lastStack = leftStack ^ rightStack;
  for (int i = 0; i < 4; i++)
  {
    leftBits = getRead();
    rightBits = getRead();

    finalByte ^= rotate(leftBits, i);
    finalByte ^= rotate(rightBits, 7 - i);

    for (int j = 0; j < 8; j++)
    {
      byte leftBit = (leftBits >> j) & 1;
      byte rightBit = (rightBits >> j) & 1;

      if (leftBit != rightBit)
      {
        if (lastStack % 2 == 0)
        {
          pushLeftStack(leftBit);
        }
        else
        {
          pushRightStackRight(leftBit);
        }
      }
    }
  }
  lastByte ^= (lastByte >> 3) ^ (lastByte << 5) ^ (lastByte >> 4);
  lastByte ^= finalByte;
  return lastByte ^ leftStack ^ rightStack;
}
int RNG(uint8_t *dest, unsigned size)
{
  flickrTest();
  while (size)
  {
    *dest = getTrueRotateRandomByte();
    ++dest;
    --size;
  }
  return 1;
}
