#pragma once
enum ClockSubcycle { INIT, FIRST_SUBCYCLE, SECOND_SUBCYCLE, THIRD_SUBCYCLE, FOURTH_SUBCYCLE };

class Clock {
public:
    Clock();

    const ClockSubcycle getSubcycle() const;
    void generatePulse();

private:
    ClockSubcycle _subcycle;
    int _cycle;
    
};