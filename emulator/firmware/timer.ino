#include "timer.h"


Timer::Timer() {};

void Timer::set_ms(int time_ms) {
    _time_start = millis();
    _time_set_ms = time_ms;
    _is_alive = true;
}

bool Timer::finished() {
    return millis() - _time_start >= _time_set_ms;
}

bool Timer::is_alive() {
    return _is_alive;
}
