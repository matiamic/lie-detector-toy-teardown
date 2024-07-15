#include "packet_generator.h"
/* #include <Arduino.h> */

#define START_LOW_US  2260
#define START_HIGH_US 3786

#define LONG_LOW_US   688
#define LONG_HIGH_US  1505

#define TINY_LOW_US   1652
#define TINY_HIGH_US  545

PacketGenerator::PacketGenerator(uint8_t pin) : _pin(pin)
{
}

void PacketGenerator::begin() {
    pinMode(_pin, OUTPUT);
}

void PacketGenerator::sendPacket(State state, bool led_8, bool led_9, int led_stripe) {
    // start by start
    _send_start();
    // continue with state, first 6 bits
    uint16_t s = (uint16_t) state;
    for (int i = 0; i < 6; i++) {
        if (s & 0x80) {
            _send_tiny();
        } else {
            _send_long();
        }
        s <<= 1;
    }
    // led 8, led 9
    if (led_8) {
        _send_tiny();
    } else {
        _send_long();
    }
    if (led_9) {
        _send_tiny();
    } else {
        _send_long();
    }
    // reserved
    _send_long();
    // led_stripe
    for (int i = 0; i < 3; i++) {
        if (led_stripe & 0x4) {
            _send_tiny();
        } else {
            _send_long();
        }
        led_stripe <<= 1;
    }
    // set low at the end
    digitalWrite(_pin, LOW);
}

void PacketGenerator::_send_start(void) {
    digitalWrite(_pin, LOW);
    delayMicroseconds(START_LOW_US);
    digitalWrite(_pin, HIGH);
    delayMicroseconds(START_HIGH_US);
}

void PacketGenerator::_send_long(void) {
    digitalWrite(_pin, LOW);
    delayMicroseconds(LONG_LOW_US);
    digitalWrite(_pin, HIGH);
    delayMicroseconds(LONG_HIGH_US);
}

void PacketGenerator::_send_tiny(void) {
    digitalWrite(_pin, LOW);
    delayMicroseconds(TINY_LOW_US);
    digitalWrite(_pin, HIGH);
    delayMicroseconds(TINY_HIGH_US);
}
