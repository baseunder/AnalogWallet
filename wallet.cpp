#include "wallet.h"
#include "bckp.h"
#include "random.h"
#include <uECC.h>
#include <EEPROM.h>

uint8_t private1[32];
uint8_t public1[64];
uint8_t stat;
#define initBitPos 64 // semi-pri
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
}
void writeStatus(){
  writeID();
  Serial.write(checkCard());
}
void writeID()
{
  for (int i = 32; i < 64; i++){
    Serial.write(EEPROM.read(i));
  }
}
uint8_t initDevice(byte *passw)
{
  uint8_t tbuffer[32];
  Serial.readBytes(tbuffer, 32); // wallet id bytes
  if (EEPROM.read(initBitPos) == 1)
  {
    return 8;
  }
  for (int i = 0; i < 32; i++){
    updateSHA(passw[i]);
    updateSHA(tbuffer[i]);
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
    EEPROM.update(i+32, tbuffer[i]);
  }
  stat = checkBackup(private1, public1);
  if (stat)return stat;
  EEPROM.update(initBitPos, 1);
  walletstart(passw);
  eccTest();
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
  uint8_t sig[64];
  byte opres = 0;
  byte tc = 0;
  do {
    opres = uECC_sign(private1, hash, 32, sig, uECC_secp256k1());
  } while ( opres==0 || ((sig[0]<0x80)||(sig[32]<0x80)));
  Serial.write(2);
  Serial.write(sig, 64);
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
