#include "wallet.h"
#include "bckp.h"
#include "random.h"
#include "uECC.h"
#include <EEPROM.h>

uint8_t private1[32];
uint8_t public1[64];
uint8_t stat;
#define initBitPos 32 // semi-pri
bool isOpen;

void setRNG()
{

  flickrTest();
  uECC_set_rng(&RNG);
}
void writePublicKey()
{
  Serial.write(1);
  Serial.write(public1, 64);
  eccTest();
}
uint8_t initDevice(byte passw[])
{
  if (EEPROM.read(initBitPos) == 1)
  {
    return 8;
  }
  stat = checkCard();
  if (stat) return stat;
  private1[0] = 0;
  while (private1[0] == 0)
  {
    uECC_make_key(public1, private1, uECC_secp256k1());
  }
  for (int i = 0; i < 32; i++)
  {
    private1[i] += passw[i];
  }
  EEPROM.update(initBitPos, 0);
  stat = initBackup(private1, public1);
  if (stat)return stat;
  for (int i = 0; i < 32; i++)
  {
    EEPROM.update(i, private1[i]);
  }
  stat = checkBackup(private1, public1);
  if (stat)return stat;
  EEPROM.update(initBitPos, 1);
  walletstart(passw);
  return 0;
}
uint8_t walletstart(byte *passw)
{
  if (EEPROM.read(initBitPos) == 0)
  {
    return 12;
  }
  for (int i = 0; i < 32; i++)
  {
    private1[i] = EEPROM.read(i) - passw[i];
  }
  uECC_compute_public_key(private1, public1, uECC_secp256k1());
  writePublicKey();
  return 0;
}
uint8_t eccTest()
{
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
  Serial.write(16);
  Serial.write(hash, 32);
  uint8_t sig[65];
  int zeroCount = 64; // check if R or S is zero to prevent key exposure
  uint8_t ID_raw;
  while (zeroCount > 32){
    sig[64] = uECC_sign(private1, hash, 32, sig, uECC_secp256k1());
    sig[64]--;
    zeroCount = 0;
    for (int i = 0; i < 32; i++){
      if (sig[i]==0)zeroCount++;
      if (sig[i+32]==0)zeroCount++;
    }
  }
  Serial.write(2);
  Serial.write(sig, 65);
  return 0;
}

uint8_t restore()
{
  if (EEPROM.read(initBitPos) == 1)
  {
    return 8;
  }
  stat = restoreBackup();
  if (stat)return stat;
  EEPROM.update(initBitPos, 1);
  return 0;
}
