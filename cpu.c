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
    // the pc will increment by 1, so we will decrement now to compensate for that
    printf("Error: Unimplemented instruction");
    state -> pc -=1;
    exit(1);
}

// FLAGS -- Constant flags made from bit shifts to manipulate different flags

// bit shifts each of the flags so that it can be logically OR'd to make it 1
#define Z_FLAG (1<<7)
#define S_FLAG (1<<6)
#define P_FLAG (1<<5)
#define CY_FLAG (1<<4)
#define AC_FLAG (1<<3)
#define INCREMENT_FLAGS (Z_FLAG | S_FLAG | P_FLAG | AC_FLAG) // used for only the increment and decrement functions
#define ALL_FLAGS (Z_FLAG | S_FLAG | P_FLAG | CY_FLAG | AC_FLAG) // used to set all the flags as true (for the arithmetic and logic instructions)


// CHECK FLAGS -- Checks certain results for the flags

// returns 1 if the result is equal to 0 and 0 if it is 1
uint8_t checkZero(uint16_t result) {
    return ((result & 0xff) == 0);
}

// returns 1 if it is positive and 0 if negative
uint8_t checkSign(uint16_t result) {
    return (result & 0xff) >> 7;
}

// returns 1 if there is even parity and 0 if there is odd parity.
uint8_t checkParity(uint16_t result) {

    uint8_t parity = 0;

    // loops through the only 8 bits
    for (int i = 0; i < 8; i++) {
        parity ^= (result & 1);  // XOR the least significant bit with the parity
        result >>= 1;            // Right shift to check the next bit
    }

    return !parity;
}

// TODO - Add borrow checker for carry functions
// returns 1 if there is a carry and returns 0 if there isn't
uint8_t checkCarry(uint16_t result) {
    return (result > 0xff); 
}

// returns if there has been a carry from bit 4 into bit 5
// also known as half carry
uint8_t checkAuxillaryCarry(uint16_t result){

    uint8_t finalByte = result & 0xff; // gets only the last 8 bits
    uint8_t cleaned = finalByte & 0b00011111; // makes the first 3 bits 0's
    return cleaned > 0xf;
}


// SET FLAGS -- function to set the flags for different groups of opcodes

// sets specific flags depending on the binary value given by flagMask
void setFlags(State8080* state, uint16_t result, uint8_t flagMask) {

    if (flagMask & Z_FLAG) {
        state->cc.z = checkZero(result);
    }
    if (flagMask & S_FLAG) {
        state->cc.s = checkSign(result);
    }
    if (flagMask & P_FLAG) {
        state->cc.p = checkParity(result);
    }
    if (flagMask & AC_FLAG) {
        state->cc.ac= checkAuxillaryCarry(result);
    }
    if (flagMask & CY_FLAG) {
        state->cc.cy = checkCarry(result);
    }
}

// MAKE WORD -- This section is anything relating to the creation of a word (2 bytes) from byte pairs

// will make a word
uint16_t make2ByteWord(uint16_t a, uint16_t b) {
    return (a << 8) | b;
}

// makes the values in register h and l into a word
uint16_t make2ByteWordHL(State8080 *state) {
    return make2ByteWord(state->h, state->l);
}

// makes the values in register b and c into a word
uint16_t make2ByteWordBC(State8080 *state) {
    return make2ByteWord(state->b, state->c);
}

// makes the values in register d and e into a word
uint16_t make2ByteWordDE(State8080 *state) {
    return make2ByteWord(state->d, state->e);
}



// BREAK WORD -- Breaking the 2 byte word back into a pair of bytes

void break2ByteWord(uint8_t *a, uint8_t *b, uint16_t word) {
    *a = (word >> 8);
    *b = word & 0xff;
}

// breaks the word into two bytes stored in h and l respectively
void break2ByteWordHL(State8080 *state, uint16_t word) {
    break2ByteWord(&state->h, &state->l,  word);
}

// breaks the word into two bytes stored in b and c respectively
void break2ByteWordBC(State8080 *state, uint16_t word) {
    break2ByteWord(&state->b, &state->c,  word);
}

// breaks the word into two bytes stored in d and e respectively
void break2ByteWordDE(State8080 *state, uint16_t word) {
    break2ByteWord(&state->d, &state->e,  word);
}

// returns the byte at a certain index in the memory of the state machine
uint8_t readByte(State8080* state, uint16_t index) {
    return state -> memory[index];
}

// inserts byte into a certain index in the memory array
void writeByte(State8080* state, uint16_t index, uint8_t value) {
    state->memory[index] = value;
}

// getters and setters for the registers H and L
uint16_t getHL(State8080* state) {
    return readByte(state, make2ByteWordHL(state));
}

void setHL(State8080* state, uint8_t value) {
    uint16_t index = make2ByteWordHL(state);
    writeByte(state, index, value);
}


// ARITHMETIC GROUP -- instructions for the arithmetic values in the isa

// Joins to 8 bit words, increments it and then splits it up again
// it is fine if the value overflows, this is expected behaviour.
void inx(uint8_t *a, uint8_t *b) {
    uint16_t word = make2ByteWord(*a, *b);
    word++;
    break2ByteWord(a, b, word);
}

void inr(State8080* state, uint8_t *a) {
    uint16_t answer = *a; // casts to 16 bit number
    answer++;
    setFlags(state, *a, INCREMENT_FLAGS);
    *a = answer & 0xff; // discards the first 8 bits
}

void dnr(State8080* state, uint8_t *a) {
    uint16_t answer = *a; // casts to 16 bit number
    answer--;
    setFlags(state, *a, INCREMENT_FLAGS);
    *a = answer & 0xff; // discards the first 8 bits
}



// setFlags(state, result, ALL_FLAGS); for all the flags
// setFlags(state, result, INCREMENT_FLAGS); for all the flags except cy

void Emulate8080p(State8080* state) {
    unsigned char* opcode = &(state -> memory[state->pc++]); // the opcode is indicated by the program counter's index in memory
                                                             // the program counter is also incremented because every instruction is one byte

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
            inx(&state -> a, &state -> b);
        case 0x04:
            inr(state, &state -> b);
        case 0x05:
            dnr(state, &state -> b);


        // mov opcodes
        case 0x40:
            state->b = state->b;
            break;
        case 0x41:
            state->b = state->c;
            break;
        case 0x42:
            state->b = state->d;
            break;
        case 0x43:
            state->b = state->e;
            break;
        case 0x44:
            state->b = state->h;
            break;
        case 0x45:
            state->b = state->l;
            break;
        case 0x46:
            state->b = getHL(state);
            break;
        case 0x47:
            state->b = state->a;
            break;
        case 0x48:
            state->c = state->b;
            break;
        case 0x49:
            state->c = state->c;
            break;
        case 0x4a:
            state->c = state->d;
            break;
        case 0x4b:
            state->c = state->e;
            break;
        case 0x4c:
            state->c = state->h;
            break;
        case 0x4d:
            state->c = state->l;
            break;
        case 0x4e:
            state->c = getHL(state);
            break;
        case 0x4f:
            state->c = state->a;
            break;
        case 0x50:
            state->d = state->b;
            break;
        case 0x51:
            state->d = state->c;
            break;
        case 0x52:
            state->d = state->d;
            break;
        case 0x53:
            state->d = state->e;
            break;
        case 0x54:
            state->d = state->h;
            break;
        case 0x55:
            state->d = state->l;
            break;
        case 0x56:
            state->d = getHL(state);
            break;
        case 0x57:
            state->d = state->a;
            break;
        case 0x58:
            state->e = state->b;
            break;
        case 0x59:
            state->e = state->c;
            break;
        case 0x5a:
            state->e = state->d;
            break;
        case 0x5b:
            state->e = state->e;
            break;
        case 0x5c:
            state->e = state->h;
            break;
        case 0x5d:
            state->e = state->l;
            break;
        case 0x5e:
            state->e = getHL(state);
            break;
        case 0x5f:
            state->e = state->a;
            break;
        case 0x60:
            state->h = state->b;
            break;
        case 0x61:
            state->h = state->c;
            break;
        case 0x62:
            state->h = state->d;
            break;
        case 0x63:
            state->h = state->e;
            break;
        case 0x64:
            state->h = state->h;
            break;
        case 0x65:
            state->h = state->l;
            break;
        case 0x66:
            state->h = getHL(state);
            break;
        case 0x67:
            state->h = state->a;
            break;
        case 0x68:
            state->l = state->b;
            break;
        case 0x69:
            state->l = state->c;
            break;
        case 0x6a:
            state->l = state->d;
            break;
        case 0x6b:
            state->l = state->e;
            break;
        case 0x6c:
            state->l = state->h;
            break;
        case 0x6d:
            state->l = state->l;
            break;
        case 0x6e:
            state->l = getHL(state);
            break;
        case 0x6f:
            state->l = state->a;
            break;
        case 0x70:
            setHL(state, state->b);
            break;
        case 0x71:
            setHL(state, state->c);
            break;
        case 0x72:
            setHL(state, state->d);
            break;
        case 0x73:
            setHL(state, state->e);
            break;
        case 0x74:
            setHL(state, state->h);
            break;
        case 0x75:
            setHL(state, state->l);
            break;
        case 0x76:
            printf("Halting emulation");
            exit(0);
            break;
        case 0x77:
            setHL(state, state->a);
            break;
        case 0x78:
            state->a = state->b;
            break;
        case 0x79:
            state->a = state->c;
            break;
        case 0x7a:
            state->a = state->d;
            break;
        case 0x7b:
            state->a = state->e;
            break;
        case 0x7c:
            state->a = state->h;
            break;
        case 0x7d:
            state->a = state->l;
            break;
        case 0x7e:
            state->a = getHL(state);
            break;
        case 0x7f:
            state->a = state->a;
            break;
    }
}

