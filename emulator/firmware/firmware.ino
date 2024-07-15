#include "packet_generator.h"
#include "timer.h"

#define START_BUTTON_PIN A6  // A6 is ONLY! analog in
#define PROTOCOL_OUT_PIN 10

#define SHOCK_OUT_PIN           13
#define SHOCK_TRIMMER_PIN       A7
#define SHOCK_INTENSITY_OUT_PIN 11
#define REPEAT_SHOCK_PACKET     10
#define SHOCK_DURATION_MS       1472

#define NUM_LEDS 9
#define NUM_LEDS_WITH_PIN 7
#define LED_1_PIN A0
#define LED_2_PIN A1
#define LED_3_PIN A2
#define LED_4_PIN A3
#define LED_5_PIN A4
#define LED_6_PIN A5
#define LED_7_PIN 12

#define N_ROUNDS 8
#define N_ROUND_PINS 8
#define ROUND_0_PIN 2
#define ROUND_1_PIN 3
#define ROUND_2_PIN 4
#define ROUND_3_PIN 5
#define ROUND_4_PIN 6
#define ROUND_5_PIN 7
#define ROUND_6_PIN 8
#define ROUND_7_PIN 9

#define CALIB_DURATION_MS 16000
#define MEASURE_DURATION_MS 16000
#define LED_STRIPE_INCR_INTERVAL_MS 3000

#define LED_ROTATION_INTERVAL_MS 100

enum class PinType {
    DIGITAL,
    DIGITAL_ANALOG,
    ANALOG
};

int led_stripe_val = 0;

bool leds[NUM_LEDS + 1] = { false };
int led_pins[NUM_LEDS_WITH_PIN + 1] = {
    0,
    LED_1_PIN,
    LED_2_PIN,
    LED_3_PIN,
    LED_4_PIN,
    LED_5_PIN,
    LED_6_PIN,
    LED_7_PIN,
};

int round_pins[N_ROUND_PINS] = {
    ROUND_0_PIN,
    ROUND_1_PIN,
    ROUND_2_PIN,
    ROUND_3_PIN,
    ROUND_4_PIN,
    ROUND_5_PIN,
    ROUND_6_PIN,
    ROUND_7_PIN,
};

PacketGenerator pg(PROTOCOL_OUT_PIN);
Timer state_timer;
Timer led_timer;
Timer stripe_timer;

State state = State::ON;

int round_num = -1;

void setup() {
    // init pins
    // leds:
    pinMode(LED_1_PIN, OUTPUT);
    pinMode(LED_2_PIN, OUTPUT);
    pinMode(LED_3_PIN, OUTPUT);
    pinMode(LED_4_PIN, OUTPUT);
    pinMode(LED_5_PIN, OUTPUT);
    pinMode(LED_6_PIN, OUTPUT);
    pinMode(LED_7_PIN, OUTPUT);
    light_leds();

    // switch array:
    pinMode(ROUND_0_PIN, INPUT_PULLUP);
    pinMode(ROUND_1_PIN, INPUT_PULLUP);
    pinMode(ROUND_2_PIN, INPUT_PULLUP);
    pinMode(ROUND_3_PIN, INPUT_PULLUP);
    pinMode(ROUND_4_PIN, INPUT_PULLUP);
    pinMode(ROUND_5_PIN, INPUT_PULLUP);
    pinMode(ROUND_6_PIN, INPUT_PULLUP);
    pinMode(ROUND_7_PIN, INPUT_PULLUP);

    // shock management:
    pinMode(SHOCK_OUT_PIN, OUTPUT);
    digitalWrite(SHOCK_OUT_PIN, HIGH);
    pinMode(SHOCK_INTENSITY_OUT_PIN, OUTPUT);  // pwm (5 or 6 - faster 980 Hz pwm)
    pinMode(SHOCK_TRIMMER_PIN, INPUT);  // analog
    delay(100); // wait for the voltage to settle
    int shock_intensity = (1023 - analogRead(SHOCK_TRIMMER_PIN)) / 4;
    analogWrite(SHOCK_INTENSITY_OUT_PIN, shock_intensity);

    // button:
    init_button(START_BUTTON_PIN);

    pg.begin();

    Serial.begin(115200);
}

void loop() {
    switch (state) { // state machine
        case State::ON:
            if (button_pressed(START_BUTTON_PIN)) { // ON->CALIB
                Serial.println("on->calib");
                state = State::CALIB;
                state_timer.set_ms(CALIB_DURATION_MS);

                // init CALIB
                leds[1] = true;
                light_leds();
                led_timer.set_ms(LED_ROTATION_INTERVAL_MS);
                stripe_timer.set_ms(LED_STRIPE_INCR_INTERVAL_MS);
            }
            break;
        case State::CALIB:
            if (state_timer.finished()) {  // CALIB->CALIB_FINISHED
                Serial.println("calib->calib_finished");
                state = State::CALIB_FINISHED;

                // init CALIB_FINISHED
                shutdown_leds();
                led_stripe_val = 5;
            } else {  // CALIB maintenance
                if (led_timer.finished()) {
                    rotate_leds();
                    led_timer.set_ms(LED_ROTATION_INTERVAL_MS);
                }
                if (stripe_timer.finished()) {
                    if (++led_stripe_val == 6) led_stripe_val = 5;
                    stripe_timer.set_ms(LED_STRIPE_INCR_INTERVAL_MS);
                }
            }
            break;
        case State::CALIB_FINISHED:
        case State::MEASURE_FIN_OK:
        case State::MEASURE_FIN_SHOCK:
            if (button_pressed(START_BUTTON_PIN)) { // start MEASURE
                Serial.println("calib/measure_fin->measure");
                state = State::MEASURE;
                state_timer.set_ms(MEASURE_DURATION_MS);

                // init MEASURE
                led_stripe_val = 0;
                if (++round_num == N_ROUNDS) {
                    round_num = 0;
                }
                leds[1] = true;
                light_leds();
                led_timer.set_ms(LED_ROTATION_INTERVAL_MS);
            }
            break;
        case State::MEASURE:
            if (state_timer.finished()) {
                Serial.println("measure->measure_fin");
                shutdown_leds();
                bool is_shock = ! digitalRead(round_pins[round_num]);
                if (is_shock) {
                    Serial.println("shock!");
                    state = State::MEASURE_FIN_SHOCK;
                    led_stripe_val = 5;
                    perform_shock();
                } else {
                    Serial.println("ok!");
                    state = State::MEASURE_FIN_OK;
                    led_stripe_val = 1;
                }
            } else { // maintanance
                if (led_timer.finished()) {
                    rotate_leds();
                    led_timer.set_ms(LED_ROTATION_INTERVAL_MS);
                }
            }
            break;

    }
    pg.sendPacket(state, leds[8], leds[9], led_stripe_val);
}

void light_leds() {
    for (int i = 1; i <= NUM_LEDS_WITH_PIN; i++) {
        if (leds[i]) {
            digitalWrite(led_pins[i], HIGH);
        } else {
            digitalWrite(led_pins[i], LOW);
        }
    }
}

void rotate_leds() {
    bool tmp1 = leds[1];
    bool tmp2;
    for (int i = 1; i < NUM_LEDS; i++) {
        tmp2 = leds[i + 1];
        leds[i + 1] = tmp1;
        // swap
        bool aux = tmp2;
        tmp2 = tmp1;
        tmp1 = aux;
    }
    leds[1] = tmp1;
    light_leds();
}

void shutdown_leds() {
    for (int i = 1; i <= NUM_LEDS; i++) {
        leds[i] = 0;
    }
    light_leds();
}

void perform_shock() {
    Timer shock_timer;
    shock_timer.set_ms(SHOCK_DURATION_MS);
    digitalWrite(SHOCK_OUT_PIN, LOW);
    while (! shock_timer.finished()) {
        pg.sendPacket(State::MEASURE_FIN_SHOCK, leds[8], leds[9], led_stripe_val);
    }
    digitalWrite(SHOCK_OUT_PIN, HIGH);
}

void init_button(int pin) {
    switch (pin_type(pin)) {
        case PinType::DIGITAL:
        case PinType::DIGITAL_ANALOG:
            pinMode(pin, INPUT_PULLUP);
            break;
        case PinType::ANALOG:
            pinMode(pin, INPUT);
            break;
    }
}

bool button_pressed(int pin) {
    switch (pin_type(pin)) {
        case PinType::DIGITAL:
        case PinType::DIGITAL_ANALOG:
            return ! digitalRead(pin);
        case PinType::ANALOG:
            return analogRead(pin) < 512;
    }
}

PinType pin_type(int pin) {
    switch (pin) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
            return PinType::DIGITAL;
        case A0:
        case A1:
        case A2:
        case A3:
        case A4:
        case A5:
            return PinType::DIGITAL_ANALOG;
        case A6:
        case A7:
            return PinType::ANALOG;
    }
}
