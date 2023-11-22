#ifndef RawSD_h
#define RawSD_h

#include <Arduino.h>
#include <SD.h>

class RawSD {
public:
    RawSD(uint8_t csPin);
    bool begin();
    bool writeBlock(uint32_t blockNumber, const uint8_t* data);
    bool readBlock(uint32_t blockNumber, uint8_t* buffer);

private:
    uint8_t _csPin;
    Sd2Card _card;
};

#endif
