#ifndef CPU_CONTROL_H
#define CPU_CONTROL_H

#include "emulator.h"
#include <stdint.h>

void ret(State *state);
void conditionalReturn(State *state, uint8_t condition);
void rnz(State *state);
void rz(State *state);
void rnc(State *state);
void rc(State *state);
void rp(State *state);
void rm(State *state);
void rpo(State *state);
void rpe(State *state);

void jmp(State *state, uint16_t addr);
void conditionalJump(State *state, uint16_t addr, uint8_t condition);
void jnz(State *state, uint16_t addr);
void jz(State *state, uint16_t addr);
void jnc(State *state, uint16_t addr);
void jc(State *state, uint16_t addr);
void jp(State *state, uint16_t addr);
void jm(State *state, uint16_t addr);
void jpo(State *state, uint16_t addr);
void jpe(State *state, uint16_t addr);

void call(State *state, uint16_t addr);
void conditionalCall(State *state, uint16_t addr, uint8_t condition);
void cnz(State *state, uint16_t addr);
void cz(State *state, uint16_t addr);
void cnc(State *state, uint16_t addr);
void cc(State *state, uint16_t addr);
void cp(State *state, uint16_t addr);
void cm(State *state, uint16_t addr);
void cpo(State *state, uint16_t addr);
void cpe(State *state, uint16_t addr);

#endif
