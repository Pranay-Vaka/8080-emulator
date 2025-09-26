# What is this project about?
This is an emulator that simulates the Intel-8080 processor, enabling programs written for the Intel 8080 to run on non-native hardware.
The main goal of this project is to be able to run the original Space Invaders on this emulator.

## Space invaders ROM file
To run this emulator, you need the original Space Invaders ROM 'invaders.rom' for the intel 8080. Due to copyright reasons, this file is
not included in the repo. Place it in the project root before running

## Limitations for now

- The AC (Auxillary Carry) flag has not been implemented. This might cause issues with certain instructions, but is not required for Space Invaders
