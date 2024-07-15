#include <Button.h>

#include "packet_generator.h"

#define PIN           13
#define REPEAT        3
#define LONG_REPEAT   30
#define DELAY_MS      2000
#define DELAY_LONG_MS 5000

Button b2(2);
Button b3(3);
Button b4(4);
Button b5(5);
Button b6(6);
Button b7(7);
Button b8(8);
Button b9(9);

int led_stripe_val = 0;
int last_led_stripe_val = 0;

int led_8 = false;
int led_9 = false;

PacketGenerator pg(PIN);

State state = State::MEASURE_FIN_OK;

void sendInterleave(PacketGenerator pg, bool led_8, bool led_9, int led_stripe)
{
  return;
  for (int i = 0; i < REPEAT; i++) {
      pg.sendPacket(State::E1, led_8, led_9, led_stripe);
  }
}

void setup() {
    b2.begin();
    b3.begin();
    b4.begin();
    b5.begin();
    b6.begin();
    b7.begin();
    b8.begin();
    b9.begin();

    pg.begin();

    Serial.begin(9600);
}

void loop() {
    if (b2.pressed()) {
        Serial.println("B2 pressed");
        state = State::ON;
        sendInterleave(pg, led_8, led_9, led_stripe_val);
    }
    if (b3.pressed()) {
        Serial.println("B3 pressed");
        state = State::ON_VIB;
        sendInterleave(pg, led_8, led_9, led_stripe_val);
      }
    if (b4.pressed()) {
        Serial.println("B4 pressed");
        state = State::ON_VIB_SHOCK;
        sendInterleave(pg, led_8, led_9, led_stripe_val);
    }
    if (b5.pressed()) {
        Serial.println("B5 pressed");
        state = State::CALIB;
        sendInterleave(pg, led_8, led_9, led_stripe_val);
    }
    if (b6.pressed()) {
        Serial.println("B6 pressed");
        state = State::CALIB_FINISHED;
        sendInterleave(pg, led_8, led_9, led_stripe_val);
    }
    if (b7.pressed()) {
        Serial.println("B7 pressed");
        state = State::MEASURE;
        sendInterleave(pg, led_8, led_9, led_stripe_val);
    }
    if (b8.pressed()) {
        Serial.println("B8 pressed");
        state = State::MEASURE_FIN_SHOCK;
        /* state = State::MEASURE_FIN_VIB; */
        sendInterleave(pg, led_8, led_9, led_stripe_val);
    }
    if (b9.pressed()) {
        Serial.println("B9 pressed");
        state = State::BEEP;
        sendInterleave(pg, led_8, led_9, led_stripe_val);
    }

    led_stripe_val = analogRead(A0) / 171;  /* 1024 / 6 ~= 171 */
    if (led_stripe_val != last_led_stripe_val) {
        last_led_stripe_val = led_stripe_val;
        Serial.println(led_stripe_val);
    }
    pg.sendPacket(state, led_8, led_9, led_stripe_val);

/*
    // try every known state
    State s = State::ON; bool led_8 = false; bool led_9 = false; bool led_stripe = 0;
    for (int i = 0; i < REPEAT; i++) {
        pg.sendPacket(s, led_8, led_9, led_stripe);
    }
    delay(DELAY_MS);

    s = State::ON_VIB; led_8 = false; led_9 = false; led_stripe = 0;
    for (int i = 0; i < REPEAT; i++) {
        pg.sendPacket(s, led_8, led_9, led_stripe);
    }
    delay(DELAY_MS);

    s = State::ON_VIB_SHOCK; led_8 = false; led_9 = false; led_stripe = 0;
    for (int i = 0; i < REPEAT; i++) {
        pg.sendPacket(s, led_8, led_9, led_stripe);
    }
    delay(DELAY_MS);

    s = State::CALIB; led_8 = false; led_9 = false; led_stripe = 0;
    for (int i = 0; i < REPEAT; i++) {
        pg.sendPacket(s, led_8, led_9, led_stripe);
    }
    delay(DELAY_MS);

    s = State::CALIB_FINISHED; led_8 = false; led_9 = false; led_stripe = 0;
    for (int i = 0; i < REPEAT; i++) {
        pg.sendPacket(s, led_8, led_9, led_stripe);
    }
    delay(DELAY_MS);

    s = State::MEASURE; led_8 = false; led_9 = false; led_stripe = 0;
    for (int i = 0; i < REPEAT; i++) {
        pg.sendPacket(s, led_8, led_9, led_stripe);
    }
    delay(DELAY_LONG_MS);

    s = State::MEASURE_FIN_OK; led_8 = false; led_9 = false; led_stripe = 0;
    for (int i = 0; i < REPEAT; i++) {
        pg.sendPacket(s, led_8, led_9, led_stripe);
    }
    delay(DELAY_MS);

    s = State::MEASURE_FIN_SHOCK; led_8 = false; led_9 = false; led_stripe = 0;
    for (int i = 0; i < REPEAT; i++) {
        pg.sendPacket(s, led_8, led_9, led_stripe);
    }
    delay(DELAY_MS);

    s = State::MEASURE_FIN_VIB; led_8 = false; led_9 = false; led_stripe = 0;
    for (int i = 0; i < REPEAT; i++) {
        pg.sendPacket(s, led_8, led_9, led_stripe);
    }
    delay(DELAY_MS);

    s = State::BEEP; led_8 = false; led_9 = false; led_stripe = 0;
    for (int i = 0; i < REPEAT; i++) {
        pg.sendPacket(s, led_8, led_9, led_stripe);
    }
    delay(DELAY_MS);


    // now try some unknown states
    s = State::E1; led_8 = false; led_9 = false; led_stripe = 0;
    for (int i = 0; i < LONG_REPEAT; i++) {
        pg.sendPacket(s, led_8, led_9, led_stripe);
    }
    delay(DELAY_LONG_MS);

    s = State::E2; led_8 = false; led_9 = false; led_stripe = 0;
    for (int i = 0; i < LONG_REPEAT; i++) {
        pg.sendPacket(s, led_8, led_9, led_stripe);
    }
    delay(DELAY_LONG_MS);

    s = State::E3; led_8 = false; led_9 = false; led_stripe = 0;
    for (int i = 0; i < LONG_REPEAT; i++) {
        pg.sendPacket(s, led_8, led_9, led_stripe);
    }
    delay(DELAY_LONG_MS);

    s = State::E4; led_8 = false; led_9 = false; led_stripe = 0;
    for (int i = 0; i < LONG_REPEAT; i++) {
        pg.sendPacket(s, led_8, led_9, led_stripe);
    }
    delay(DELAY_LONG_MS);

    s = State::E5; led_8 = false; led_9 = false; led_stripe = 0;
    for (int i = 0; i < LONG_REPEAT; i++) {
        pg.sendPacket(s, led_8, led_9, led_stripe);
    }
    delay(DELAY_LONG_MS);

    s = State::E6; led_8 = false; led_9 = false; led_stripe = 0;
    for (int i = 0; i < LONG_REPEAT; i++) {
        pg.sendPacket(s, led_8, led_9, led_stripe);
    }
    delay(DELAY_LONG_MS);
*/
}
