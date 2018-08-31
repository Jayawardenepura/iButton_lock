iButton lock
---------------------------------------------------------------------------------
The prototype of the electronic lock. Utilizes bare-metal AVR (no Arduino libs)

The core concept behind the lock is iButton key using the 1-Wire protocol:

We process the answer from the Read ROM command and search corresponding keys stored in EEPROM.

EEPROM utilization and organization structure:

- [0] - q
- [1] - w
- .
- .
- [9] - p

```(crc)(family code)(serial number) in HEX```

Pay attention:Are saved only 5 bytes of the serial number!

You can define size of the keys database in the lock.c:
```#define NUMBER_OF_KEYS 36```

```USART Settings: 19200,even,2```

The keys q,w,e,r,t,y.. are used to select one of 6 cells as current for further operations (replace, remove),which also could be imitated with the COM-terminal:
To write a new key to the EEPROM, use:
```
q,w,e,r,t,y,u,i,o,p..
0,1,2,3,4,5,6,7,8,9..
```
## For deletion of a key from the EEPROM index, use:
```
-q,-w,-e-,-r,-t,-y,-u,-i,-o,-p..
-1,-2,-3,-4,-5,-6,-7,-8,-9,-10..
```
## To delete all the keys from EEPROM use:
```
- cla(clean all)
```
## To read all keys stored in EEPROM, use:
```
- la(list all)
```
It is possible to store up to 1024 8-byte keys in EEPROM (specified via define constant)

Has been envisaged the protection from needless memory recleanings and recreating.

The results of the ACCESS GRANTED and ACCESS DENIAL are accompanied by led blinking.

Moreover ACCESS GRANTED is accompanied by uart output,which shows HMAC-256 from ibutton id and the key below(You can give yours):

```
uint8_t key[] = {
  0x3d, 0xc6, 0xca, 0xa4, 0x82, 0x4a, 0x6d, 0x28,
  0x87, 0x67, 0xb2, 0x33, 0x1e, 0x20, 0xb4, 0x31,
  0x66, 0xcb, 0x85, 0xd9 
};
```
It need to work wish serial applications, for example: https://github.com/Jayawardenepura/iButton-PAM-module

## Pinout

- PB0 - the 1-Wire bus contact used to communicate with iButton device
- GNG - the second (ground) contact of the 1-Wire bus
- PB2 - The LED indicating ACCESS DENIAL (i.e. red)
- PB3 - The LED indicating ACCESS GRANTED (i.e. green)

**My recommendation is to use an RGB LED!**

## Flash

In case of using MCU other than AtMega328p, change the MCU constant in project Makefile). Build steps:
```
$make clean hex
$make flash
```
## 1-Wire library has been developed in C/Embedded BaseCamp Course
