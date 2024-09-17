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


// MAKE WORD -- This section is anything relating to the creation of a word (2 bytes) from byte pairs

// will make a word
uint16_t makeWord(uint8_t a, uint8_t b) {
    uint16_t result = (a << 7) | b;
    return result;
}

// makes the values in register h and l into a word
uint16_t makeWordhl(State8080 *state) {
    return makeWord(state->h, state->l);
}

// makes the values in register b and c into a word
uint16_t makeWordbc(State8080 *state) {
    return makeWord(state->b, state->c);
}



// BREAK WORD -- Breaking the 2 byte word back into a pair of bytes

void breakWord(uint8_t *a, uint8_t *b, uint16_t word) {
    *a = (word >> 8) & 0xff;
    *b = word & 0xff;
}

// breaks the word into two bytes stored in h and l respectively
void breakWordhl(State8080 *state, uint16_t word) {
    breakWord(&state->h, &state->l,  word);
}

// breaks the word into two bytes stored in b and c respectively
void breakWordbc(State8080 *state, uint16_t word) {
    breakWord(&state->b, &state->c,  word);
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
            state->b = get_hl_mem(state);
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
            state->c = get_hl_mem(state);
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
            state->d = get_hl_mem(state);
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
            state->e = get_hl_mem(state);
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
            state->h = get_hl_mem(state);
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
            state->l = get_hl_mem(state);
            break;
        case 0x6f:
            state->l = state->a;
            break;
        case 0x70:
            set_hl_mem(state, state->b);
            break;
        case 0x71:
            set_hl_mem(state, state->c);
            break;
        case 0x72:
            set_hl_mem(state, state->d);
            break;
        case 0x73:
            set_hl_mem(state, state->e);
            break;
        case 0x74:
            set_hl_mem(state, state->h);
            break;
        case 0x75:
            set_hl_mem(state, state->l);
            break;
        case 0x76:
            printf("Execution has been halted\n");
            exit(0);
            break;
        case 0x77:
            set_hl_mem(state, state->a);
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
            state->a = get_hl_mem(state);
            break;
        case 0x7f:
            state->a = state->a;
            break;
    }

    // the opcode counts as one byte, so we always increment the program counter atleast by one
    state -> pc += 1;
}


