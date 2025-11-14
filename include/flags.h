#ifndef FLAGS_H
#define FLAGS_H

#include "emulator.h" // for State and flag masks
#include <stdint.h>

uint8_t checkZero(uint8_t value);
uint8_t checkSign(uint8_t value);
uint8_t checkParity(uint8_t value);
uint8_t checkCarry(uint16_t result, uint8_t isSubtraction);
void checkFlags(State *state, uint16_t value, uint8_t flagMask,
                uint8_t isSubtraction);
uint8_t getFlags(const State *state);
void setFlags(State *state, uint8_t flags);

#endif
