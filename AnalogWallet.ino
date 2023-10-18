#include "random.h"
#include "wallet.h"

uint8_t sbuffer[32] = {0};

void waitForSerial(){
  while (!Serial.available()){getTrueRotateRandomByteWithSHAupdate();}
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  setRNG();
  Serial.begin(115200);
  while (!Serial){getTrueRotateRandomByteWithSHAupdate();}
}
void loop()
{
  waitForSerial();
  int cmd = Serial.read();
  if (cmd==0){ // init
      waitForSerial();
      Serial.readBytes(sbuffer, 32);
      Serial.write(initDevice(sbuffer));
  }
  if (cmd==1){ // open
      waitForSerial();
      Serial.readBytes(sbuffer, 32);
      Serial.write(walletstart(sbuffer));
  }
  if (cmd==2){ // erase
      eraseDevice();
      Serial.write((uint8_t)0);
  }
  if (cmd==3){ // sign
      waitForSerial();
      Serial.readBytes(sbuffer, 32);
      Serial.write(sign(sbuffer));
      while (1){}
  }
  if (cmd==4){ // restore
      waitForSerial();
      Serial.write(restore());
      while (1){}
  }
  if (cmd==5){ // rnd
      while (1)
      {
        Serial.write(getTrueRotateRandomByte());
      }
  }
  delay(250);
}
