#ifndef CPU_ARITHMETIC_H
#define CPU_ARITHMETIC_H

#include "emulator.h"
#include <stdint.h>

void add(State *state, uint8_t value);
void adc(State *state, uint8_t value);
void sub(State *state, uint8_t value);
void sbb(State *state, uint8_t value);
void cmp(State *state, uint8_t value);
void ana(State *state, uint8_t value);
void ora(State *state, uint8_t value);
void xra(State *state, uint8_t value);

#endif
