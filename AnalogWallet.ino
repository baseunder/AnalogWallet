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
  if (Serial.available() > 0)
  {
    int cmd = Serial.read();
    switch (cmd){
      case 0: // init
        while (!Serial.available()){}
        Serial.readBytes(pass, 32);
        if (initDevice(pass))
        {
          writePublicKey();
        }
      case 1: // version
        Serial.println("V1.0");
      case 2: // open
        while (!Serial.available()){}
        Serial.readBytes(pass, 32);
        walletstart(pass);
      case 3: // erase
        eraseDevice();
        Serial.write(12); //"Erase done"
      case 4: // sign
        Serial.println("hash:");
        while (!Serial.available()){}
        Serial.readBytes(hash, 32);
        sign(hash);
        while (1){}
      case 5: // restore
        restore(Serial.readStringUntil('\n'), pass);
      case 6: // test
        eccTest();
      case 7: // rnd
        while (1)
        {
          Serial.write(getTrueRotateRandomByte());
        }
      case 8: // rea
        while (1)
        {
          Serial.println(getRead());
        }
    }
    delay(250);
  }
  getTrueRotateRandomByte();
}
