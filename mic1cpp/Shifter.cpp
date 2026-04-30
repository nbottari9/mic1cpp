#include <stdlib.h>

namespace Shifter {
    uint16_t activate(uint8_t ctlBits, uint16_t input) {
        uint16_t result;

        switch (ctlBits) {
            case 0b00: result = input;
            case 0b01: result = input >> 1;
            case 0b10: result = input << 1;
        }

        return result;
    }
}