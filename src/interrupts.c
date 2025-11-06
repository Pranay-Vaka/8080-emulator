#include "cpu_control.h"
#include "emulator.h"

void rst(State *state, uint8_t n) { call(state, 8 * n); }