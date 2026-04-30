#include "Control.h"

ControlStore::ControlStore(std::string& promFilePath) {
    // burn in promfile
    std::ifstream promFile(promFilePath);

    if (!promFile.is_open()) {
        std::cerr << "Error opening promfile at path: " << promFilePath << "!\n";
        exit(2);
    }

    // initialize control store
    _microMemory.resize(CONTROL_STORE_MAX_SIZE, 0);

    std::string line;
    while (std::getline(promFile, line)) {
        std::cout << line << "\n";
    }

    promFile.close();
}