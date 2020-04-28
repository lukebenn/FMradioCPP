# FMradioCPP
FM Radio Project for LPC802

## Software Requirments
 - MCUXpresso 11.1.0+ 
 - LPC802 SDK
 

## Hardware Requirments
 - LPC802 (or any LPC80x with minor modification)
 - I<sup>2</sup>C 20*4 Charater LCD
 - I<sup>2</sup>C RDA5807M FM Tuner Chip
 - 4 Buttons, 1 Roatary Encoder
 
## Features
 - Manual Tune
 - Auto Scan
 - Signal Strength Display
 - Stereo/Mono Display
 - 4 Programmable Presets
 - Digital Volume Control (0 to 15)
 - Dyanmic I<sup>2</sup>C Bus, Allows for total isloation between multiple I<sup>2</sup>C devices by changing pins assosiated with I<sup>2</sup>C dynamically.
 
## Installation

 1. Import ZIP of this repo into MCUXpresso
 2. Wire up circuit acording to pinouts described in FMradioCPP.cpp (Buttons are Active High)
 3. Build and Run
