#include "cpu_data.h"
#include "flags.h"
#include "memory.h"

void inxRegPair(State *state, uint8_t *highByte, uint8_t *lowByte) {
    uint16_t word = combineBytesToWord(*highByte, *lowByte);
    word++;
    writeRegPairFromWord(state, highByte, lowByte, word);
}

// increments the 16 bit word
void inx(State *state, uint16_t *value) { (*value)++; }

void inr(State *state, uint8_t *value) {
    uint8_t result = *value + 1;
    checkFlags(state, result, INCREMENT_FLAGS, 0);
    *value = result; // discards the first 8 bits
}

// Joins to 8 bit words, decrements it and then splits it up again
// it is fine if the value overflows, this is expected behaviour.
void dcxRegPair(State *state, uint8_t *highByte, uint8_t *lowByte) {
    uint16_t word = combineBytesToWord(*highByte, *lowByte);
    word--;
    writeRegPairFromWord(state, highByte, lowByte, word);
}

// decrements the 16 bit word
void dcx(State *state, uint16_t *value) { (*value)--; }

void dcr(State *state, uint8_t *value) {
    uint8_t result = *value - 1;
    checkFlags(state, result, INCREMENT_FLAGS, 0);
    *value = result; // discards the first 8 bits
};

// dad opcode takes word and then adds them to register h and l
void dad(State *state, uint16_t value) {
    uint32_t result = addToRegPair(state, &state->h, &state->l, value);
    state->cc.cy = (result >> 16) & 1;
}

// dadRegPair opcode that joins two bytes and then adds them to register h and l
void dadRegPair(State *state, const uint8_t *highByte, const uint8_t *lowByte) {
    uint16_t value = combineBytesToWord(*highByte, *lowByte);
    dad(state, value);
}

// loads a 16 bit value into a register pair
void lxiRegPair(State *state, uint8_t *highByte, uint8_t *lowByte,
                uint16_t value) {
    writeRegPairFromWord(state, highByte, lowByte, value);
}

// loads a 16 bit value into a register pair
void lxi(State *state, uint16_t *index, uint16_t value) {
    writeDirectFromWord(state, index, value);
}

void lhld(State *state, uint16_t address) {
    // stores the data in the address to register l
    state->l = readByte(state, address);
    // stores the data in the address + 1 to register h
    state->h = readByte(state, address + 1);
}

void mov(State *state, uint8_t *dest, uint8_t src) { *dest = src; }