# bb (aka beer brewer)
Cheap beer brewing electronics.
The goal is make and program a very cheap electronic based on arduino, that will automate home beer brew. Every step of brewing can be easily defined by changing the CSV file, that is store on SD card and possibility to watch and interfernce to the program with web viewer.

# Hardware parts for electronics
* 1x Arduino Nano
* 1x I2C LCD 2004
* 1x 4 Channel Relay Board Module
* 1x Waterproof PT100 Platinum Resister Temperature Sensor
* 1x Micro SD Card Reader Module SPI
* 1x Passive Buzzer
* 1x 1x4 Membrane Switch Keypad
* 1x Nano I / O Expansion sensor Shield Module
* 1x Power Supply Module 3.3V/5V For Arduino
* 1x 40pcs Jumper Wire Cable Female-Female 2.54mm 10cm
* 1x ESP8266 Remote WiFI Wireless Transceiver Module (optional)
Total price on ebay about 20 USD

# Software code
Arduino IDE 1.6.0 or newer. NOTE: Some version has a bugs (like 1.8.0 with SD card module). The code was tested on 1.8.1

# 2do
* add web server
* add go to next, previous step
* add cancel and pause
* repair some bugs
* memory improvements
