#ifndef EMULATOR_H
#define EMULATOR_H

#include <stddef.h>
#include <stdint.h>

// constants
#define MEMORY_SIZE 0x10000
#define MAX_MEMORY_SIZE (MEMORY_SIZE - 1)

#define S_FLAG (1 << 7)
#define Z_FLAG (1 << 6)
#define AC_FLAG (1 << 4)
#define P_FLAG (1 << 2)
#define CY_FLAG (1 << 0)
#define INCREMENT_FLAGS                                                        \
    (Z_FLAG | S_FLAG |                                                         \
     P_FLAG) // used for only the increment and decrement functions
#define ALL_FLAGS                                                              \
    (Z_FLAG | S_FLAG | P_FLAG | CY_FLAG) // used to set all the flags as true
                                         // (for the arithmetic instructions)
#define NON_CARRY_FLAGS                                                        \
    (Z_FLAG | S_FLAG |                                                         \
     P_FLAG) // used to set all the flags as true (for the logic instructions)
#define PSW_FLAGS                                                              \
    (Z_FLAG | S_FLAG | P_FLAG | CY_FLAG |                                      \
     AC_FLAG) // used to set all the flags as true (for the arithmetic and logic
              // instructions)

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
void EmulateInstruction(State *state);
void initialiseIO(State *state);
void loadRom(const char *filename, size_t fileSize, State *state);

#endif
