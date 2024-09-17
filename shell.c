#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct ConditionCodes {
    uint8_t z:1;
    uint8_t s:1;
    uint8_t p:1;
    uint8_t cy:1;
    uint8_t ac:1; // space invaders doesn't use this flag
    uint8_t pad:3;
} ConditionCodes;

typedef struct State8080 {
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t h;
    uint8_t l;
    uint16_t sp;
    uint16_t pc;
    uint8_t *memory; // this is an array that stores integers.
    struct ConditionCodes cc;
    uint8_t int_enable;
} State8080;

// this is for any instruction that we have not yet implemented, so that the program does not crash
void UnimplementedInstruction(State8080* state) {
    // the pc will increment by 1, so we will decrement now to compenstate for that
    printf("Error: Unimplemented instruction");
    state -> pc -=1;
    exit(1);
}

// returns 1 if there is a carry and returns 0 if there isn't
uint8_t carry(uint8_t answer) {
    return (answer > 0xff);
}

// will make a word (16 bytes) from two 8 byte pairs
uint16_t makeWord(uint8_t a, uint8_t b) {
    uint16_t result = (a << 7) | b;
    return result;
}



void Emulate8080p(State8080* state) {
    unsigned char* opcode = &(state -> memory[state->pc]); // the opcode is indicated by the program counter's index in memory.

    switch(*opcode) {
        case 0x00: break;
        case 0x01:
                   state -> b = opcode[1];
                   state -> c = opcode[2];

                   state -> pc += 2;
                   break;
        case 0x02:
                   state -> b = state -> a;
                   state -> c = state -> a;

                   break;
        case 0x03:
                   UnimplementedInstruction(state);
                   break;

    }

    // the opcode counts as one byte, so we always increment the program counter atleast by one
    state -> pc += 1;
}


