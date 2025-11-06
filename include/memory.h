#ifndef MEMORY_H
#define MEMORY_H

#include "emulator.h"
#include <stdint.h>

/*
 * Memory and Stack Utilities
 *
 * This module provides low-level operations for:
 *  - Reading/writing bytes and words from emulator memory
 *  - Manipulating register pairs and word values
 *  - Managing stack operations (push, pop, stack pointer arithmetic)
 *
 * These functions abstract memory access for CPU instructions.
 */

// ===== WORD OPERATIONS =====

// Combines two bytes into a 16-bit word (high << 8 | low)
uint16_t combineBytesToWord(uint8_t highByte, uint8_t lowByte);

// Splits a 16-bit word into high and low bytes
void splitWordToBytes(uint8_t *highByte, uint8_t *lowByte, uint16_t word);

// Extracts the upper byte from a 16-bit value
uint8_t getHighByte(uint16_t value);

// Extracts the lower byte from a 16-bit value
uint8_t getLowByte(uint16_t value);

// ===== MEMORY ACCESS =====

// Reads a single byte from memory
uint8_t readByte(State *state, uint16_t index);

// Reads a byte from the address pointed to by the stack pointer
uint8_t readByteAtSP(State *state);

// Writes a byte to memory
void writeByte(State *state, uint16_t index, uint8_t value);

// Writes a byte at the address of the stack pointer
void writeByteAtSP(State *state, uint8_t value);

// Copies a full block of memory into the emulator’s memory
void loadMemory(State *state, uint8_t *memory);

// Reads the next byte from memory and increments the program counter
uint8_t nextByte(State *state);

// Reads the next two bytes as a 16-bit word and increments the program counter
uint16_t nextWord(State *state);

// ===== REGISTER PAIR & ADDRESSING =====

// Reads the value at the address formed by a register pair
uint8_t readMemoryAtRegPair(State *state, uint8_t highByte, uint8_t lowByte);

// Reads the value at the address pointed to by H and L
uint8_t readMemoryAtHL(State *state);

// Writes a 16-bit value into a register pair
void writeRegPairFromWord(State *state, uint8_t *highByte, uint8_t *lowByte,
                          uint16_t value);

// Writes a 16-bit value into a direct 16-bit variable (like SP or PC)
void writeDirectFromWord(State *state, uint16_t *index, uint16_t value);

// Writes a value to the memory address formed by a register pair
void writeMemoryAtRegPair(State *state, uint8_t highByte, uint8_t lowByte,
                          uint8_t value);

// Writes a value to the memory address pointed to by H and L
void writeMemoryAtHL(State *state, uint8_t value);

// Adds a 16-bit value to a register pair and returns a 32-bit result
uint32_t addToRegPair(State *state, uint8_t *highByte, uint8_t *lowByte,
                      uint16_t value);

// ===== STACK OPERATIONS =====

// Adjusts the stack pointer by the given signed increment
void stackArithmetic(State *state, uint16_t incrementValue);

// Pops two bytes from the stack into a register pair
void popIntoRegPair(State *state, uint8_t *highByte, uint8_t *lowByte);

// Pops two bytes from the stack into a 16-bit variable
uint8_t pop(State *state, uint16_t *value);

// Pushes two bytes from a register pair onto the stack
void pushIntoRegPair(State *state, uint8_t *highByte, uint8_t *lowByte);

// Pushes a 16-bit value onto the stack
void push(State *state, uint16_t value);

#endif // MEMORY_H
