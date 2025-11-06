#include "emulator.h"
#include "memory.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

State *setupStateMachine() {

    // This allocates the memory for the state
    State *state = malloc(sizeof(State));
    if (state == NULL) {
        perror("Failed to allocate memory of the CPU");
        exit(EXIT_FAILURE);
    }
    memset(state, 0, sizeof(State)); // fills the state machine with zeroes

    // Allocates memory for the emulated systems memory
    state->memory = malloc(MEMORY_SIZE); // 64KB of memory
    if (state->memory == NULL) {
        perror("Failed to allocate memory for emulated system");
        free(state);
        exit(EXIT_FAILURE);
    }
    memset(state->memory, 0, MEMORY_SIZE); // clears all 64KB of memory

    // Initialise the condition codes with 0
    state->cc.z = 0;
    state->cc.s = 0;
    state->cc.p = 0;
    state->cc.cy = 0;
    state->cc.ac = 0;

    // Initialise the registers and state variables with 0
    state->a = 0;
    state->b = 0;
    state->c = 0;
    state->d = 0;
    state->e = 0;
    state->h = 0;
    state->l = 0;
    state->sp = 0;
    state->pc = 0;

    return state;
}

void initialiseIO(State *state) {
    // bit meanings in order
    // coin, p2 start, p1 start, always 1, p1 shoot, p1 left, p1 right, unused
    state->io.port1 = 0b00001000;

    // bit meanings in order
    // dip-switch, tilt, p2 shoot, p2 left, p2 right, rest of the bits mean
    // dip-switches for coin/bonus/lives
    state->io.port2 = 0b00000000;
}

// loads memory into state memory
void loadRom(const char *filename, size_t fileSize, State *state) {

    // Store the game binary in heap allocated space
    uint8_t *gameBinary = malloc(fileSize);
    if (!gameBinary) {
        perror("malloc failed");
        exit(1);
    }

    // opens the file
    FILE *file;
    file = fopen(filename, "rb");

    // checks if the file exists
    if (file == NULL) {
        perror("File returns null");
        exit(1);
    }

    // gets the size of the file into memory
    size_t romSize = fread(gameBinary, 1, fileSize, file);
    printf("Rom size is %zu\n", romSize);
    fclose(file);

    // check if the size matches
    if (romSize != fileSize) {
        fprintf(
            stderr,
            "Error: ROM size mismatch (expected %zu bytes, got %zu bytes)\n",
            fileSize, romSize);
        exit(1);
    }

    // writes the game file into memory
    for (int i = 0; i < fileSize; i++) {
        writeByte(state, i, gameBinary[i]);
    }
}

// stores the games metadata
struct gameMetadata {
    size_t fileSize;
    const char *filename;
};

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Usage: %s <romfile>\n", argv[0]);
        return 1;
    }

    FILE *rom = fopen(argv[1], "rb");
    if (!rom) {
        perror("Failed to open ROM");
        return 1;
    }

    // sets up the intial state machine
    State *state = setupStateMachine();

    // setup the IO
    initialiseIO(state);

    size_t bytesRead = fread(state->memory, 1, MEMORY_SIZE, rom);
    fclose(rom);

    if (bytesRead == 0) {
        fprintf(stderr, "Failed to read ROM\n");
        return 1;
    }

    // initialise the pointer values
    state->pc = 0x0000;
    state->sp = 0x2400;
    state->interruptEnabled = 0;

    // run the program loop
    while (1) {
        EmulateInstruction(state);
    }
    printf("-----Emulated successfully-----\n");
}
