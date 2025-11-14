#include "debug.h"
#include <stdio.h>
#include <stdlib.h>

void outputStateValues(const State *state) {
    /* print out processor state */
    printf("\tC=%d,P=%d,S=%d,Z=%d\n", state->cc.cy, state->cc.p, state->cc.s,
           state->cc.z);
    printf(
        "\tA $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP %04x\n",
        state->a, state->b, state->c, state->d, state->e, state->h, state->l,
        state->sp);
}

// this is for any instruction that we have not yet implemented
void UnimplementedInstruction(const State *state, uint8_t opcode) {
    // Error messages
    fprintf(stderr, "Error: Unimplemented instruction 0x%02x encountered\n",
            opcode);
    fprintf(stderr, "Program counter: %x\n", state->pc);
    outputStateValues(state);

    // Terminates the execution
    exit(EXIT_FAILURE);
}