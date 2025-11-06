#ifndef DEBUG_H
#define DEBUG_H

#include "emulator.h"
#include <stdint.h>

void outputStateValues(State *state);
void UnimplementedInstruction(State *state, uint8_t opcode);

#endif
