#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <sstream>
#include <exception>
#include <algorithm>

#define MICRO_WORD_SIZE sizeof(uint32_t)
#define CONTROL_STORE_MAX_SIZE 256

class MicroWord {
public:
    MicroWord(uint32_t raw);

    void printFormatted();
    friend std::ostream& operator<<(std::ostream& os, const MicroWord& mw);

    bool amux; // 0 = A latch 1 = MBR
    uint8_t cond; // 0 = no jmp, 1 = jmp if n=1, 2 jmp if z=1, 3= always jmp
    uint8_t alu; // 0 = A + B, 1 = A and B, 2 = A, 3 = NOT A
    uint8_t shift; // 0 = no shift, 1 = right shift, 2 = left shift
    bool mbr; // 0 = no, 1 = yes
    bool mar; // 0 = no, 1 = yes
    bool rd; // 0 = no, 1 = yes
    bool wr; // 0 = no, 1 = yes
    bool enc; // 0 = no, 1 = yes enable c, writeback to registers
    uint8_t aField; // register to put on the a bus
    uint8_t bField; // register to put on the b bus
    uint8_t cField; // register to write back to (after shifter)
    uint8_t addr; // address in micro memory to jump to, if jump 
};

class ControlStore {
public:
    ControlStore(const char* promFilePath);

    friend std::ostream& operator<<(std::ostream& os, ControlStore& cs); // output entire micro memory

    const MicroWord& GetMicroInstructionAt(size_t addr) const; // get one instruction from micro memory

    MicroWord* GetMIR() const;
    void IncrementMIR();
    void SetMIR();


private:
    std::vector<MicroWord> _microMemory;
    MicroWord* _mir;
};

