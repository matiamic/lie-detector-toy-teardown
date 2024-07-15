#ifndef Packet_generator_h
#define Packet_generator_h
/* #include "Arduino.h" */

enum class State : uint16_t {
    ON                = 0xa0, // TLTL LL(LL) # short beep sound
    ON_VIB            = 0x30, // LLTT LL(LL) # vibration sound
    ON_VIB_SHOCK      = 0x20, // LLTL LL(LL) # electricity sound
    CALIB             = 0x80, // TLLL LL(LL) # calib mutter
    CALIB_FINISHED    = 0x90, // TLLT LL(LL) # calib finished sound
    MEASURE           = 0x10, // LLLT LL(LL) # dramatic sounds
    MEASURE_FIN_OK    = 0x00, // LLLL LL(LL) # short beep sound
    MEASURE_FIN_SHOCK = 0x60, // LTTL LL(LL) # scream sound
    MEASURE_FIN_VIB   = 0x70, // LTTT LL(LL) # scream sound
    BEEP              = 0x40, // LTLL LL(LL) # loud alarm sound

    // look for easter eggs: spoler: the first one is some hight pitch beep
    // the others are the same short beep sound as ON
    E1 = 0x50,
    E2 = 0xb0,
    E3 = 0xc0,
    E4 = 0xd0,
    E5 = 0xe0,
    E6 = 0xf0,
    E7 = 0xa8,
};

class PacketGenerator
{
	public:
		PacketGenerator(uint8_t pin);
                void sendPacket(State state, bool led_8, bool led_9, int led_stripe);
                void begin(void);

	private:
                int _pin;
                void _send_start(void);
                void _send_tiny(void);
                void _send_long(void);
};

#endif
