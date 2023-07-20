#include "random.h"
#include "wallet.h"
uint8_t pass[32] = {0};
uint8_t hash[32] = {0};

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);
  setRNG();
  flickrTest();
  while (!Serial)  { }
}
void loop() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    if (cmd == "init") {
      while (!Serial.available());
      Serial.readBytes(pass, 32);
      if (initDevice(pass)){
        writePublicKey();
      }
    } else if (cmd == "version") {
      Serial.println("V1.0");
    } else if (cmd == "open") {
      while (!Serial.available());
      Serial.readBytes(pass, 32);
      walletstart(pass);
    } else if (cmd == "erase") {
      eraseDevice();
      Serial.println("12"); //"Erase done"
    } else if (cmd == "sign") {
        Serial.println("hash:");
        while (!Serial.available());
        Serial.readBytes(hash, 32);
        sign(hash);
    } else if (cmd == "restore") {
      restore(Serial.readStringUntil('\n'));
    } else if (cmd == "test") {
      Serial.println(eccTest());
    } else if (cmd == "rnd") {
      while(1){
        Serial.write(getTrueRotateRandomByte());
        }
    } else {
      Serial.println("0");
    }
  }
  getTrueRotateRandomByte();
}
