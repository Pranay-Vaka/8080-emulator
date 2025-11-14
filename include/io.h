#ifndef IO_H
#define IO_H

#include "emulator.h"
#include <stdint.h>

void handle_OUT(State *state, uint8_t port, uint8_t value);
uint8_t handle_IN(const State *state, uint8_t port);

#endif
