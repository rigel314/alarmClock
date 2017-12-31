alarmClock
========
# About
My fiancé wanted an alarm clock that simulated a sunrise before ringing.  Since all the ones you can buy are expensive, dumb, or both, I decided to make one.

# Dependencies
 * [My fork of firelizzard's Nokia LCD library](https://github.com/rigel314/Arduino-LCD-Screen), which is a submodule
 * [TimerOne](https://github.com/PaulStoffregen/TimerOne) as installed by the teensy installer
 * [TimeLib](https://github.com/PaulStoffregen/Time) as installed by the teensy installer
 * [DS1307RTC](https://github.com/PaulStoffregen/DS1307RTC) as installed through the Arduino IDE "Manage Libraries" interface

# Hardware
 * 3-color LED strip
 * Resistors and transistors
 * DS1307 Real Time Clock
 * MCP4725 I2C DAC for audio
 * Speaker
 * Nokia 5110 84x48 pixel LCD screen
 * Arduino Duemilanove/Uno with atmega328P
 * 12V power supply

See the [/eagle](https://github.com/rigel314/alarmClock/tree/master/eagle) folder in this repo for the board layout.
<a href="https://raw.githubusercontent.com/rigel314/alarmClock/master/images/schematic.png">![Schematic](https://raw.githubusercontent.com/rigel314/alarmClock/master/images/schematic.png)</a>
