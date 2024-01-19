#include "integrity.h"
#include "sha256.h"
/*
extern int _etext;
extern int _edata;
extern int __ctors_start;
extern int __ctors_end;
extern int __dtors_start;
extern int __dtors_end;
extern int __trampolines_start;
extern int __trampolines_end;
extern int __data_start;
extern int __data_end;
extern int __bss_start;
extern int __bss_end;
extern int __data_load_start;
*/
extern int __data_load_end;
/*
extern int __noinit_start;
extern int __noinit_end;
extern int __heap_start;
extern int __heap_end;
extern int __eeprom_start;
extern int __eeprom_end;
*/

uint8_t getIntegrityHash(byte *seed)
{
  Sha256 integritySHA = Sha256();
  integritySHA.init();
  integritySHA.write(seed, 32);
  /*
    Serial.println(F("\nTest sketch"));
    Serial.print(F("_etext= "));
    Serial.println( (unsigned long) &_etext);
    Serial.print(F("_edata= "));
    Serial.println( (unsigned long) &_edata);
    Serial.print(F("__ctors_start= "));
    Serial.println( (unsigned long) &__ctors_start);
    Serial.print(F("__ctors_end= "));
    Serial.println( (unsigned long) &__ctors_end);
    Serial.print(F("__dtors_start= "));
    Serial.println( (unsigned long) &__dtors_start);
    Serial.print(F("__dtors_end= "));
    Serial.println( (unsigned long) &__dtors_end);
    Serial.print(F("__trampolines_start= "));
    Serial.println( (unsigned long) &__trampolines_start);
    Serial.print(F("__trampolines_end= "));
    Serial.println( (unsigned long) &__trampolines_end);
    Serial.print(F("__data_start= "));
    Serial.println( (unsigned long) &__data_start);
    Serial.print(F("__data_end= "));
    Serial.println( (unsigned long) &__data_end);
    Serial.print(F("__bss_start= "));
    Serial.println( (unsigned long) &__bss_start);
    Serial.print(F("__bss_end= "));
    Serial.println( (unsigned long) &__bss_end);
    Serial.print(F("__data_load_start= "));
    Serial.println( (unsigned long) &__data_load_start);
    Serial.print(F("__data_load_end= "));
    Serial.println( (unsigned long) &__data_load_end);
    Serial.print(F("__noinit_start= "));
    Serial.println( (unsigned long) &__noinit_start);
    Serial.print(F("__noinit_end= "));
    Serial.println( (unsigned long) &__noinit_end);
    Serial.print(F("__heap_start= "));
    Serial.println( (unsigned long) &__heap_start);
    Serial.print(F("__heap_end= "));
    Serial.println( (unsigned long) &__heap_end);
    // Serial.print(F("__eeprom_start= "));
    // Serial.println( (unsigned long) &__eeprom_start, HEX);
    Serial.print(F("__eeprom_end= "));
    Serial.println( (unsigned long) &__eeprom_end);
  */
  unsigned long address;
  for (address = 0; address < (unsigned long)&__data_load_end; address++) {
    integritySHA.write(pgm_read_byte_near(address));
  }
  Serial.write((uint8_t)19);
  Serial.write(integritySHA.result(),32);
  Serial.write((uint8_t)0);
}