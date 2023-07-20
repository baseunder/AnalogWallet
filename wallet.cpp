#include "wallet.h"
#include "bckp.h"
#include "random.h"
#include <uECC.h>
#include <EEPROM.h>

uint8_t private1[32];
uint8_t public1[64];

const struct uECC_Curve_t * curve = uECC_secp256k1();
#define initBitPos 32+64 //pri pub

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
  //ADMUX &= ~(1 << REFS1);
  ADMUX |= (1 << REFS1);
  ADMUX |= (1 << REFS0);
  // Disab ADC Noise Reduction Mode
  SMCR &= ~(1 << SM2);  
  // Disable free running mode
  ADCSRA &= ~(1 << ADATE);
  // Disable auto triggering
  ADCSRA &= ~(1 << ADIE);
  uECC_set_rng(&RNG);
}
void writePublicKey(){
  Serial.write(public1, 64);
}
bool initDevice(byte passw[]){
  if (EEPROM.read(initBitPos)==1){
    Serial.println("19"); //"device already initialized"
    return false;
  }
  unsigned long a = millis();
  private1[0] = 0;
  while (private1[0] == 0){
    uECC_make_key(public1, private1, curve);
  }
  for (int i = 0; i < 32; i++) {
    private1[i] += passw[i];
  }
  Serial.print("Made key 1 in "); Serial.println(millis()-a);
  if (!initBackup(private1, public1)){
    Serial.println("check SD card");
    return false;
  }
  EEPROM.update(initBitPos, 0);
  for (int i = 0; i < 32; i++) {
    EEPROM.update(i, private1[i]);
  }
  for (int i = 0; i < 64; i++) {
    EEPROM.update(i+32, public1[i]);
  }
  EEPROM.update(initBitPos, 1);
  walletstart(passw);
  if (checkBackup(private1, public1)){
      for(int i = 0; i < 64; i++) {
        if(public1[i] < 0x10) {
          Serial.print('0');
        }
        Serial.print(String(public1[i], HEX));
      }
      Serial.println();
      return true;
  }else{
    return false;
  }
}

void walletstart(byte passw[]){
  delay(1000);
  for (int i = 0; i < 32; i++) {
    if (EEPROM.read(i+96) != passw[i]){
      Serial.println("18"); //"ERROR wrong password"
      while(1);
      }
  }
  for (int i = 0; i < 32; i++) {
    private1[i] = EEPROM.read(i)-passw[i];
  }
  uint8_t public2[64];
  uECC_compute_public_key(private1, public2, curve);
  for (int i = 0; i < 64; i++) {
    public1[i] = EEPROM.read(i+32);
    if(public1[i]!=public2[i]){
      Serial.print("ERROR on public key and private/pass restore ");
    }
  }
}

bool eccTest(){
  if (EEPROM.read(initBitPos)==0){
    Serial.println("16"); //Device state -> INIT not done
    return false;
  }
  Serial.println("17"); //Device state -> INIT ok
  uint8_t hash[32] = {0};
  memcpy(hash, public1, 32);
  sign(hash);
}
void eraseDevice(){
  for (int i = 0; i <= initBitPos; i++) {
    EEPROM.update(i, 0);
  }
  for (int i = 0; i < 32; i++) {
    private1[i] = 0;
    public1[i] = 0;
  }
}
void sign(uint8_t *hash){
  uint8_t sig[64] = {0};
  if (!uECC_sign(private1, hash, 32, sig, curve)){
    Serial.println("15"); //not able to sign...
  }else{
    for(int i = 0; i < 64; i++) {
      if(sig[i] < 0x10) {
        Serial.print('0');
      }
      Serial.print(String(sig[i], HEX));
    }
    Serial.println();
  }
}
void restore(String fn){
  if (restoreBackup(private1, fn)){
    Serial.println("restore done:");
    for(int i = 0; i < 64; i++) {
      if(public1[i] < 0x10) {
        Serial.print('0');
      }
      Serial.print(String(public1[i], HEX));
    }    
    Serial.println();
  }else{
    Serial.println("14"); //restore ERROR
  }
}

