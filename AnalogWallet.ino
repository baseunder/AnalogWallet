#include "random.h"
#include "wallet.h"
uint8_t pass[32] = {0};
uint8_t hash[32] = {0};

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  setRNG();
  Serial.begin(115200);
  while (!Serial)
  {
    getTrueRotateRandomByte();
  }
}
void loop()
{
  while (Serial.available() == 0) {
    getTrueRotateRandomByte();
  }
  int cmd = Serial.read();
  if (cmd==0){ // init
      while (!Serial.available()){}
      Serial.readBytes(pass, 32);
      uint8_t initSt = initDevice(pass);
      Serial.write(initSt);
      if (initSt==0)
      {
        writePublicKey();
      }
  }
  if (cmd==1){ // version
      Serial.println("V1.0");
  }
  if (cmd==2){ // open
      while (!Serial.available()){}
      Serial.readBytes(pass, 32);
      walletstart(pass);
  }
  if (cmd==3){ // erase
      eraseDevice();
      Serial.write(12); //"Erase done"
  }
  if (cmd==4){ // sign
      while (!Serial.available()){}
      Serial.readBytes(hash, 32);
      sign(hash);
      while (1){}
  }
  if (cmd==5){ // restore
      Serial.write(restore(Serial.readStringUntil('\n'), pass));
  }
  if (cmd==6){ // test
      eccTest();
  }
  if (cmd==7){ // rnd
      while (1)
      {
        Serial.write(getTrueRotateRandomByte());
      }
  }
  if (cmd==8){ // rea
      while (1)
      {
        Serial.println(getRead());
      }
  }
  if (cmd==9){ // blink
      digitalWrite(LED_BUILTIN, HIGH);
      delay(1000);
      digitalWrite(LED_BUILTIN, LOW);
      delay(1000);
  }
  delay(250);
}
