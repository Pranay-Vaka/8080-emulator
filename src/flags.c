#include "flags.h"

// CHECK FLAGS -- Checks certain values for the flags
// this will get rid of the first 8 bits since they could affect the value

// returns 1 if the value is equal to 0 and 0 if it is 1
uint8_t checkZero(uint8_t value) { return ((value & 0xff) == 0); }

// returns 1 if it is positive and 0 if negative
uint8_t checkSign(uint8_t value) { return value >> 7; }

// returns 1 if there is even parity and 0 if there is odd parity.
uint8_t checkParity(uint8_t value) {

    uint8_t parity = 0;

    // loops through the only 8 bits
    for (int i = 0; i < 8; i++) {
        parity ^= (value & 1); // XOR the least significant bit with the parity
        value >>= 1;           // Right shift to check the next bit
    }

    return !parity;
}

// returns 1 if there is a carry and returns 0 if there isn't
/*
uint8_t checkCarry(uint16_t value) {
    return (value > 0xff);
}
*/

uint8_t checkCarry(uint16_t result, uint8_t isSubtraction) {
    if (isSubtraction) { // does a check if there is a borrow as the value will
                         // overflow
        return (result > 0xff);
    } else {
        return (result & 0x100) != 0;
    }
}

// CHECK FLAGS -- function to set the flags for different groups of opcodes

// checks and then sets specific flags depending on the binary value given by
// flagMask
void checkFlags(State *state, uint16_t value, uint8_t flagMask,
                uint8_t isSubtraction) {

    // Empties the top 8 bits
    value &= 0xff;

    if (flagMask & Z_FLAG) {
        state->cc.z = checkZero(value);
    }
    if (flagMask & S_FLAG) {
        state->cc.s = checkSign(value);
    }
    if (flagMask & P_FLAG) {
        state->cc.p = checkParity(value);
    }
    if (flagMask & CY_FLAG) {
        state->cc.cy = checkCarry(value, isSubtraction);
    }
}

// SET AND GET FLAGS

uint8_t getFlags(State *state) {
    uint8_t flags = 0;

    flags |= (state->cc.s << 7);
    flags |= (state->cc.z << 6);
    flags |= (state->cc.ac << 4);
    flags |= (state->cc.p << 2);
    flags |= (state->cc.cy << 0);

    return flags;
}

void setFlags(State *state, uint8_t flags) {

    state->cc.s = (flags >> 7) & 0x1;
    state->cc.z = (flags >> 6) & 0x1;
    state->cc.ac = (flags >> 4) & 0x1;
    state->cc.p = (flags >> 2) & 0x1;
    state->cc.cy = (flags >> 0) & 0x1;
}