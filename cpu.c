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
uint8_t readMemoryAtRegPair(State8080 *state, uint8_t highByte, uint8_t lowByte) {
    return readByte(state, combineBytesToWord(highByte, lowByte));
}

// breaks the 16 bit value in half and assigns each half to the register pair respectfully
void writeRegPairFromWord(State8080 *state, uint8_t *highByte, uint8_t *lowByte, uint16_t value) {
    *highByte = (value >> 8) & 0xff; // the 0xff is redundant, but keeping it for clarity
    *lowByte = value & 0xff;
}

void writeDirectFromWord(State8080 *state, uint16_t *index, uint16_t value) {
    *index = value;
}

// set a value to the address pointed to by a register pair
void writeMemoryAtRegPair(State8080 *state, uint8_t highByte, uint8_t lowByte, uint8_t value) {
    uint16_t index = combineBytesToWord(highByte, lowByte);
    writeByte(state, index, value);
}


// adds values in two registers together and returns the 32 bit value
uint32_t addToRegPair(State8080 *state, uint8_t *highByte, uint8_t *lowByte, uint16_t value) {
    uint16_t twoByteWord = combineBytesToWord(*highByte, *lowByte);
    value = twoByteWord + value;

    writeRegPairFromWord(state, highByte, lowByte, value);
    return value;
}


// ARITHMETIC GROUP -- instructions for the arithmetic values in the isa

// Joins to 8 bit words, increments it and then splits it up again
// it is fine if the value overflows, this is expected behaviour.
void inxRegPair(State8080 *state, uint8_t *highByte, uint8_t *lowByte) {
    uint16_t word = combineBytesToWord(*highByte, *lowByte);
    word++;
    writeRegPairFromWord(state, highByte, lowByte, word);
}

// increments the 16 bit word
void inx(State8080 *state, uint16_t *value) {
    (*value)++;
}

void inr(State8080 *state, uint8_t *value) {
    uint8_t result = *value + 1;
    setFlags(state, result, INCREMENT_FLAGS);
    *value = result; // discards the first 8 bits
}

// Joins to 8 bit words, decrements it and then splits it up again
// it is fine if the value overflows, this is expected behaviour.
void dcxRegPair(State8080 *state, uint8_t *highByte, uint8_t *lowByte) {
    uint16_t word = combineBytesToWord(*highByte, *lowByte);
    word--;
    writeRegPairFromWord(state, highByte, lowByte, word);
}

// decrements the 16 bit word
void dcx(State8080 *state, uint16_t *value) {
    (*value)--;
}

void dcr(State8080 *state, uint8_t *value) {
    uint8_t result = *value - 1;
    setFlags(state, result, INCREMENT_FLAGS);
    *value = result; // discards the first 8 bits
};


// dad opcode takes word and then adds them to register h and l
void dad(State8080 *state, uint16_t value) {
    uint32_t result = addToRegPair(state, &state -> h, &state -> l, value);
    state -> cc.cy = (result >> 16) & 1;
}

// dadRegPair opcode that joins two bytes and then adds them to register h and l
void dadRegPair(State8080 *state, uint8_t *highByte, uint8_t *lowByte) {
    uint16_t value = combineBytesToWord(*highByte, *lowByte);
    dad(state, value);
}

// loads a 16 bit value into a register pair
void lxiRegPair(State8080 *state, uint8_t *highByte, uint8_t *lowByte, uint16_t value) {
    writeRegPairFromWord(state, highByte, lowByte, value);
}

// loads a 16 bit value into a register pair
void lxi(State8080 *state, uint16_t *index, uint16_t value) {
    writeDirectFromWord(state, index, value);
}

void lhld(State8080 *state, uint16_t address) {
    // stores the data in the address to register l
    state -> l = readByte(state, address);
    // stores the data in the address + 1 to register h
    state -> h = readByte(state, address + 1);
}

void mov(State8080* state, uint8_t *dest, uint8_t src) {
    *dest = src;
}


void Emulate(State8080 *state) {
    unsigned char *opcode = &(state -> memory[state->pc++]); // the opcode is indicated by the program counter's index in memory

    switch(*opcode) {
        case 0x00: break;
        case 0x01:
            lxiRegPair(state, &state -> b, &state -> c, nextWord(state));
            break;

        case 0x02:
            writeMemoryAtRegPair(state, state -> b, state -> c, state -> a);
            break;

        case 0x03:
            inxRegPair(state, &state -> b, &state -> c);
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
            dadRegPair(state, &state -> b, &state -> c);
            break;

        case 0x0a:
            state -> a = readMemoryAtRegPair(state, state -> b, state -> c);
            break;

        case 0x0b:
            dcxRegPair(state, &state -> b, &state -> c);
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
            lxiRegPair(state, &state -> d, &state -> e, nextWord(state));
            break;

        case 0x12:
            writeMemoryAtRegPair(state, state -> d, state -> e, state -> a);
            break;

        case 0x13:
            inxRegPair(state, &state -> d, &state -> e);
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
            dadRegPair(state, &state -> d, &state -> e);
            break;

        case 0x1a:
            state -> a = readMemoryAtRegPair(state, state -> d, state -> e);
            break;

        case 0x1b:
            dcxRegPair(state, &state -> d, &state -> e);
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
            lxiRegPair(state, &state -> h, &state -> l, nextWord(state));
            break;

        case 0x22:
            {
                uint16_t address = nextWord(state);
                writeByte(state, address, state -> l);
                writeByte(state, address + 1, state -> h);
                break;
            }

        case 0x23:
            inxRegPair(state, &state -> h, &state -> l);
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
            dadRegPair(state, &state -> h, &state -> l);
            break;

        case 0x2a:
            {
            uint16_t address = nextWord(state);
            lhld(state, address);
            }
            break;

        case 0x2b:
            dcxRegPair(state, &state -> h, &state -> l);
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

        case 0x31:
            lxi(state, &state -> sp, nextWord(state));
            break;

        // sta instruction
        case 0x32:
            writeByte(state, nextWord(state), state -> a);
            break;

        case 0x33:
            inx(state, &state -> sp);
            break;

        // inr for HL
        case 0x34:
            {
                uint16_t address = combineBytesToWord(state->h, state->l);
                uint8_t value = readByte(state, address);
                inr(state, &value);
                writeByte(state, address, value);
                break;
            }

        // dcr for HL
        case 0x35:
            {
                uint16_t address = combineBytesToWord(state->h, state->l);
                uint8_t value = readByte(state, address);
                dcr(state, &value);
                writeByte(state, address, value);
                break;
            }

        // mvi for hl
        case 0x36:
            {
                uint8_t data = nextByte(state);
                writeMemoryAtRegPair(state, state -> h, state -> l, data);
                break;
            }

        // stc instruction
        case 0x37:
            state -> cc.cy = 1;
            break;

        case 0x38:
            UnimplementedInstruction(state, 0x38);
            break;

        case 0x39:
            dad(state, state -> sp);
            break;

        // lda addr
        case 0x3a:
            state -> a = nextWord(state);
            break;

        case 0x3b:
            dcx(state, &state -> sp);
            break;

        case 0x3c:
            inr(state, &state -> a);
            break;

        case 0x3d:
            dcr(state, &state -> a);
            break;

        // mvi
        case 0x3e:
            {
                state -> a = nextByte(state);
                break;
            }

        // cmc
        case 0x3f:
            state -> cc.cy = ~(state -> cc.cy);
            break;

        // mov opcodes
        case 0x40:
            mov(state, &state->b, state->b);
            break;
        case 0x41:
            mov(state, &state->b, state->c);
            break;
        case 0x42:
            mov(state, &state->b, state->d);
            break;
        case 0x43:
            mov(state, &state->b, state->e);
            break;
        case 0x44:
            mov(state, &state->b, state->h);
            break;
        case 0x45:
            mov(state, &state->b, state->l);
            break;
        case 0x46:
            mov(state, &state->b, readMemoryAtRegPair(state, state -> h, state -> l));
            break;
        case 0x47:
            mov(state, &state->b, state->a);
            break;
        case 0x48:
            mov(state, &state->c, state->b);
            break;
        case 0x49:
            mov(state, &state->c, state->c);
            break;
        case 0x4a:
            mov(state, &state->c, state->d);
            break;
        case 0x4b:
            mov(state, &state->c, state->e);
            break;
        case 0x4c:
            mov(state, &state->c, state->h);
            break;
        case 0x4d:
            mov(state, &state->c, state->l);
            break;
        case 0x4e:
            mov(state, &state->c, readMemoryAtRegPair(state, state -> h, state -> l));
            break;
        case 0x4f:
            mov(state, &state->c, state->a);
            break;
        case 0x50:
            mov(state, &state->d, state->b);
            break;
        case 0x51:
            mov(state, &state->d, state->c);
            break;
        case 0x52:
            mov(state, &state->d, state->d);
            break;
        case 0x53:
            mov(state, &state->d, state->e);
            break;
        case 0x54:
            mov(state, &state->d, state->h);
            break;
        case 0x55:
            mov(state, &state->d, state->l);
            break;
        case 0x56:
            mov(state, &state->d, readMemoryAtRegPair(state, state -> h, state -> l));
            break;
        case 0x57:
            mov(state, &state->d, state->a);
            break;
        case 0x58:
            mov(state, &state->e, state->b);
            break;
        case 0x59:
            mov(state, &state->e, state->c);
            break;
        case 0x5a:
            mov(state, &state->e, state->d);
            break;
        case 0x5b:
            mov(state, &state->e, state->e);
            break;
        case 0x5c:
            mov(state, &state->e, state->h);
            break;
        case 0x5d:
            mov(state, &state->e, state->l);
            break;
        case 0x5e:
            mov(state, &state->e, readMemoryAtRegPair(state, state -> h, state -> l));
            break;
        case 0x5f:
            mov(state, &state->e, state->a);
            break;
        case 0x60:
            mov(state, &state->h, state->b);
            break;
        case 0x61:
            mov(state, &state->h, state->c);
            break;
        case 0x62:
            mov(state, &state->h, state->d);
            break;
        case 0x63:
            mov(state, &state->h, state->e);
            break;
        case 0x64:
            mov(state, &state->h, state->h);
            break;
        case 0x65:
            mov(state, &state->h, state->l);
            break;
        case 0x66:
            mov(state, &state->h, readMemoryAtRegPair(state, state -> h, state -> l));
            break;
        case 0x67:
            mov(state, &state->h, state->a);
            break;
        case 0x68:
            mov(state, &state->l, state->b);
            break;
        case 0x69:
            mov(state, &state->l, state->c);
            break;
        case 0x6a:
            mov(state, &state->l, state->d);
            break;
        case 0x6b:
            mov(state, &state->l, state->e);
            break;
        case 0x6c:
            mov(state, &state->l, state->h);
            break;
        case 0x6d:
            mov(state, &state->l, state->l);
            break;
        case 0x6e:
            mov(state, &state->l, readMemoryAtRegPair(state, state -> h, state -> l));
            break;
        case 0x6f:
            mov(state, &state->l, state->a);
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
            mov(state, &state->a, state->b);
            break;
        case 0x79:
            mov(state, &state->a, state->c);
            break;
        case 0x7a:
            mov(state, &state->a, state->d);
            break;
        case 0x7b:
            mov(state, &state->a, state->e);
            break;
        case 0x7c:
            mov(state, &state->a, state->h);
            break;
        case 0x7d:
            mov(state, &state->a, state->l);
            break;
        case 0x7e:
            mov(state, &state->a, readMemoryAtRegPair(state, state -> h, state -> l));
            break;
        case 0x7f:
            mov(state, &state->a, state->a);
            break;
        default:
            fprintf(stderr, "Unknown opcode: 0x%02x\n", *opcode);
            exit(EXIT_FAILURE);
    }
}
