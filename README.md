# AnalogWallet

## Introduction
Inspired by the online news about a hardware wallet manufacturer that allows private key extraction via closed source firmware i decided to create my own DIY open source crypto wallet.

## Repo
The repository contains the microcontroller source code, 3D STL & Blender files for the case as well as 10MB of random numbers tested with DieHard. Since there is no hardware random number generator on the module I used, i followed the tests of bloc97 
(https://gist.github.com/bloc97/b55f684d17edd8f50df8e918cbc00f94). His 
example uses an analog input with a delay, in my solution I use the 
registers of the Mega32u4 controller to wait for a new ADC conversion. This speeds up the process of generating random numbers a lot.

## Requirements
* Arduino IDE (uECC, EEPROM and SD library)
* Arduino UNO with SD Card reader OR MEGA32u4 SD Card Module (see pictures)
* ISP Programmer (optional and only for production use)
* 3D Printer (optional, for style and electronics protection)

## Crypto
Currently all secp256k1 crypto currencies are supported, this includes for example Bitcoin, Ethereum and of course all Ethereum compatible chains.
The wallet provides plain secp256k1 elliptic curve operations ( https://github.com/kmackay/micro-ecc ):
* Private key generation (random number)
* Public key extraction (on curve)
* Hash signing (for example TX or message signature creation)
* SD Card backup
* SD Card restore

The private key is NEVER stored complete inside the EEPROM or on the SD Card, you always have to provide the correct password to complete the private key. If you enter another password you will end up with another private key that would work too but has no access to your funds.

## Interface
The wallet uses a serial port to communicate with the host system. Inside the py folder you find an example of the protocol used.

## Disclaimer
It should be used only for educational or testing purposes as default 
Arduino boards are not secure by default, their firmware can be updated from the 
computer and this process doesn't require any user interaction. 

## Security
Please upload your sketch with an ISP Programmer and be sure that the Arduino Bootloader is deleted.
Gather your own 10MB of randomness from your device and check the data with some statistic tests (for example https://github.com/nmondal/diehard.c ).
After each "sign" command the device blocks itself and you have to unplug/replug it into your computer.
After each answer from the device there is a 250ms delay inside the communication.


## TODO
Optimize the size of the project so that we can add HMAC-SHA256 key derivation for HD Wallets. Another possible way is to find another USB SD-Card uC board with a bigger flash.
Gather randomness from an Arduino UNO to prove the concept on that platform, this could increase the number of community members.
Generate BTC and ETH addresses from the xpub and make some transactions -> publish the xpub inside this repo so that everyone can try to regenerate the corresponding xpriv (like a bug bounty ;-) ).
User friendly GUI for the communication with the module.

## BadUSB!!
ATTENTION! If you receive such a module DON'T plug it into you Computer because you don't know what program is already loaded inside it. You have to flash the module with an ISP or an ArduinoUNO (Arduino ISP example) the first time. Otherwise the module could act as a virtual Keyboard and harm your privacy or data.

![BadUSB Module Top](images/BadUSB-Module-Top.png)
![BadUSB Module Bottom](images/BadUSB-Module-Bottom.png)
