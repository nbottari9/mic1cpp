#include "Control.h"

ControlStore::ControlStore(const char* promFilePath) {
    // burn in promfile
    std::ifstream promFile(promFilePath);
    std::string promFileName(promFilePath);
    promFileName = promFileName.substr(2);

    if (!promFile.is_open()) {
        std::cerr << "Error opening promfile at path: " << promFilePath << "!\n";
        exit(2);
    }

    MicroWord zeroedOut(0);

    // initialize control store
    _microMemory.resize(CONTROL_STORE_MAX_SIZE, zeroedOut);

    std::string line;
    size_t i = 0;
    while (std::getline(promFile, line)) {
        if (i > CONTROL_STORE_MAX_SIZE) throw std::overflow_error("You may not have more than 256 micro instructions in promfile as control store max size is " + std::to_string(CONTROL_STORE_MAX_SIZE));
        uint32_t raw = std::stoul(line, nullptr, 2); // the nullptr arg here ignores an start offset
        MicroWord instr(raw);
        _microMemory[i++] = (instr);
    }

    std::cout << "Successfully read in " << i << " micro instructions from \"" << promFileName << "\"!\n";

    
    promFile.close();
}

MicroWord::MicroWord(uint32_t raw) {
    this->amux      = (raw >> 31) & 1;
    this->cond      = (raw >> 29) & 0x3;
    this->alu       = (raw >> 27) & 0x3;
    this->shift     = (raw >> 25) & 0x3;
    this->mbr       = (raw >> 24) & 1;
    this->mar       = (raw >> 23) & 1;
    this->rd        = (raw >> 22) & 1;
    this->wr        = (raw >> 21) & 1;
    this->enc       = (raw >> 20) & 1;
    this->aField    = (raw >> 16) & 0xF;
    this->bField    = (raw >> 12) & 0xF;
    this->cField    = (raw >> 8) & 0xF;
    this->addr      = (raw >> 0) & 0xFF;
}

std::ostream& operator<<(std::ostream& os, const MicroWord& mw) {
    std::stringstream out;
    out << std::to_string(mw.amux);
    out << std::bitset<2>(mw.cond);
    out << std::bitset<2>(mw.alu);
    out << std::bitset<2>(mw.shift);
    out << std::to_string(mw.mbr);
    out << std::to_string(mw.mar);
    out << std::to_string(mw.rd);
    out << std::to_string(mw.wr);
    out << std::to_string(mw.enc);
    out << std::bitset<4>(mw.aField);
    out << std::bitset<4>(mw.bField);
    out << std::bitset<4>(mw.cField);
    out << std::bitset<8>(mw.addr);

    return os << out.str();
}

void MicroWord::printFormatted() {
    std::cout << "amux: " << std::to_string(this->amux) << "\n";
    std::cout << "cond: " << std::to_string(this->cond) << "\n";
    std::cout << "alu: " << std::to_string(this->alu) << "\n";
    std::cout << "shift: " << std::to_string(this->shift) << "\n";
    std::cout << "mbr: " << std::to_string(this->mbr) << "\n";
    std::cout << "mar: " << std::to_string(this->mar) << "\n";
    std::cout << "rd: " << std::to_string(this->rd) << "\n";
    std::cout << "wr: " << std::to_string(this->wr) << "\n";
    std::cout << "enc: " << std::to_string(this->enc) << "\n";
    std::cout << "aField: " << std::to_string(this->aField) << "\n";
    std::cout << "bField: " << std::to_string(this->bField) << "\n";
    std::cout << "cField: " << std::to_string(this->cField) << "\n";
    std::cout << "addr: " << std::to_string(this->addr) << "\n";

}

std::ostream& operator<<(std::ostream& os, ControlStore& cs) {
    for (const auto instruction : cs._microMemory) {
        os << instruction << "\n";
    }
    return os;
}

const MicroWord& ControlStore::getInstruction(size_t addr) const {
    return _microMemory.at(addr);
}