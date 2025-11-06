#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// MAKE WORD -- This section is anything relating to the creation of a word (2
// bytes) from byte pairs

// will make a 16 bit word
uint16_t combineBytesToWord(uint8_t highByte, uint8_t lowByte) {
    return (highByte << 8) | lowByte;
}

// BREAK WORD -- Breaking the 2 byte word into a pair of bytes

void splitWordToBytes(uint8_t *highByte, uint8_t *lowByte, uint16_t word) {
    *highByte = (word >> 8);
    *lowByte = word & 0xff;
}

uint8_t getHighByte(uint16_t value) { return (value >> 8) & 0xff; }

uint8_t getLowByte(uint16_t value) { return value & 0xff; }

// returns the byte at a certain index in the memory of the state machine
uint8_t readByte(State *state, uint16_t index) {
    if (index > MAX_MEMORY_SIZE) {
        fprintf(stderr, "Memory read out of bounds: 0x%04X\n", index);
        exit(EXIT_FAILURE);
    }
    return state->memory[index];
}

uint8_t readByteAtSP(State *state) { return readByte(state, state->sp); }

// loading memory
void loadMemory(State *state, uint8_t *memory) {
    memcpy(state->memory, memory, MEMORY_SIZE);
}

// inserts byte into a certain index in the memory array
void writeByte(State *state, uint16_t index, uint8_t value) {
    if (index > MAX_MEMORY_SIZE) {
        fprintf(stderr, "Memory write out of bounds: 0x%04X\n", index);
        exit(EXIT_FAILURE);
    }
    state->memory[index] = value;
}

// inserts byte into the stack pointer
void writeByteAtSP(State *state, uint8_t value) {
    writeByte(state, state->sp, value);
}

uint8_t nextByte(State *state) { return readByte(state, state->pc++); }

uint16_t nextWord(State *state) {
    uint8_t lowByte = nextByte(state);
    uint8_t highByte = nextByte(state);

    return combineBytesToWord(highByte, lowByte);
}

// getters and setters for register pairs

// get value of the address pointed to by a register pair
uint8_t readMemoryAtRegPair(State *state, uint8_t highByte, uint8_t lowByte) {
    return readByte(state, combineBytesToWord(highByte, lowByte));
}

uint8_t readMemoryAtHL(State *state) {
    return readMemoryAtRegPair(state, state->h, state->l);
}

// breaks the 16 bit value in half and assigns each half to the register pair
// respectfully
void writeRegPairFromWord(State *state, uint8_t *highByte, uint8_t *lowByte,
                          uint16_t value) {
    *highByte = (value >> 8) &
                0xff; // the 0xff is redundant, but keeping it for clarity
    *lowByte = value & 0xff;
}

void writeDirectFromWord(State *state, uint16_t *index, uint16_t value) {
    *index = value;
}

// set a value to the address pointed to by a register pair
void writeMemoryAtRegPair(State *state, uint8_t highByte, uint8_t lowByte,
                          uint8_t value) {
    uint16_t index = combineBytesToWord(highByte, lowByte);
    writeByte(state, index, value);
}

void writeMemoryAtHL(State *state, uint8_t value) {
    writeMemoryAtRegPair(state, state->h, state->l, value);
}

// adds values in two registers together and returns the 32 bit value
uint32_t addToRegPair(State *state, uint8_t *highByte, uint8_t *lowByte,
                      uint16_t value) {
    uint32_t result = combineBytesToWord(*highByte, *lowByte) + value;
    writeRegPairFromWord(state, highByte, lowByte, result & 0xffff);
    return result;
}

void stackArithmetic(State *state, uint16_t incrementValue) {
    state->sp += incrementValue;
    // Apparently the hardware just wraps addresses, so we don't need the whole
    // wrap stuff
    /*
    if (state -> sp > STACK_TOP) {
        printf("Stack overflow error: %d", state -> sp);
        exit(EXIT_FAILURE);
    }
    else if (state -> sp < STACK_BOTTOM) {
        printf("Stack underflow error: %d", state -> sp);
        exit(EXIT_FAILURE);
    }
    */
}

// takes stack pointer and stores them into a register pair
void popIntoRegPair(State *state, uint8_t *highByte, uint8_t *lowByte) {
    *lowByte = readByteAtSP(state);
    stackArithmetic(state, 1);
    *highByte = readByteAtSP(state);
    stackArithmetic(state, 1);
}

// takes stack pointer and stores them into a register pair
uint8_t pop(State *state, uint16_t *value) {
    uint8_t low = readByteAtSP(state);
    uint8_t high = readByte(state, state->sp + 1);
    *value = combineBytesToWord(high, low);
    stackArithmetic(state, 2);
    return 0;
}

// pushes register pair onto the stack
void pushIntoRegPair(State *state, uint8_t *highByte, uint8_t *lowByte) {
    stackArithmetic(state, -2);
    writeByte(state, state->sp + 1, *highByte);
    writeByteAtSP(state, *lowByte);
}

void push(State *state, uint16_t value) {
    stackArithmetic(state, -2);
    writeByte(state, state->sp + 1, getHighByte(value));
    writeByteAtSP(state, getLowByte(value));
}