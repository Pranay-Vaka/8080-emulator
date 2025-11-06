#include "cpu_control.h"
#include "memory.h"

// RETURN INSTRUCTIONS

void ret(State *state) { pop(state, &state->pc); }

void conditionalReturn(State *state, uint8_t condition) {
    if (condition)
        ret(state);
}

// return if value is 0
void rnz(State *state) { conditionalReturn(state, state->cc.z == 0); }

// return if value is 1
void rz(State *state) { conditionalReturn(state, state->cc.z == 1); }

// return if carry bit is not set
void rnc(State *state) { conditionalReturn(state, state->cc.cy == 0); }

// return if carry bit is set
void rc(State *state) { conditionalReturn(state, state->cc.cy == 1); }

// return if positive (sign bit is 0)
void rp(State *state) { conditionalReturn(state, state->cc.s == 0); }

// return if negative (sign bit is 1)
void rm(State *state) { conditionalReturn(state, state->cc.s == 1); }

// return if odd parity (parity bit is 0)
void rpo(State *state) { conditionalReturn(state, state->cc.p == 0); }

// return if even parity (parity bit is 1)
void rpe(State *state) { conditionalReturn(state, state->cc.p == 1); }

// JUMP INSTRUCTIONS

void jmp(State *state, uint16_t addr) { state->pc = addr; }

void conditionalJump(State *state, uint16_t addr, uint8_t condition) {
    if (condition) {
        jmp(state, addr);
    }
}

// jump if value is 0
void jnz(State *state, uint16_t addr) {
    conditionalJump(state, addr, state->cc.z == 0);
}

// jump if value is 1
void jz(State *state, uint16_t addr) {
    conditionalJump(state, addr, state->cc.z == 1);
}

// jump if carry bit is not set
void jnc(State *state, uint16_t addr) {
    conditionalJump(state, addr, state->cc.cy == 0);
}

// jump if carry bit is set
void jc(State *state, uint16_t addr) {
    conditionalJump(state, addr, state->cc.cy == 1);
}

// jump if positive (sign bit is 0)
void jp(State *state, uint16_t addr) {
    conditionalJump(state, addr, state->cc.s == 0);
}

// jump if negative (sign bit is 1)
void jm(State *state, uint16_t addr) {
    conditionalJump(state, addr, state->cc.s == 1);
}

// jump if odd parity (parity bit is 0)
void jpo(State *state, uint16_t addr) {
    conditionalJump(state, addr, state->cc.p == 0);
}

// jump if even parity (parity bit is 1)
void jpe(State *state, uint16_t addr) {
    conditionalJump(state, addr, state->cc.p == 1);
}

// CALL INSTRUCTIONS

void call(State *state, uint16_t addr) {
    push(state, state->pc); // pushes the return address to the stack
    jmp(state, addr);
}

void conditionalCall(State *state, uint16_t addr, uint8_t condition) {
    if (condition) {
        call(state, addr);
    }
}

// call if value is 0
void cnz(State *state, uint16_t addr) {
    conditionalCall(state, addr, state->cc.z == 0);
}

// call if value is 1
void cz(State *state, uint16_t addr) {
    conditionalCall(state, addr, state->cc.z == 1);
}

// call if carry bit is not set
void cnc(State *state, uint16_t addr) {
    conditionalCall(state, addr, state->cc.cy == 0);
}

// call if carry bit is set
void cc(State *state, uint16_t addr) {
    conditionalCall(state, addr, state->cc.cy == 1);
}

// call if positive (sign bit is 0)
void cp(State *state, uint16_t addr) {
    conditionalCall(state, addr, state->cc.s == 0);
}

// call if negative (sign bit is 1)
void cm(State *state, uint16_t addr) {
    conditionalCall(state, addr, state->cc.s == 1);
}

// call if odd parity (parity bit is 0)
void cpo(State *state, uint16_t addr) {
    conditionalCall(state, addr, state->cc.p == 0);
}

// call if even parity (parity bit is 1)
void cpe(State *state, uint16_t addr) {
    conditionalCall(state, addr, state->cc.p == 1);
}