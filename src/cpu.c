#include "cpu_arithmetic.h"
#include "cpu_control.h"
#include "cpu_data.h"
#include "debug.h"
#include "emulator.h"
#include "flags.h"
#include "interrupts.h"
#include "io.h"
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>

static int hitCount = 0;

int EmulateInstruction(State *state) {
    unsigned char opcode =
        (state->memory[state->pc++]); // the opcode is indicated by the program
                                      // counter's index in memory
    hitCount = state->pc;
    // printf("PC value: %d\n", state -> pc);
    // printf("Opcode is %u\n", opcode);
    // outputStateValues(state);

    switch (opcode) {
    case 0x00:
        break;
    case 0x01:
        lxiRegPair(state, &state->b, &state->c, nextWord(state));
        break;

    case 0x02:
        writeMemoryAtRegPair(state, state->b, state->c, state->a);
        break;

    case 0x03:
        inxRegPair(state, &state->b, &state->c);
        break;

    case 0x04:
        inr(state, &state->b);
        break;

    case 0x05:
        dcr(state, &state->b);
        break;

    case 0x06:
        state->b = nextByte(state);
        break;

    case 0x07: {
        uint8_t leftMost = state->a >> 7;
        state->cc.cy = leftMost;
        state->a = (state->a << 1) | leftMost;
        break;
    }

    case 0x08:
        UnimplementedInstruction(state, opcode);
        outputStateValues(state);
        break;

    case 0x09:
        dadRegPair(state, &state->b, &state->c);
        break;

    case 0x0a:
        state->a = readMemoryAtRegPair(state, state->b, state->c);
        break;

    case 0x0b:
        dcxRegPair(state, &state->b, &state->c);
        break;

    case 0x0c:
        inr(state, &state->c);
        break;

    case 0x0d:
        dcr(state, &state->c);
        break;

    case 0x0e:
        state->c = nextByte(state);
        break;

    // rotate instruction
    // rrc instruction
    case 0x0f: {
        // bit mask applied to isolate the right most bit
        uint8_t rightMost = state->a & 1;
        state->cc.cy = rightMost;
        // move the bits to the right by 1 and move the rightmost bit to the
        // first bit
        state->a = (state->a >> 1) | rightMost << 7;
        break;
    }

    case 0x10:
        UnimplementedInstruction(state, opcode);
        outputStateValues(state);
        break;

    case 0x11:
        lxiRegPair(state, &state->d, &state->e, nextWord(state));
        break;

    case 0x12:
        writeMemoryAtRegPair(state, state->d, state->e, state->a);
        break;

    case 0x13:
        inxRegPair(state, &state->d, &state->e);
        break;

    case 0x14:
        inr(state, &state->d);
        break;

    case 0x15:
        dcr(state, &state->d);
        break;

    case 0x16:
        state->d = nextByte(state);
        break;

    // ral
    case 0x17: {
        uint8_t leftMost = state->a >> 7;
        state->a = (state->a << 1) | state->cc.cy << 7;
        state->cc.cy = leftMost;
        break;
    }

    case 0x18:
        UnimplementedInstruction(state, opcode);
        outputStateValues(state);
        break;

    case 0x19:
        dadRegPair(state, &state->d, &state->e);
        break;

    case 0x1a:
        state->a = readMemoryAtRegPair(state, state->d, state->e);
        break;

    case 0x1b:
        dcxRegPair(state, &state->d, &state->e);
        break;

    case 0x1c:
        inr(state, &state->e);
        break;

    case 0x1d:
        dcr(state, &state->e);
        break;

    case 0x1e:
        state->e = nextByte(state);
        break;

    // rar
    case 0x1f: {
        uint8_t rightMost = state->a & 1;
        state->a = (state->a >> 1) | (state->cc.cy << 7);
        state->cc.cy = rightMost;
        break;
    }

    case 0x20:
        UnimplementedInstruction(state, opcode);
        outputStateValues(state);
        break;

    case 0x21:
        lxiRegPair(state, &state->h, &state->l, nextWord(state));
        break;

    case 0x22: {
        uint16_t address = nextWord(state);
        writeByte(state, address, state->l);
        writeByte(state, address + 1, state->h);
        break;
    }

    case 0x23:
        inxRegPair(state, &state->h, &state->l);
        break;

    case 0x24:
        inr(state, &state->h);
        break;

    case 0x25:
        dcr(state, &state->h);
        break;

    case 0x26:
        state->h = nextByte(state);
        break;

    // daa
    //  space invaders does not use the daa instruction
    case 0x27:
        // lower nibble adjustment
        if (((state->a & 0xf) > 9) || state->cc.ac == 1) {
            state->a += 6;
        }

        // higher nibble adjustment
        uint8_t higherNibble = (state->a & 0x0f) >> 4;
        if ((higherNibble > 9) || (state->cc.cy == 1)) {
            state->a += 0x60;
        }
        break;

    case 0x28:
        UnimplementedInstruction(state, opcode);
        outputStateValues(state);
        break;

    case 0x29:
        dadRegPair(state, &state->h, &state->l);
        break;

    case 0x2a: {
        uint16_t address = nextWord(state);
        lhld(state, address);
    } break;

    case 0x2b:
        dcxRegPair(state, &state->h, &state->l);
        break;

    case 0x2c:
        inr(state, &state->l);
        break;

    case 0x2d:
        dcr(state, &state->l);
        break;

    case 0x2e:
        state->l = nextByte(state);
        break;

    case 0x2f:
        state->a = ~(state->a);
        break;

    case 0x30:
        UnimplementedInstruction(state, opcode);
        outputStateValues(state);
        break;

    case 0x31:
        lxi(state, &state->sp, nextWord(state));
        break;

    // sta instruction
    case 0x32:
        writeByte(state, nextWord(state), state->a);
        break;

    case 0x33:
        inx(state, &state->sp);
        break;

    // inr for HL
    case 0x34: {
        uint16_t address = combineBytesToWord(state->h, state->l);
        uint8_t value = readByte(state, address);
        inr(state, &value);
        writeByte(state, address, value);
        break;
    }

    // dcr for HL
    case 0x35: {
        uint16_t address = combineBytesToWord(state->h, state->l);
        uint8_t value = readByte(state, address);
        dcr(state, &value);
        writeByte(state, address, value);
        break;
    }

    // mvi for hl
    case 0x36: {
        uint8_t data = nextByte(state);
        writeMemoryAtHL(state, data);
        break;
    }

    // stc instruction
    case 0x37:
        state->cc.cy = 1;
        break;

    case 0x38:
        UnimplementedInstruction(state, 0x38);
        outputStateValues(state);
        break;

    case 0x39:
        dad(state, state->sp);
        break;

    // lda addr
    case 0x3a: {
        uint16_t addr = nextWord(state);
        state->a = readByte(state, addr);
    } break;

    case 0x3b:
        dcx(state, &state->sp);
        break;

    case 0x3c:
        inr(state, &state->a);
        break;

    case 0x3d:
        dcr(state, &state->a);
        break;

    // mvi
    case 0x3e: {
        state->a = nextByte(state);
        break;
    }

    // cmc
    case 0x3f:
        state->cc.cy = !state->cc.cy;
        break;

    // mov opcodes
    case 0x40:
        mov(state, &state->b, state->b);
        break;
    case 0x41:
        mov(state, &state->b, state->c);
        break;
    case 0x42:
        mov(state, &state->b, state->d);
        break;
    case 0x43:
        mov(state, &state->b, state->e);
        break;
    case 0x44:
        mov(state, &state->b, state->h);
        break;
    case 0x45:
        mov(state, &state->b, state->l);
        break;
    case 0x46:
        mov(state, &state->b, readMemoryAtHL(state));
        break;
    case 0x47:
        mov(state, &state->b, state->a);
        break;
    case 0x48:
        mov(state, &state->c, state->b);
        break;
    case 0x49:
        mov(state, &state->c, state->c);
        break;
    case 0x4a:
        mov(state, &state->c, state->d);
        break;
    case 0x4b:
        mov(state, &state->c, state->e);
        break;
    case 0x4c:
        mov(state, &state->c, state->h);
        break;
    case 0x4d:
        mov(state, &state->c, state->l);
        break;
    case 0x4e:
        mov(state, &state->c, readMemoryAtHL(state));
        break;
    case 0x4f:
        mov(state, &state->c, state->a);
        break;
    case 0x50:
        mov(state, &state->d, state->b);
        break;
    case 0x51:
        mov(state, &state->d, state->c);
        break;
    case 0x52:
        mov(state, &state->d, state->d);
        break;
    case 0x53:
        mov(state, &state->d, state->e);
        break;
    case 0x54:
        mov(state, &state->d, state->h);
        break;
    case 0x55:
        mov(state, &state->d, state->l);
        break;
    case 0x56:
        mov(state, &state->d, readMemoryAtHL(state));
        break;
    case 0x57:
        mov(state, &state->d, state->a);
        break;
    case 0x58:
        mov(state, &state->e, state->b);
        break;
    case 0x59:
        mov(state, &state->e, state->c);
        break;
    case 0x5a:
        mov(state, &state->e, state->d);
        break;
    case 0x5b:
        mov(state, &state->e, state->e);
        break;
    case 0x5c:
        mov(state, &state->e, state->h);
        break;
    case 0x5d:
        mov(state, &state->e, state->l);
        break;
    case 0x5e:
        mov(state, &state->e, readMemoryAtHL(state));
        break;
    case 0x5f:
        mov(state, &state->e, state->a);
        break;
    case 0x60:
        mov(state, &state->h, state->b);
        break;
    case 0x61:
        mov(state, &state->h, state->c);
        break;
    case 0x62:
        mov(state, &state->h, state->d);
        break;
    case 0x63:
        mov(state, &state->h, state->e);
        break;
    case 0x64:
        mov(state, &state->h, state->h);
        break;
    case 0x65:
        mov(state, &state->h, state->l);
        break;
    case 0x66:
        mov(state, &state->h, readMemoryAtHL(state));
        break;
    case 0x67:
        mov(state, &state->h, state->a);
        break;
    case 0x68:
        mov(state, &state->l, state->b);
        break;
    case 0x69:
        mov(state, &state->l, state->c);
        break;
    case 0x6a:
        mov(state, &state->l, state->d);
        break;
    case 0x6b:
        mov(state, &state->l, state->e);
        break;
    case 0x6c:
        mov(state, &state->l, state->h);
        break;
    case 0x6d:
        mov(state, &state->l, state->l);
        break;
    case 0x6e:
        mov(state, &state->l, readMemoryAtHL(state));
        break;
    case 0x6f:
        mov(state, &state->l, state->a);
        break;
    case 0x70:
        writeMemoryAtHL(state, state->b);
        break;
    case 0x71:
        writeMemoryAtHL(state, state->c);
        break;
    case 0x72:
        writeMemoryAtHL(state, state->d);
        break;
    case 0x73:
        writeMemoryAtHL(state, state->e);
        break;
    case 0x74:
        writeMemoryAtHL(state, state->h);
        break;
    case 0x75:
        writeMemoryAtHL(state, state->l);
        break;
    case 0x76:
        printf("Halting emulation\n");
        exit(EXIT_SUCCESS);
        break;
    case 0x77:
        writeMemoryAtHL(state, state->a);
        break;
    case 0x78:
        mov(state, &state->a, state->b);
        break;
    case 0x79:
        mov(state, &state->a, state->c);
        break;
    case 0x7a:
        mov(state, &state->a, state->d);
        break;
    case 0x7b:
        mov(state, &state->a, state->e);
        break;
    case 0x7c:
        mov(state, &state->a, state->h);
        break;
    case 0x7d:
        mov(state, &state->a, state->l);
        break;
    case 0x7e:
        mov(state, &state->a, readMemoryAtHL(state));
        break;
    case 0x7f:
        mov(state, &state->a, state->a);
        break;

    case 0x80:
        add(state, state->b);
        break;
    case 0x81:
        add(state, state->c);
        break;
    case 0x82:
        add(state, state->d);
        break;
    case 0x83:
        add(state, state->e);
        break;
    case 0x84:
        add(state, state->h);
        break;
    case 0x85:
        add(state, state->l);
        break;
    case 0x86:
        add(state, readMemoryAtHL(state));
        break;
    case 0x87:
        add(state, state->a);
        break;

    case 0x88:
        adc(state, state->b);
        break;
    case 0x89:
        adc(state, state->c);
        break;
    case 0x8a:
        adc(state, state->d);
        break;
    case 0x8b:
        adc(state, state->e);
        break;
    case 0x8c:
        adc(state, state->h);
        break;
    case 0x8d:
        adc(state, state->l);
        break;
    case 0x8e:
        adc(state, readMemoryAtHL(state));
        break;
    case 0x8f:
        adc(state, state->a);
        break;

    case 0x90:
        sub(state, state->b);
        break;
    case 0x91:
        sub(state, state->c);
        break;
    case 0x92:
        sub(state, state->d);
        break;
    case 0x93:
        sub(state, state->e);
        break;
    case 0x94:
        sub(state, state->h);
        break;
    case 0x95:
        sub(state, state->l);
        break;
    case 0x96:
        sub(state, readMemoryAtHL(state));
        break;
    case 0x97:
        sub(state, state->a);
        break;

    case 0x98:
        sbb(state, state->b);
        break;
    case 0x99:
        sbb(state, state->c);
        break;
    case 0x9a:
        sbb(state, state->d);
        break;
    case 0x9b:
        sbb(state, state->e);
        break;
    case 0x9c:
        sbb(state, state->h);
        break;
    case 0x9d:
        sbb(state, state->l);
        break;
    case 0x9e:
        sbb(state, readMemoryAtHL(state));
        break;
    case 0x9f:
        sbb(state, state->a);
        break;

    case 0xa0:
        ana(state, state->b);
        break;
    case 0xa1:
        ana(state, state->c);
        break;
    case 0xa2:
        ana(state, state->d);
        break;
    case 0xa3:
        ana(state, state->e);
        break;
    case 0xa4:
        ana(state, state->h);
        break;
    case 0xa5:
        ana(state, state->l);
        break;
    case 0xa6:
        ana(state, readMemoryAtHL(state));
        break;
    case 0xa7:
        ana(state, state->a);
        break;

    case 0xa8:
        xra(state, state->b);
        break;
    case 0xa9:
        xra(state, state->c);
        break;
    case 0xaa:
        xra(state, state->d);
        break;
    case 0xab:
        xra(state, state->e);
        break;
    case 0xac:
        xra(state, state->h);
        break;
    case 0xad:
        xra(state, state->l);
        break;
    case 0xae:
        xra(state, readMemoryAtHL(state));
        break;
    case 0xaf:
        xra(state, state->a);
        break;

    case 0xb0:
        ora(state, state->b);
        break;
    case 0xb1:
        ora(state, state->c);
        break;
    case 0xb2:
        ora(state, state->d);
        break;
    case 0xb3:
        ora(state, state->e);
        break;
    case 0xb4:
        ora(state, state->h);
        break;
    case 0xb5:
        ora(state, state->l);
        break;
    case 0xb6:
        ora(state, readMemoryAtHL(state));
        break;
    case 0xb7:
        ora(state, state->a);
        break;

    case 0xb8:
        cmp(state, state->b);
        break;
    case 0xb9:
        cmp(state, state->c);
        break;
    case 0xba:
        cmp(state, state->d);
        break;
    case 0xbb:
        cmp(state, state->e);
        break;
    case 0xbc:
        cmp(state, state->h);
        break;
    case 0xbd:
        cmp(state, state->l);
        break;
    case 0xbe:
        cmp(state, readMemoryAtHL(state));
        break;
    case 0xbf:
        cmp(state, state->a);
        break;

    // rnz
    case 0xc0:
        rnz(state);
        break;

    // pop b
    case 0xc1:
        popIntoRegPair(state, &state->b, &state->c);
        break;

    case 0xc2:
        jnz(state, nextWord(state));
        break;

    case 0xc3:
        jmp(state, nextWord(state));
        break;

    case 0xc4:
        cnz(state, nextWord(state));
        break;

    case 0xc5:
        pushIntoRegPair(state, &state->b, &state->c);
        break;

    case 0xc6:
        add(state, nextByte(state));
        break;

    case 0xc7:
        rst(state, 0);
        break;

    // rz
    case 0xc8:
        rz(state);
        break;

    // ret
    case 0xc9:
        ret(state);
        break;

    case 0xca:
        jz(state, nextWord(state));
        break;

    case 0xcb:
        UnimplementedInstruction(state, 0xcb);
        outputStateValues(state);
        break;

    case 0xcc:
        cz(state, nextWord(state));
        break;

    case 0xcd:
        call(state, nextWord(state));
        break;

    case 0xce:
        adc(state, nextByte(state));
        break;

    case 0xcf:
        rst(state, 1);
        break;

    // rnc
    case 0xd0:
        rnc(state);
        break;

    case 0xd1:
        popIntoRegPair(state, &state->d, &state->e);
        break;

    case 0xd2:
        jnc(state, nextWord(state));
        break;

    // OUT instruction only works with external hardware
    case 0xd3: {
        uint8_t port = nextByte(state);
        handle_OUT(state, port, state->a); // send register A to port
    } break;

    case 0xd4:
        cnc(state, nextWord(state));
        break;

    case 0xd5:
        pushIntoRegPair(state, &state->d, &state->e);
        break;

    // sui instruction
    case 0xd6:
        sub(state, nextByte(state));
        break;

    case 0xd7:
        rst(state, 2);
        break;

    case 0xd8:
        rc(state);
        break;

    case 0xd9:
        UnimplementedInstruction(state, 0xd9);
        outputStateValues(state);
        break;

    case 0xda:
        jc(state, nextWord(state));
        break;

    // IN instruction only works with external hardware
    case 0xdb: {
        uint8_t port = nextByte(state);
        state->a = handle_IN(state, port);
    } break;

    case 0xdc:
        cc(state, nextWord(state));
        break;

    case 0xdd:
        UnimplementedInstruction(state, 0xdd);
        outputStateValues(state);
        break;

    case 0xde:
        sbb(state, nextByte(state));
        break;

    case 0xdf:
        rst(state, 3);
        break;

    case 0xe0:
        rpo(state);
        break;

    case 0xe1:
        popIntoRegPair(state, &state->h, &state->l);
        break;

    case 0xe2:
        jpo(state, nextWord(state));
        break;

    // xthl
    case 0xe3: {
        // swap register l and byte pointed at stack pointer
        uint8_t temp = state->l;
        mov(state, &state->l, readByteAtSP(state));
        writeByteAtSP(state, temp);

        // swap register h and byte pointed at stack pointer + 1
        temp = state->h;
        mov(state, &state->h, readByte(state, state->sp + 1));
        writeByte(state, state->sp + 1, temp);
    } break;

    case 0xe4:
        cpo(state, nextWord(state));
        break;

    case 0xe5:
        pushIntoRegPair(state, &state->h, &state->l);
        break;

    case 0xe6:
        ana(state, nextByte(state));
        break;

    case 0xe7:
        rst(state, 4);
        break;

    case 0xe8:
        rpe(state);
        break;

    // pchl
    case 0xe9:
        writeDirectFromWord(state, &state->pc,
                            combineBytesToWord(state->h, state->l));
        break;

    case 0xea:
        jpe(state, nextWord(state));
        break;

    // xchg
    case 0xeb: {
        // swap register h and d
        uint8_t temp = state->h;
        mov(state, &state->h, state->d);
        mov(state, &state->d, temp);

        // swap register l and e
        temp = state->l;
        mov(state, &state->l, state->e);
        mov(state, &state->e, temp);
    } break;

    case 0xec:
        cpe(state, nextWord(state));
        break;

    case 0xed:
        UnimplementedInstruction(state, 0xed);
        outputStateValues(state);
        break;

    case 0xee:
        xra(state, nextByte(state));
        break;

    case 0xef:
        rst(state, 5);
        break;

    case 0xf0:
        rp(state);
        break;

    // popPSW
    case 0xf1: {
        uint8_t flags = readByteAtSP(state);
        stackArithmetic(state, 1);
        state->a = readByteAtSP(state);
        stackArithmetic(state, 1);
        setFlags(state, flags);
    } break;

    case 0xf2:
        jp(state, nextWord(state));
        break;

    // DI instruction
    case 0xf3:
        state->interruptEnabled = 0;
        break;

    case 0xf4:
        cp(state, nextWord(state));
        break;

    // pushPSW
    case 0xf5: {
        uint8_t flags = getFlags(state);
        stackArithmetic(state, -2);
        writeByte(state, state->sp + 1, state->a);
        writeByteAtSP(state, flags);
    } break;

    case 0xf6:
        ora(state, nextByte(state));
        break;

    case 0xf7:
        rst(state, 6);
        break;

    case 0xf8:
        rm(state);
        break;

    // sphl
    case 0xf9:
        state->sp = combineBytesToWord(state->h, state->l);
        break;

    case 0xfa:
        jm(state, nextWord(state));
        break;

    // EI instruction
    case 0xfb:
        state->interruptEnabled = 1;
        break;

    case 0xfc:
        cm(state, nextWord(state));
        break;

    case 0xfd:
        UnimplementedInstruction(state, 0xfd);
        outputStateValues(state);
        break;

    case 0xfe:
        cmp(state, nextByte(state));
        break;

    case 0xff:
        rst(state, 7);
        break;

    default:
        fprintf(stderr, "Unknown opcode: 0x%02x\n", opcode);
        exit(EXIT_FAILURE);
    }
}