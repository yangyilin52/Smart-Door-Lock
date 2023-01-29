# Smart Door Lock
A smart door lock based on STM32F103.
## Overview
This is a prototype of a smart door lock based on STM32 microcontroller, which provides multiple access options including fingerprint verification, RFID Card access and PIN code access. It can also detect the status of the door lock to lock down automatically right after closing the door or to give an alarm when the user forgets to close the door, improving the performance in intelligence and automation. The friendly user interface and personalized settings options like permissions management and preference settings enhance the security of the door lock and bring more convenience to user’s life.
## Hardware Design
The smart door lock consists of two main parts, the electronic control unit and the execution unit. The electronic control unit includes a STM32F103 minimal system with a W25Q128 SPI flash onboard, an AS608 optical fingerprint sensor, a RC522 RFID reader, a 0.96-inch OLED screen, a DC-DC power supply module, a 3x4 matrix keyboard, a buzzer module, a LED module and an ESP8266 Wi-Fi module for IoT extension. The execution unit includes an electronic lock, a relay, a magnetometer and a Neodymium magnet. The outlook of the smart door lock is shown below.<br>

Here is the schematic circuit diagram. The AS608 fingerprint sensor interface with the MCU using USART3 while the ESP8266 module using USART2. The OLED screen and the RC522 RFID module are both connected to SPI1 and W25Q128 uses SPI2 alone. Detailed information about the hardware design can be found here.<br>

## Software Design
I use the SPL (Standard Peripherals Library) to write the application for the STM32 microcontroller. Written in C, SPL is a set of libraries created by ST company, providing a set of functions in which relative registers are operated and necessary calculations are executed to rid the developers of complicated operation of the registers and increase development efficiency.<br>

To make those modules work properly, I wrote drivers for each of them. Considering of the software portability, codes related with hardware operation and SPL library calls are all packed in specific low-level functions to provide APIs to high-level functions to control those modules, which is similar to BSP (Board Support Package). So that the hardware can be easily migrated to other MCU platforms like TI MSP430 and NXP KL4x through modifying only a few hardware-related codes.<br>

## Demonstration
Three frequently used functions are demonstrated here.
