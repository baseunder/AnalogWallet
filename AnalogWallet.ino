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
  while (!Serial){getTrueRotateRandomByte();}
}
void loop()
{
  waitForSerial();
  int cmd = Serial.read();
  if (cmd==0){ // init
      waitForSerial();
      Serial.readBytes(buffer, 32);
      Serial.write(initDevice(buffer));
  }
  if (cmd==1){ // open
      waitForSerial();
      Serial.readBytes(buffer, 32);
      Serial.write(walletstart(buffer));
  }
  if (cmd==2){ // erase
      eraseDevice();
      Serial.write((uint8_t)0);
  }
  if (cmd==3){ // sign
      waitForSerial();
      Serial.readBytes(buffer, 32);
      Serial.write(sign(buffer));
      while (1){}
  }
  if (cmd==4){ // restore
      waitForSerial();
      Serial.write(restore());
  }
  if (cmd==5){ // rnd
      while (1)
      {
        Serial.write(getTrueRotateRandomByte());
      }
  }
  delay(250);
}
