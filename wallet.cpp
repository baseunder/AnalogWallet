#include "wallet.h"
#include "bckp.h"
#include "random.h"
#include <uECC.h>
#include <EEPROM.h>

uint8_t private1[32];
uint8_t public1[64];
uint8_t stat;
#define initBitPos 32 // semi-pri
bool isOpen;
void setRNG()
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
  flickrTest();
  uECC_set_rng(&RNG);
}
void writePublicKey()
{
  Serial.write(public1, 64);
  eccTest();
}
uint8_t initDevice(byte passw[])
{
  if (EEPROM.read(initBitPos) == 1)
  {
    return 20;
  }
  private1[0] = 0;
  while (private1[0] == 0)
  {
    uECC_make_key(public1, private1, uECC_secp256k1());
  }
  for (int i = 0; i < 32; i++)
  {
    private1[i] += passw[i];
  }
  stat = initBackup(private1, public1);
  if (stat)return stat;
  EEPROM.update(initBitPos, 0);
  for (int i = 0; i < 32; i++)
  {
    EEPROM.update(i, private1[i]);
  }
  EEPROM.update(initBitPos, 1);
  walletstart(passw);
  stat = checkBackup(private1, public1);
  if (stat)return stat;
  return 0;
}
void walletstart(byte passw[])
{
  for (int i = 0; i < 32; i++)
  {
    private1[i] = EEPROM.read(i) - passw[i];
  }
  uECC_compute_public_key(private1, public1, uECC_secp256k1());
  writePublicKey();
}
uint8_t eccTest()
{
  if (EEPROM.read(initBitPos) == 0)
  {
    Serial.write(16); // Device state -> INIT not done
    return 30;
  }
  uint8_t hash[32];
  for (int i = 0; i < 32; i++)
  {
    hash[i] = public1[i] + public1[i + 32];
  }
  sign(hash);
  return 0;
}
void eraseDevice()
{
  for (int i = 0; i <= initBitPos; i++)
  {
    EEPROM.update(i, 0);
  }
  for (int i = 0; i < 32; i++)
  {
    private1[i] = 0;
    public1[i] = 0;
    public1[i+32] = 0;
  }
}
uint8_t sign(uint8_t *hash)
{
  uint8_t sig[64];
  uECC_sign(private1, hash, 32, sig, uECC_secp256k1());
  Serial.write(sig, 64);
  return 0;
}
uint8_t restore(String fn, byte passw[])
{
  if (restoreBackup(fn))
  {
    for (int i = 0; i < 32; i++)
    {
      private1[i] = EEPROM.read(i) - passw[i];
    }
    uECC_compute_public_key(private1, public1, uECC_secp256k1());
    writePublicKey();
    return 0;
  }
  else
  {
    return 40;
  }
}
