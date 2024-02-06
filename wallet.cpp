#include "wallet.h"
#include "bckp.h"
#include "random.h"
#include <uECC.h>
#include <EEPROM.h>

uint8_t private1[32];
uint8_t public1[64];
uint8_t stat;
#define initBitPos 64 // semi-pri
bool setRngDone = false;
void setRNG()
{
  if (setRngDone) while(1);
  setRngDone = true;
  flickrTest();
  uECC_set_rng(&RNG);
}
void writePublicKey()
{
  Serial.write(1);
  Serial.write(public1, 64);
}
void writeStatus(){
  Serial.write((uint8_t)18);
  writeID();
  Serial.write(checkCard());
  Serial.write((uint8_t)0);
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
  if (EEPROM.read(initBitPos) == 1) return 8;
  for (int i = 0; i < 32; i++){
    updateSHA(passw[i]);
    updateSHA(tbuffer[i]);
  }
  if (stat = checkCard()) return stat;
  uECC_make_key(public1, private1, uECC_secp256k1());
  for (int i = 0; i < 32; i++)
  {
    private1[i] += passw[i];
  }
  EEPROM.update(initBitPos, 0);
  if (stat = initBackup(private1, public1))return stat;
  for (int i = 0; i < 32; i++)
  {
    EEPROM.update(i, private1[i]);
  }
  if (stat = checkBackup(private1, public1))return stat;
  for (int i = 0; i < 32; i++)
  {
    EEPROM.update(i+32, tbuffer[i]);
  }

  EEPROM.update(initBitPos, 1);
  walletstart(passw, true);
  //eccTest();
  return 0;
}
uint8_t walletstart(byte *passw, bool sdplugged)
{
  if (!sdplugged){
    if (checkCard()==0) return 6;
  }
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
uint8_t secp256k1_n[32] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xBA,0xAE,0xDC,0xE6,0xAF,0x48,0xA0,0x3B,0xBF,0xD2,0x5E,0x8C,0xD0,0x36,0x41,0x41};

uint8_t sign(uint8_t *hash)
{
  Serial.write(16);
  Serial.write(hash, 32);
  uint8_t sig[64];
  do {
  } while (uECC_sign(private1, hash, 32, sig, uECC_secp256k1())==0);
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
  if (stat = restoreBackup())return stat;
  EEPROM.update(initBitPos, 1);
  return 0;
}
