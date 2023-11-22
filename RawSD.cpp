#include "RawSD.h"

RawSD::RawSD(uint8_t csPin) : _csPin(csPin) {}

bool RawSD::begin() {
    return _card.init(SPI_QUARTER_SPEED, _csPin);
}

bool RawSD::writeBlock(uint32_t blockNumber, const uint8_t* data) {
    return _card.writeBlock(blockNumber, data);
}

bool RawSD::readBlock(uint32_t blockNumber, uint8_t* buffer) {
    return _card.readBlock(blockNumber, buffer);
}