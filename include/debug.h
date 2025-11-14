#ifndef DEBUG_H
#define DEBUG_H

#include "emulator.h"
#include <stdint.h>

void outputStateValues(const State *state);
void UnimplementedInstruction(const State *state, uint8_t opcode);

#endif
