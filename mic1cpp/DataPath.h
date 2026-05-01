#pragma once
#include <array>
#include <cstdint>
#include <iostream>
#include "Control.h"
#include "Clock.h"

enum Register : uint8_t {
    PC      = 0,
    AC      = 1,
    SP      = 2,
    IR      = 3,
    TIR     = 4,
    ZERO    = 5,   // read-only constant: 0x0000
    POS_ONE = 6,   // read-only constant: 0x0001
    NEG_ONE = 7,   // read-only constant: 0xFFFF
    AMASK   = 8,   // address mask: 0x0FFF
    SMASK   = 9,   // stack mask: 0x00FF
    A_REG   = 10,
    B_REG   = 11,
    C_REG   = 12,
    D_REG   = 13,
    E_REG   = 14,
    F_REG   = 15,
};

class DataPath {
public:
    DataPath(int initialPc, int initialSp);

    // Called once per subcycle by the CPU. Performs the work appropriate
    // to the current subcycle as driven by the decoded microinstruction.
    void activate(const MicroWord& mir, ClockSubcycle subcycle);

    // Wired to the address/data bus — the CPU reads these after activate()
    // and passes them to Memory, then writes mbr back if a read occurred.
    uint16_t mar;
    uint16_t mbr;

    // Set by the ALU in subcycle 3; read by ControlStore in subcycle 4
    // to evaluate conditional branches.
    bool nBit;
    bool zBit;

    uint16_t getReg(uint8_t idx) const { return _regs[idx]; }
    void dumpRegisters() const;

private:
    std::array<uint16_t, 16> _regs;
    uint16_t _aLatch;
    uint16_t _bLatch;
    uint16_t _shifterResult;

    void _loadALatch(uint8_t aField);
    void _loadBLatch(uint8_t bField);
    void _writeBack(uint8_t cField, uint16_t value);
};
