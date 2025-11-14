#include "emulator.h"
#include <stdio.h>

void handle_OUT(State *state, const uint8_t port, const uint8_t value) {
    switch (port) {
    case 2:
        state->io.shift_offset = value & 0x07;
        break;
    case 3:
        // Sound effects for later
        break;
    case 4:
        // Shift register works like fifo
        state->io.shift_register =
            (value << 8) | (state->io.shift_register >> 8);
        break;
    case 5:
        // sound effects
        break;
    default:
        fprintf(stderr, "Unhandled OUT port 0x%02X\n", port);
        break;
    }
}

uint8_t handle_IN(const State *state, const uint8_t port) {

    switch (port) {
    case 1:
        return state->io.port1;
    case 2:
        return state->io.port2;
    case 3: {
        // read from the shift register
        uint16_t v = state->io.shift_register >> (8 - state->io.shift_offset);
        return v & 0xff;
    }
    default:
        fprintf(stderr, "Unhandled IN port 0x%02X\n", port);
        return 0;
    }
}