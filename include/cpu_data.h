#ifndef CPU_DATA_H
#define CPU_DATA_H

#include "emulator.h"
#include <stdint.h>

/*
 * CPU Data and Register Operations
 *
 * This module contains functions that manipulate register pairs,
 * perform increment/decrement operations, load/store register pairs,
 * and handle direct register-to-register transfers (MOV).
 *
 * These are low-level instruction implementations used by the CPU core.
 */

// Increment / Decrement (Register and Register-Pair)
void inxRegPair(State *state, uint8_t *highByte, uint8_t *lowByte);
void inx(State *state, uint16_t *value);
void inr(State *state, uint8_t *value);
void dcxRegPair(State *state, uint8_t *highByte, uint8_t *lowByte);
void dcx(State *state, uint16_t *value);
void dcr(State *state, uint8_t *value);

// Double Add (DAD)
void dad(State *state, uint16_t value);
void dadRegPair(State *state, uint8_t *highByte, uint8_t *lowByte);

// Load Immediate (LXI)
void lxiRegPair(State *state, uint8_t *highByte, uint8_t *lowByte,
                uint16_t value);
void lxi(State *state, uint16_t *index, uint16_t value);

// Load HL Direct (LHLD)
void lhld(State *state, uint16_t address);

// Register-to-Register Move (MOV)
void mov(State *state, uint8_t *dest, uint8_t src);

#endif // CPU_DATA_H
