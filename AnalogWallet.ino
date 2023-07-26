#include "random.h"
#include "wallet.h"
uint8_t buffer[32] = {0};

void waitForSerial(){
  while (!Serial.available()){getTrueRotateRandomByte();}
}
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  setRNG();
  Serial.begin(115200);
  waitForSerial();
}
void loop()
{
  waitForSerial();
  int cmd = Serial.read();
  if (cmd==0){ // init
      waitForSerial();
      Serial.readBytes(buffer, 32);
      uint8_t initSt = initDevice(buffer);
      Serial.write(initSt);
      if (initSt==0)
      {
        writePublicKey();
      }
  }
  if (cmd==1){ // open
      waitForSerial();
      Serial.readBytes(buffer, 32);
      walletstart(buffer);
  }
  if (cmd==2){ // erase
      eraseDevice();
      Serial.write(12); //"Erase done"
  }
  if (cmd==3){ // sign
      waitForSerial();
      Serial.readBytes(buffer, 32);
      sign(buffer);
      while (1){}
  }
  if (cmd==4){ // restore
      waitForSerial();
      Serial.readBytes(buffer, 32);
      Serial.write(restore(Serial.readStringUntil('\n'), buffer));
  }
  if (cmd==5){ // rnd
      while (1)
      {
        Serial.write(getTrueRotateRandomByte());
      }
  }
  delay(250);
}
