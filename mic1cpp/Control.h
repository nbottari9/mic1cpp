#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#define MICRO_WORD_SIZE sizeof(uint32_t)
#define CONTROL_STORE_MAX_SIZE 256

class ControlStore {
public:
    ControlStore(std::string& promFilePath);

private:
    std::vector<uint32_t> _microMemory;
};