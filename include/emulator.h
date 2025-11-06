#ifndef EMULATOR_H
#define EMULATOR_H

#include <stddef.h>
#include <stdint.h>

// constants
#define MEMORY_SIZE 0x10000
#define MAX_MEMORY_SIZE (MEMORY_SIZE - 1)

// structs
typedef struct ConditionCodes {
    uint8_t z : 1, s : 1, p : 1, cy : 1, ac : 1, pad : 3;
} ConditionCodes;

typedef struct IO {
    uint16_t shift_register;
    uint8_t shift_offset;
    uint8_t port1;
    uint8_t port2;
} IO;

typedef struct State {
    uint8_t a, b, c, d, e, h, l;
    uint16_t sp, pc;
    uint8_t *memory;
    ConditionCodes cc;
    uint8_t interruptEnabled;
    IO io;
} State;

// function prototypes
State *setupStateMachine(void);
void Emulate(State *state);
void initializeIO(State *state);
uint8_t handle_IN(State *state, uint8_t port);
void handle_OUT(State *state, uint8_t port, uint8_t value);
void loadRom(const char *filename, size_t fileSize, State *state);

#endif
