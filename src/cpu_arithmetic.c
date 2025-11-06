#include "cpu_arithmetic.h"
#include "flags.h"

void add(State *state, uint8_t value) {
    uint16_t data = (state->a) + value;
    checkFlags(state, data & 0xff, ALL_FLAGS, 0);
    state->a = (uint8_t)data & 0xff;
}

void adc(State *state, uint8_t value) {
    uint16_t data = (state->a) + value + (state->cc.cy);
    checkFlags(state, data & 0xff, ALL_FLAGS, 0);
    state->a = (uint8_t)data & 0xff;
}

void sub(State *state, uint8_t value) {
    uint16_t data = (state->a) - value;
    checkFlags(state, data & 0xff, ALL_FLAGS, 1);
    state->a = (uint8_t)data & 0xff;
}

void sbb(State *state, uint8_t value) {
    uint16_t data = (state->a) - value - (state->cc.cy);
    checkFlags(state, data & 0xff, ALL_FLAGS, 1);
    state->a = (uint8_t)data & 0xff;
}

void cmp(State *state, uint8_t value) {
    uint16_t data = (state->a) - value;
    checkFlags(state, data, ALL_FLAGS, 1);
}

// LOGICAL methods

void ana(State *state, uint8_t value) {
    uint16_t data = (state->a) & value;
    state->cc.cy = 0; // logical methods clear the carry flag
    checkFlags(state, data, NON_CARRY_FLAGS,
               0); // make sure not to alter the carry flag as it is cleared
    state->a = (uint8_t)data;
}

void ora(State *state, uint8_t value) {
    uint16_t data = (state->a) | value;
    state->cc.cy = 0;
    checkFlags(state, data, NON_CARRY_FLAGS, 0);
    state->a = (uint8_t)data;
}

void xra(State *state, uint8_t value) {
    uint16_t data = (state->a) ^ value;
    state->cc.cy = 0;
    checkFlags(state, data, NON_CARRY_FLAGS, 0);
    state->a = (uint8_t)data;
}