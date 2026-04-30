#include "Clock.h"

Clock::Clock() {
    _subcycle = ClockSubcycle::INIT;
    _cycle = 0;
};

const ClockSubcycle Clock::getSubcycle() const {
    return _subcycle;
}

void Clock::generatePulse() {
    _subcycle = static_cast<ClockSubcycle>((static_cast<int>(_subcycle) % 4) + 1);
    if (_subcycle == ClockSubcycle::FIRST_SUBCYCLE) _cycle++;
}