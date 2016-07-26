Dimmer Library for Arduino (V0)
==========================


### NOTE: This is an early version of the library to work with non updated older projects.
#### Please look up for the new and improved version on Github.


This is the Arduino software library to control AC loads using triacs and a zero cross detector circuit. The library functions use only one Object structure to control the AC load power of each Triac independently, using a single zero cross circuit.

The reference circuit is based on the Triac and Zero Cross Nanoshields from Circuitar, and can be found at:

- Triac: https://www.circuitar.com.br/en/nanoshields/modules/triac/ 
- Zero Cross: https://www.circuitar.com.br/en/nanoshields/modules/zero-cross/

There are different ways to implement zero cross detector circuits. This library is based on the implementation above, but it can be easily adapted to use any type of zero cross detector circuit.

To install, copy the library under <arduinoSketchFolder>/libraries/

This library needs TimerOne library which can be found here: https://github.com/PaulStoffregen/TimerOne

---

Usage:

Use the library functions `attachZeroCross()` and `attachTriac()` to assign each triac and the zero cross shield to the program.

To start controlling the lamps, initialize with `init()` or `initRamp()` to make smooth transitions between power levels. In both cases the lamps will start with 50% of the AC power.

The triacs can be controlled using the functions: `set(lamp, power)`, `on()`, `on(lamp)`, `off()`, `off(lamp)` and `toogle(lamp)`.

---

This software is released under an MIT license. See the attached LICENSE file for details.

