#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MEMORY_SIZE 65536
#define MAX_MEMORY_SIZE (MEMORY_SIZE - 1)

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


// this is for any instruction that we have not yet implemented
void UnimplementedInstruction(State8080 *state, uint8_t opcode) {
    // Error messages
    fprintf(stderr, "Error: Unimplemented instruction 0x%02x encountered\n", opcode);
    fprintf(stderr, "Program counter: %x\n", state -> pc);

    // Terminates the execution
    exit(EXIT_FAILURE);
}

// FLAGS -- Constant flags made from bit shifts to manipulate different flags

// bit shifts each of the flags so that it can be logically OR'd to make it 1
#define Z_FLAG (1<<7)
#define S_FLAG (1<<6)
#define P_FLAG (1<<5)
#define CY_FLAG (1<<4)
#define AC_FLAG (1<<3)
#define INCREMENT_FLAGS (Z_FLAG | S_FLAG | P_FLAG) // used for only the increment and decrement functions
#define ALL_FLAGS (Z_FLAG | S_FLAG | P_FLAG | CY_FLAG) // used to set all the flags as true (for the arithmetic and logic instructions)
#define NON_CARRY_FLAGS (Z_FLAG | S_FLAG | P_FLAG) // used to set all the flags as true (for the arithmetic and logic instructions)


// CHECK FLAGS -- Checks certain values for the flags
// this will get rid of the first 8 bits since they could affect the value

// returns 1 if the value is equal to 0 and 0 if it is 1
uint8_t checkZero(uint8_t value) {
    return ((value & 0xff) == 0);
}

// returns 1 if it is positive and 0 if negative
uint8_t checkSign(uint8_t value) {
    return (value & 0xff) >> 7;
}

// returns 1 if there is even parity and 0 if there is odd parity.
uint8_t checkParity(uint8_t value) {

    uint8_t parity = 0;

    // loops through the only 8 bits
    for (int i = 0; i < 8; i++) {
        parity ^= (value & 1);  // XOR the least significant bit with the parity
        value >>= 1;            // Right shift to check the next bit
    }

    return !parity;
}

// returns 1 if there is a carry and returns 0 if there isn't
uint8_t checkCarry(uint16_t value) {
    return (value > 0xff);
}


// SET FLAGS -- function to set the flags for different groups of opcodes

// sets specific flags depending on the binary value given by flagMask
void setFlags(State8080 *state, uint16_t value, uint8_t flagMask) {

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
        state->cc.cy = checkCarry(value);
    }
}

// MAKE WORD -- This section is anything relating to the creation of a word (2 bytes) from byte pairs

// will make a 16 bit word
uint16_t combineBytesToWord(uint8_t highByte, uint8_t lowByte) {
    return (highByte << 8) | lowByte;
}

// BREAK WORD -- Breaking the 2 byte word into a pair of bytes

void splitWordToBytes(uint8_t *highByte, uint8_t *lowByte, uint16_t word) {
    *highByte = (word >> 8);
    *lowByte = word & 0xff;
}

// returns the byte at a certain index in the memory of the state machine
uint8_t readByte(State8080 *state, uint16_t index) {
    if (index >= MAX_MEMORY_SIZE) {
        fprintf(stderr, "Memory read out of bounds: 0x%04X\n", index);
        exit(EXIT_FAILURE);
    }
    return state -> memory[index];
}

// inserts byte into a certain index in the memory array
void writeByte(State8080 *state, uint16_t index, uint8_t value) {
    if (index >= MAX_MEMORY_SIZE) {
        fprintf(stderr, "Memory write out of bounds: 0x%04X\n", index);
        exit(EXIT_FAILURE);
    }
    state->memory[index] = value;
}

uint8_t nextByte(State8080 *state){
    return readByte(state, state -> pc++);
}

uint16_t nextWord(State8080 *state) {
    uint8_t highByte = nextByte(state);
    uint8_t lowByte = nextByte(state);

    return combineBytesToWord(highByte, lowByte);
}


// getters and setters for register pairs

// get value of the address pointed to by a register pair
uint16_t readMemoryAtRegPair(State8080 *state, uint8_t highByte, uint8_t lowByte) {
    return readByte(state, combineBytesToWord(highByte, lowByte));
}

// breaks the 16 bit value in half and assigns each half to the register pair respectfully
void writeRegPairFromWord(State8080 *state, uint8_t *highByte, uint8_t *lowByte, uint16_t value) {
        *highByte = (value >> 8) & 0xff; // the 0xff is redundant, but keeping it for clarity
        *lowByte = value & 0xff;
}

// set a value to the address pointed to by a register pair
void writeMemoryAtRegPair(State8080 *state, uint8_t highByte, uint8_t lowByte, uint16_t value) {
    uint16_t index = combineBytesToWord(highByte, lowByte);
    writeByte(state, index, value);
}


// adds values in two registers together and returns the 32 bit value
uint32_t addToRegPair(uint8_t *highByte, uint8_t *lowByte, uint16_t value) {
    uint16_t twoByteWord = combineBytesToWord(*highByte, *lowByte);
    value = twoByteWord + value;

    *highByte = (value & 0xff00) >> 8;
    *lowByte = (value & 0xff);

    return value;
}


// ARITHMETIC GROUP -- instructions for the arithmetic values in the isa

// Joins to 8 bit words, increments it and then splits it up again
// it is fine if the value overflows, this is expected behaviour.
void inx(uint8_t *highByte, uint8_t *lowByte) {
    uint16_t word = combineBytesToWord(*highByte, *lowByte);
    word++;
    splitWordToBytes(highByte, lowByte, word);
}

void inr(State8080 *state, uint8_t *value) {
    uint8_t result = *value + 1;
    setFlags(state, result, INCREMENT_FLAGS);
    *value = result; // discards the first 8 bits
}

// Joins to 8 bit words, decrements it and then splits it up again
// it is fine if the value overflows, this is expected behaviour.
void dcx(uint8_t *highByte, uint8_t *lowByte) {
    uint16_t word = combineBytesToWord(*highByte, *lowByte);
    word--;
    splitWordToBytes(highByte, lowByte, word);
}

void dcr(State8080 *state, uint8_t *value) {
    uint8_t result = *value - 1;
    setFlags(state, result, INCREMENT_FLAGS);
    *value = result; // discards the first 8 bits
};

// dad opcode that joins two bytes and then adds them to register h and l
void dad(State8080 *state, uint8_t *highByte, uint8_t *lowByte) {
    uint16_t value = combineBytesToWord(*highByte, *lowByte);
    uint32_t result = addToRegPair(&state -> h, &state -> l, value);
    state -> cc.cy = (result >> 16) & 1;
}

void lxi(State8080 *state, uint8_t *highByte, uint8_t *lowByte, uint16_t value) {
    writeRegPairFromWord(state, highByte, lowByte, value);
}


void lhld(State8080 *state, uint16_t address) {
    // stores the data in the address to register l
    state -> l = readByte(state, address);
    // stores the data in the address + 1 to register h
    state -> h = readByte(state, address + 1);
}

void mov(uint8_t *dest, uint8_t src) {
    *dest = src;
}


void Emulate(State8080 *state) {
    unsigned char *opcode = &(state -> memory[state->pc++]); // the opcode is indicated by the program counter's index in memory

    switch(*opcode) {
        case 0x00: break;
        case 0x01:
            lxi(state, &state -> b, &state -> c, nextWord(state));
            break;

        case 0x02:
            writeMemoryAtRegPair(state, state -> b, state -> c, state -> a);
            break;

        case 0x03:
            inx(&state -> b, &state -> c);
            break;

        case 0x04:
            inr(state, &state -> b);
            break;

        case 0x05:
            dcr(state, &state -> b);
            break;

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
            UnimplementedInstruction(state, *opcode);
            break;

        case 0x09:
            dad(state, &state -> b, &state -> c);
            break;

        case 0x0a:
            state -> a = readMemoryAtRegPair(state, state -> b, state -> c);
            break;

        case 0x0b:
            dcx(&state -> b, &state -> c);
            break;

        case 0x0c:
            inr(state, &state -> c);
            break;

        case 0x0d:
            dcr(state, &state -> c);
            break;

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
            UnimplementedInstruction(state, *opcode);
            break;

        case 0x11:
            lxi(state, &state -> d, &state -> e, nextWord(state));
            break;

        case 0x12:
            writeMemoryAtRegPair(state, state -> d, state -> e, state -> a);
            break;

        case 0x13:
            inx(&state -> d, &state -> e);
            break;

        case 0x14:
            inr(state, &state -> d);
            break;

        case 0x15:
            dcr(state, &state -> d);
            break;

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
            UnimplementedInstruction(state, *opcode);
            break;

        case 0x19:
            dad(state, &state -> d, &state -> e);
            break;

        case 0x1a:
            state -> a = readMemoryAtRegPair(state, state -> d, state -> e);
            break;

        case 0x1b:
            dcx(&state -> d, &state -> e);
            break;

        case 0x1c:
            inr(state, &state -> e);
            break;

        case 0x1d:
            dcr(state, &state -> e);
            break;

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
            UnimplementedInstruction(state, *opcode);
            break;


        case 0x21:
            lxi(state, &state -> h, &state -> l, nextWord(state));
            break;

        case 0x22:
            {
                uint16_t address = nextWord(state);
                writeByte(state, address, state -> l);
                writeByte(state, address + 1, state -> h);
                break;
            }

        case 0x23:
            inx(&state -> h, &state -> l);
            break;

        case 0x24:
            inr(state, &state -> h);
            break;

        case 0x25:
            dcr(state, &state -> h);
            break;

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
            UnimplementedInstruction(state, *opcode);
            break;

        case 0x29:
            dad(state, &state -> h, &state -> l);
            break;

        case 0x2a:
            {
            uint16_t address = nextWord(state);
            lhld(state, address);
            }
            break;

        case 0x2b:
            dcx(&state -> h, &state -> l);
            break;

        case 0x2c:
            inr(state, &state -> l);
            break;

        case 0x2d:
            dcr(state, &state -> l);
            break;

        case 0x2e:
            state -> l = nextByte(state);
            break;

        case 0x2f:
            state -> a = ~(state -> a);
            break;

        case 0x30:
            UnimplementedInstruction(state, *opcode);
            break;


        // mov opcodes
        case 0x40:
            mov(&state->b, state->b);
            break;
        case 0x41:
            mov(&state->b, state->c);
            break;
        case 0x42:
            mov(&state->b, state->d);
            break;
        case 0x43:
            mov(&state->b, state->e);
            break;
        case 0x44:
            mov(&state->b, state->h);
            break;
        case 0x45:
            mov(&state->b, state->l);
            break;
        case 0x46:
            mov(&state->b, readMemoryAtRegPair(state, state -> h, state -> l));
            break;
        case 0x47:
            mov(&state->b, state->a);
            break;
        case 0x48:
            mov(&state->c, state->b);
            break;
        case 0x49:
            mov(&state->c, state->c);
            break;
        case 0x4a:
            mov(&state->c, state->d);
            break;
        case 0x4b:
            mov(&state->c, state->e);
            break;
        case 0x4c:
            mov(&state->c, state->h);
            break;
        case 0x4d:
            mov(&state->c, state->l);
            break;
        case 0x4e:
            mov(&state->c, readMemoryAtRegPair(state, state -> h, state -> l));
            break;
        case 0x4f:
            mov(&state->c, state->a);
            break;
        case 0x50:
            mov(&state->d, state->b);
            break;
        case 0x51:
            mov(&state->d, state->c);
            break;
        case 0x52:
            mov(&state->d, state->d);
            break;
        case 0x53:
            mov(&state->d, state->e);
            break;
        case 0x54:
            mov(&state->d, state->h);
            break;
        case 0x55:
            mov(&state->d, state->l);
            break;
        case 0x56:
            mov(&state->d, readMemoryAtRegPair(state, state -> h, state -> l));
            break;
        case 0x57:
            mov(&state->d, state->a);
            break;
        case 0x58:
            mov(&state->e, state->b);
            break;
        case 0x59:
            mov(&state->e, state->c);
            break;
        case 0x5a:
            mov(&state->e, state->d);
            break;
        case 0x5b:
            mov(&state->e, state->e);
            break;
        case 0x5c:
            mov(&state->e, state->h);
            break;
        case 0x5d:
            mov(&state->e, state->l);
            break;
        case 0x5e:
            mov(&state->e, readMemoryAtRegPair(state, state -> h, state -> l));
            break;
        case 0x5f:
            mov(&state->e, state->a);
            break;
        case 0x60:
            mov(&state->h, state->b);
            break;
        case 0x61:
            mov(&state->h, state->c);
            break;
        case 0x62:
            mov(&state->h, state->d);
            break;
        case 0x63:
            mov(&state->h, state->e);
            break;
        case 0x64:
            mov(&state->h, state->h);
            break;
        case 0x65:
            mov(&state->h, state->l);
            break;
        case 0x66:
            mov(&state->h, readMemoryAtRegPair(state, state -> h, state -> l));
            break;
        case 0x67:
            mov(&state->h, state->a);
            break;
        case 0x68:
            mov(&state->l, state->b);
            break;
        case 0x69:
            mov(&state->l, state->c);
            break;
        case 0x6a:
            mov(&state->l, state->d);
            break;
        case 0x6b:
            mov(&state->l, state->e);
            break;
        case 0x6c:
            mov(&state->l, state->h);
            break;
        case 0x6d:
            mov(&state->l, state->l);
            break;
        case 0x6e:
            mov(&state->l, readMemoryAtRegPair(state, state -> h, state -> l));
            break;
        case 0x6f:
            mov(&state->l, state->a);
            break;
        case 0x70:
            writeMemoryAtRegPair(state, state -> h, state -> l, state -> b);
            break;
        case 0x71:
            writeMemoryAtRegPair(state, state -> h, state -> l, state -> c);
            break;
        case 0x72:
            writeMemoryAtRegPair(state, state -> h, state -> l, state -> d);
            break;
        case 0x73:
            writeMemoryAtRegPair(state, state -> h, state -> l, state -> e);
            break;
        case 0x74:
            writeMemoryAtRegPair(state, state -> h, state -> l, state -> h);
            break;
        case 0x75:
            writeMemoryAtRegPair(state, state -> h, state -> l, state -> l);
            break;
        case 0x76:
            printf("Halting emulation\n");
            exit(EXIT_SUCCESS);
            break;
        case 0x77:
            writeMemoryAtRegPair(state, state -> h, state -> l, state -> a);
            break;
        case 0x78:
            mov(&state->a, state->b);
            break;
        case 0x79:
            mov(&state->a, state->c);
            break;
        case 0x7a:
            mov(&state->a, state->d);
            break;
        case 0x7b:
            mov(&state->a, state->e);
            break;
        case 0x7c:
            mov(&state->a, state->h);
            break;
        case 0x7d:
            mov(&state->a, state->l);
            break;
        case 0x7e:
            mov(&state->a, readMemoryAtRegPair(state, state -> h, state -> l));
            break;
        case 0x7f:
            mov(&state->a, state->a);
            break;
        default:
            fprintf(stderr, "Unknown opcode: 0x%02x\n", *opcode);
            exit(EXIT_FAILURE);
    }
}
