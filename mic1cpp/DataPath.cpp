#include "DataPath.h"

namespace Alu {
    extern uint16_t activate(uint8_t ctlBits, uint16_t left, uint16_t right, bool& n, bool& z);
}
namespace Shifter {
    extern uint16_t activate(uint8_t ctlBits, uint16_t input);
}

DataPath::DataPath(int pc, int sp) : mar(0), mbr(0), nBit(false), zBit(false),
                                     _aLatch(0), _bLatch(0), _shifterResult(0) {
    _regs.fill(0);

    // Read-only constants — never written by writeBack (cases 5-9 are absent from the switch)
    _regs[ZERO]    = 0x0000;
    _regs[POS_ONE] = 0x0001;
    _regs[NEG_ONE] = 0xFFFF;
    _regs[AMASK]   = 0x0FFF;
    _regs[SMASK]   = 0x00FF;

    // Valid PC range: 0-2047, valid SP range: 0-4095 (matches original bounds check)
    if (pc >= 0 && pc <= 2047 && sp >= 0 && sp <= 4095) {
        _regs[PC] = static_cast<uint16_t>(pc);
        _regs[SP] = static_cast<uint16_t>(sp);
    } else {
        _regs[PC] = 0x0000;
        _regs[SP] = 0x0F80;  // "0000111110000000" = 3968, the original default
    }
}

void DataPath::activate(const MicroWord& mir, ClockSubcycle subcycle) {
    switch (subcycle) {

        case SECOND_SUBCYCLE:
            _loadALatch(mir.aField);
            _loadBLatch(mir.bField);
            break;

        case THIRD_SUBCYCLE: {
            // AMUX selects the left ALU operand: MBR (from memory) or ALatch (from register)
            uint16_t left  = mir.amux ? mbr : _aLatch;
            uint16_t right = _bLatch;

            // MAR bit: load the memory address register from the lower 12 bits of BLatch.
            // Mirrors LoadMar() in the original: MAR[I] = DataLines[I+4], i.e. bits 4-15.
            if (mir.mar)
                mar = _bLatch & 0x0FFF;

            uint16_t aluResult = Alu::activate(mir.alu, left, right, nBit, zBit);
            _shifterResult     = Shifter::activate(mir.shift, aluResult);
            break;
        }

        case FOURTH_SUBCYCLE:
            // ENC bit gates write-back to the register file
            if (mir.enc)
                _writeBack(mir.cField, _shifterResult);

            // MBR bit: write the shifter result onto the data bus (for memory writes)
            if (mir.mbr)
                mbr = _shifterResult;
            break;

        default:
            break;
    }
}

void DataPath::_loadALatch(uint8_t aField) {
    _aLatch = _regs[aField];
}

void DataPath::_loadBLatch(uint8_t bField) {
    _bLatch = _regs[bField];
}

void DataPath::_writeBack(uint8_t cField, uint16_t value) {
    // Registers 5-9 are read-only constants — no cases for them, matching the original.
    // cField 16 (ENC=0, all-zero one-hot) is impossible here since enc already guards this.
    switch (cField) {
        case PC:    _regs[PC]    = value; break;
        case AC:    _regs[AC]    = value; break;
        case SP:    _regs[SP]    = value; break;
        case IR:    _regs[IR]    = value; break;
        case TIR:   _regs[TIR]   = value; break;
        case A_REG: _regs[A_REG] = value; break;
        case B_REG: _regs[B_REG] = value; break;
        case C_REG: _regs[C_REG] = value; break;
        case D_REG: _regs[D_REG] = value; break;
        case E_REG: _regs[E_REG] = value; break;
        case F_REG: _regs[F_REG] = value; break;
        default: break;  // 5-9: constants, silently ignored
    }
}

void DataPath::dumpRegisters() const {
    std::cout << "PC=" << _regs[PC]    << " AC=" << _regs[AC]
              << " SP=" << _regs[SP]    << " IR=" << _regs[IR]
              << " TI=" << _regs[TIR]   << "\n"
              << "AR=" << _regs[A_REG] << " BR=" << _regs[B_REG]
              << " CR=" << _regs[C_REG] << " DR=" << _regs[D_REG]
              << " ER=" << _regs[E_REG] << " FR=" << _regs[F_REG] << "\n";
}
