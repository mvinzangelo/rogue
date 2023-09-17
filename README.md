# Embedded Rogue

A port of the classic videogame "Rogue" on the Arduino.

## Description
Rogue is a top-down dungeon crawler where the player traverses a maze-like castle to slay goblins, skeletons, and dragons on their way to defeat the king and save the kingdom from tyranny. Although the enemies may be tough at first, fall in battle and evolve from your previous attempt, saving your progress on each run with the hopes of growing strong enough to defeat the powerful king. Level up and increase your HP and STR, allowing you to take and deal more damage to any monster encountered.

## Requirements
* ArduinoIDE

## Hardware Components
* Arduino UNO
* 16x2 LCD
* Nokia 5110 LCD
* 2x Potentiometer 10k
* Joystick Module
* 4x 10k Resistor
* 2x 1k Resistor

## Wiring Diagram
![image](https://github.com/mvinzangelo/rogue/assets/74069694/eebdb0e3-f973-4f95-8065-07056b55ef52)

LCD Pins:
* RS - 8
* EN - 7
* D4 - 6
* D5 - 5
* D6 - 4
* D7 - 3 (1k resistor)

Nokia 5110 Pins:
* CLK - 9 (10k resistor)
* DIN - 13 (10k resistor)
* D/C - 12 (10k resistor)
* CE - 10 (1k resistor)
* RST - 11 (10k resistor)

Joystick Pins:
* SW - 2
* VRx - A0
* VRy - A1


## Installation

```bash
git clone https://github.com/mvinzangelo/rogue.git
```

## Executing
* Open the project in the ArduinoIDE
* Flash the program onto the board

## Project Demo
