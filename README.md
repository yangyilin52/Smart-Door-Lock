# Smart Door Lock
A smart door lock based on STM32F103.
<br>
## Overview
<br>
This is a prototype of a smart door lock based on STM32 microcontroller, which provides multiple access options including fingerprint verification, RFID Card access and PIN code access. It can also detect the status of the door lock to lock down automatically right after closing the door or to give an alarm when the user forgets to close the door, improving the performance in intelligence and automation. The friendly user interface and personalized settings options like permissions management and preference settings enhance the security of the door lock and bring more convenience to user’s life.
<br>
## Hardware Design
The smart door lock consists of two main parts, the electronic control unit and the execution unit. The electronic control unit includes a STM32F103 minimal system with a W25Q128 SPI flash onboard, an AS608 optical fingerprint sensor, a RC522 RFID reader, a 0.96-inch OLED screen, a DC-DC power supply module, a 3x4 matrix keyboard, a buzzer module, a LED module and an ESP8266 Wi-Fi module for IoT extension. The execution unit includes an electronic lock, a relay, a magnetometer and a Neodymium magnet. The outlook of the smart door lock is shown below.
<br>
![a](https://github.com/zhangjingye03/BLE_WeRun_Faker/raw/master/circuit.png)
