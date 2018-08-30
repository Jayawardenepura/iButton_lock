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

**(0)(crc)(family code)(serial number)**

USART Settings: 19200,even,2

The keys q,w,e,r,t,y are used to select one of 6 cells as current for further operations (replace, remove),which also could be imitated with the COM-terminal:
To write a new key to the EEPROM, use:

- q,w,e,r,t,y,u,i,o,p
- 0,1,2,3,4,5,6,7,8,9

## For deletion of a key from the EEPROM index, use:

- -q,-w,-e-,-r,-t,-y,-u,-i,-o,-p
- -0,-1,-2,-3,-4,-5,-6,-7,-8,-9

## To delete all the keys from EEPROM use:

- a

## To read all keys stored in EEPROM, use:

- k

It is possible to store up to 1024 8-byte keys in EEPROM (specified via define constant)

## Pinout

- PB0 - the 1-Wire bus contact used to communicate with iButton device
- GNG - the second (ground) contact of the 1-Wire bus
- PB2 - The LED indicating ACCESS DENIAL (i.e. red)
- PB3 - The LED indicating ACCESS GRANTED (i.e. green)

**My recommendation is to use an RGB LED!**

## Flash

In case of using MCU other than AtMega328p, change the MCU constant in project Makefile). Build steps:

- make clean hex
- make flash

## 1-Wire library has been developed in C/Embedded BaseCamp Course
