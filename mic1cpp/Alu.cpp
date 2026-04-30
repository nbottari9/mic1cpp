#include <stdlib.h>

namespace Alu {
    uint16_t activate(uint8_t ctlBits, uint16_t left, uint16_t right, bool& n, bool& z) {
        uint16_t result;
        
        switch (ctlBits) {
            case 0b00: result = left + right; // 00: add(left, right)
            case 0b01: result = left & right; // 01: and(left, right)
            case 0b10: result = left;         // 10: pass through left
            case 0b11: result = ~left;        // 11: inv(left)
        }

        n = (result >> 15) & 1;
        z = (result == 0);

        return result;
    }
}