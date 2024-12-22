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
    printf("Error: Unimplemented instruction\n");
    printf("Program counter: %x\n", state -> pc);
    state -> pc -=1;
    exit(EXIT_FAILURE);
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
#define NON_CARRY_FLAGS (Z_FLAG | S_FLAG | P_FLAG) // used to set all the flags as true (for the arithmetic and logic instructions)


// CHECK FLAGS -- Checks certain results for the flags
// this will get rid of the first 8 bits since they could affect the result

// returns 1 if the result is equal to 0 and 0 if it is 1
uint8_t checkZero(uint8_t result) {
    return ((result & 0xff) == 0);
}

// returns 1 if it is positive and 0 if negative
uint8_t checkSign(uint8_t result) {
    return (result & 0xff) >> 7;
}

// returns 1 if there is even parity and 0 if there is odd parity.
uint8_t checkParity(uint8_t result) {

    uint8_t parity = 0;

    // loops through the only 8 bits
    for (int i = 0; i < 8; i++) {
        parity ^= (result & 1);  // XOR the least significant bit with the parity
        result >>= 1;            // Right shift to check the next bit
    }

    return !parity;
}

// returns 1 if there is a carry and returns 0 if there isn't
uint8_t checkCarry(uint16_t result) {
    return (result > 0xff);
}

// space invaders does not use this and so is not implemented.
// returns if there has been a carry from bit 4 into bit 5
// also known as half carry
uint8_t checkAuxiliaryCarry(uint16_t result){

    uint8_t finalByte = result & 0xff; // gets only the last 8 bits
    uint8_t cleaned = finalByte & 0b00011111; // makes the first 3 bits 0's
    return cleaned > 0x0f; // checks if the 4th bit is affected
}


// SET FLAGS -- function to set the flags for different groups of opcodes

// sets specific flags depending on the binary value given by flagMask
void setFlags(State8080* state, uint16_t value, uint8_t flagMask) {

    if (flagMask & Z_FLAG) {
        state->cc.z = checkZero(value);
    }
    if (flagMask & S_FLAG) {
        state->cc.s = checkSign(value);
    }
    if (flagMask & P_FLAG) {
        state->cc.p = checkParity(value);
    }
    if (flagMask & AC_FLAG) {
        state->cc.ac= checkAuxiliaryCarry(value);
    }
    if (flagMask & CY_FLAG) {
        state->cc.cy = checkCarry(value);
    }
}

// MAKE WORD -- This section is anything relating to the creation of a word (2 bytes) from byte pairs

// will make a 16 bit word
uint16_t make2ByteWord(uint8_t high, uint8_t low) {
    return (high << 8) | low;
}


// BREAK WORD -- Breaking the 2 byte word into a pair of bytes

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

uint8_t nextByte(State8080* state){
    return readByte(state, state -> pc++);
}

uint16_t nextWord(State8080* state) {
    uint8_t left = nextByte(state);
    uint8_t right = nextByte(state);

    return make2ByteWord(left, right);
}


// getters and setters for register pairs

// get value of the address pointed to by a register pair
uint16_t getMem(State8080* state, uint8_t a, uint8_t b) {
    return readByte(state, make2ByteWord(a, b));
}

// breaks the 16 bit value in half and assigns each half to the register pair respectfully
void setPair(State8080* state, uint8_t* a, uint8_t* b, uint16_t value) {
        *a = (value >> 8) & 0xff; // the 0xff is redundant, but keeping it for clarity
        *b = value & 0xff;
}

// set a value to the address pointed to by a register pair
void setMem(State8080* state, uint8_t a, uint8_t b, uint16_t value) {
    uint16_t index = make2ByteWord(a, b);
    writeByte(state, index, value);
}


// adds values in two registers together and returns the 32 bit result
uint32_t twoRegAddition(uint8_t* a, uint8_t* b, uint16_t value) {
    uint8_t twoByteWord = make2ByteWord(*a, *b);
    uint32_t result = twoByteWord + value;

    *a = (result & 0xff00) >> 8;
    *b = (result & 0xff);

    return result;
}


// ARITHMETIC GROUP -- instructions for the arithmetic values in the isa

// Joins to 8 bit words, increments it and then splits it up again
// it is fine if the value overflows, this is expected behaviour.
void inx(uint8_t *a, uint8_t *b) {
    uint16_t word = make2ByteWord(*a, *b);
    word++;
    break2ByteWord(a, b, word);
}

void inr(State8080* state, uint8_t* a) {
    uint8_t answer = *a + 1; // casts to 16 bit number
    setFlags(state, answer, INCREMENT_FLAGS);
    *a = answer; // discards the first 8 bits
}

// Joins to 8 bit words, decrements it and then splits it up again
// it is fine if the value overflows, this is expected behaviour.
void dcx(uint8_t *a, uint8_t *b) {
    uint16_t word = make2ByteWord(*a, *b);
    word--;
    break2ByteWord(a, b, word);
}

void dcr(State8080* state, uint8_t *a) {
    uint8_t answer = *a - 1; // casts to 16 bit number
    setFlags(state, answer, INCREMENT_FLAGS);
    *a = answer; // discards the first 8 bits
};

// dad opcode that joins two bytes and then adds them to register h and l
void dad(State8080* state, uint8_t *a, uint8_t *b) {
    uint16_t value = make2ByteWord(*a, *b);
    uint32_t answer = twoRegAddition(&state -> h, &state -> l, value);
    state -> cc.cy = (answer >> 16) | 1;
}

void lxi(State8080* state, uint8_t* a, uint8_t* b, uint16_t value) {
    setPair(state, a, b, value);
}

void stax(State8080* state, uint8_t a, uint8_t b, uint16_t value) {
    setMem(state, a, b, value);
}

void lhld(State8080* state, uint16_t address) {
    // stores the data in the address to register l
    state -> l = readByte(state, address);
    // stores the data in the address + 1 to register h
    state -> h = readByte(state, address + 1);
}

// setFlags(state, result, ALL_FLAGS); for all the flags
// setFlags(state, result, INCREMENT_FLAGS); for all the flags except cy

void Emulate8080p(State8080* state) {
    unsigned char* opcode = &(state -> memory[state->pc++]); // the opcode is indicated by the program counter's index in memory

    switch(*opcode) {
        case 0x00: break;
        case 0x01:
            lxi(state, &state -> b, &state -> c, nextWord(state));
            break;

        case 0x02:
            stax(state, state -> b, state -> c, state -> a);
            break;

        case 0x03:
            inx(&state -> a, &state -> b);

        case 0x04:
            inr(state, &state -> b);

        case 0x05:
            dcr(state, &state -> b);

        case 0x06:
            state -> b = nextByte(state);
            break;

        case 0x07: {
            uint8_t leftMost =  state -> a >> 7;
            state -> cc.cy = leftMost;
            state -> a = (state -> a << 1) | leftMost;
            break;
        }

        case 0x08:
            UnimplementedInstruction(state);
            break;

        case 0x09:
            dad(state, &state -> b, &state -> c);

        case 0x0a:
            state -> a = getMem(state, state -> b, state -> c);
            break;

        case 0x0b:
            dcx(&state -> b, &state -> c);

        case 0x0c:
            inr(state, &state -> c);

        case 0x0d:
            dcr(state, &state -> c);

        case 0x0e:
            state -> c = nextByte(state);
            break;

        // rotate instruction
        // rrc instruction
        case 0x0f: {
            // bit mask applied to isolate the right most bit
            uint8_t rightMost = state -> a & 1;
            state -> cc.cy = rightMost;
            // move the bits to the right by 1 and move the rightmost bit to the first bit
            state -> a = (state -> a >> 1) | rightMost << 7;
            break;
        }

        case 0x10:
            UnimplementedInstruction(state);
            break;

        case 0x11:
            lxi(state, &state -> d, &state -> e, nextWord(state));
            break;

        case 0x12:
            stax(state, state -> d, state -> e, state -> a);
            break;

        case 0x13:
            inx(&state -> d, &state -> e);

        case 0x14:
            inr(state, &state -> d);

        case 0x15:
            dcr(state, &state -> d);

        case 0x16:
            state -> d = nextByte(state);
            break;

        // ral
        case 0x17: {
            uint8_t leftMost = state -> a >> 7;
            state -> a = (state -> a << 1) | state -> cc.cy << 7;
            state -> cc.cy = leftMost;
            break;
        }

        case 0x18:
            UnimplementedInstruction(state);
            break;

        case 0x19:
            dad(state, &state -> d, &state -> e);

        case 0x1a:
            state -> a = getMem(state, state -> b, state -> c);
            break;

        case 0x1b:
            dcx(&state -> d, &state -> e);

        case 0x1c:
            inr(state, &state -> e);

        case 0x1d:
            dcr(state, &state -> e);

        case 0x1e:
            state -> e = nextByte(state);
            break;

        //rar
        case 0x1f: {
            uint8_t rightMost = state -> a & 1;
            state -> a = (state -> a >> 1) | (state -> cc.cy << 7);
            state -> cc.cy = rightMost;
            break;
        }

        case 0x20:
            UnimplementedInstruction(state);
            break;


        case 0x21:
            lxi(state, &state -> h, &state -> l, nextWord(state));
            break;

        case 0x22:
            stax(state, state -> h, state -> l, state -> a);
            break;

        case 0x23:
            inx(&state -> a, &state -> h);

        case 0x24:
            inr(state, &state -> h);

        case 0x25:
            dcr(state, &state -> h);

        case 0x26:
            state -> h = nextByte(state);
            break;

        //daa
        // space invaders does not use the daa instruction
        case 0x27:
            // lower nibble adjustment
            if (((state -> a & 0xf) > 9) || state -> cc.ac == 1) {
                state -> a = state -> a + 6;
            }

            // higher nibble adjustment
            uint8_t higherNibble = (state -> a & 0x0f) >> 4;
            if ((higherNibble > 9) || (state -> cc.cy == 1)) {
                state -> a = state -> a + 1;
            }
            break;

        case 0x28:
            UnimplementedInstruction(state);

        case 0x29:
            dad(state, &state -> h, &state -> l);

        case 0x2a:
            {
            uint16_t address = nextWord(state);
            lhld(state, address);
            }

        case 0x2b:
            dcx(&state -> h, &state -> l);

        case 0x2c:
            inr(state, &state -> l);

        case 0x2d:
            dcr(state, &state -> l);

        case 0x2e:
            state -> l = nextByte(state);
            break;

        case 0x2f:
            state -> a = !(state -> a);

        case 0x30:
            UnimplementedInstruction(state);


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
            state->b = getMem(state, state -> h, state -> l);
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
            state->c = getMem(state, state -> h, state -> l);
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
            state->d = getMem(state, state -> h, state -> l);
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
            state->e = getMem(state, state -> h, state -> l);
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
            state->h = getMem(state, state -> h, state -> l);
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
            state->l = getMem(state, state -> h, state -> l);
            break;
        case 0x6f:
            state->l = state->a;
            break;
        case 0x70:
            setMem(state, state -> h, state -> l, state -> b);
            break;
        case 0x71:
            setMem(state, state -> h, state -> l, state -> c);
            break;
        case 0x72:
            setMem(state, state -> h, state -> l, state -> d);
            break;
        case 0x73:
            setMem(state, state -> h, state -> l, state -> e);
            break;
        case 0x74:
            setMem(state, state -> h, state -> l, state -> h);
            break;
        case 0x75:
            setMem(state, state -> h, state -> l, state -> l);
            break;
        case 0x76:
            printf("Halting emulation");
            exit(0);
            break;
        case 0x77:
            setMem(state, state -> h, state -> l, state -> a);
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
            state->a = getMem(state, state -> h, state -> l);
            break;
        case 0x7f:
            state->a = state->a;
            break;
    }
}
