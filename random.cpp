#include "random.h"
byte lastByte = 0;
byte leftStack = 0;
byte rightStack = 0;
int PINS[] = {A0,A1,A2,A3};
int randomPin=0;
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
  ADMUX = (ADMUX & 0xF0) | (PINS[randomPin] & 0x0F); // select A0
  ADCSRA |= (1 << ADSC);               // Start the ADC conversion
  while (ADCSRA & (1 << ADSC)); // Wait for the conversion to complete
  ADCSRA &= ~(1 << ADEN);
  return ADC;
}

void flickrTest()
{
  // Disable the analog comparator
  ACSR |= (1 << ACD);
  // Set the gain amplifier for A0 to A3 to 200x
  ADMUX |= (1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0);
  // Set the ADC clock to the fastest (prescaler of 2)
  ADCSRA |= (1 << ADPS2);
  ADCSRA |= (1 << ADPS1);
  ADCSRA |= (1 << ADPS0);
  // Set reference voltage to external (AREF)
  // ADMUX &= ~(1 << REFS1);
  ADMUX |= (1 << REFS1);
  ADMUX |= (1 << REFS0);
  // Disab ADC Noise Reduction Mode
  SMCR &= ~(1 << SM2);
  // Disable free running mode
  ADCSRA &= ~(1 << ADATE);
  // Disable auto triggering
  ADCSRA &= ~(1 << ADIE);
  int maxVal = 0;
  int maxPin = 0;
  digitalWrite(LED_BUILTIN, HIGH);
  while (maxVal<16){
    for (int i = 0; i < 4; i++)
    {
      randomPin = i;
      int vonneumann = 0;
      for (int i = 0; i < 64; i++)
      {
        int leftBits = getRead();
        int rightBits = getRead();
        if (leftBits != rightBits)
        {
          vonneumann++;
        }
      }
      if (vonneumann>maxVal){
        maxVal = vonneumann;
        maxPin = i;
      }
    }
  }
  randomPin = maxPin;
  digitalWrite(LED_BUILTIN, LOW);
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
