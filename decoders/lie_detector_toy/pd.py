##
## Copyright (C) 2024 Michal Matias <mich4l.matias@gmail.com>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, see <http://www.gnu.org/licenses/>.
##


import sigrokdecode as srd
from .on_change_annotator import OnChangeAnnotator

START_MIN_LENGTH_US = 3000
LONG_MIN_LENGTH_US  = 1000
SHORT_MIN_LENGTH_US = 500

BITS_IN_PACKET = 12


# HYPOTHESIS:
# BIT: 00 01 02 03 04 05 06 07 08 09 10 11
#      \_______________/ |  |  |  \______/> LLL 0, LLT 1, LTL 2, ...: state of the lie-o meter - big LEDs
#                     |  |  |  |__________> ??? guessing, this is reserve
#                     |  |  |_____________> LED9 - L 0, T 1
#                     |  |________________> LED8 - L 0, T 1
#                     |___________________> ??? - probably state, setup info known: TLTLLL:ON,
#                                                                                   LLTTLL:ON->VIB,
#                                                                                   TLLLLL:CALIB,
#                                                                                   TLLTLL:CALIB_FINISHED,
#                                                                                   LLLTLL:MEASURE,
#                                                                                   LLLLLL:MEASURE_FIN_OK,
#                                                                                   LTLLLL:BEEPING,
#                                                                                   LTTLLL:SHOCK,
# KNOWN_STATES = {
#         'TLTLLLLLLLLL': ['ON : TLTLLLLLLLLL', 'ON'],
#         'LLTTLLLLLLLL': ['ON->VIB : LLTTLLLLLLLL', 'ON->VIB'],
#         'TLLLLLLLLLLL': ['CALIB MAIN SHOCK 0 : TLLLLLLLLLLL', 'CMS0'],
#         'TLLLLLLLLLLT': ['CALIB MAIN SHOCK 1 : TLLLLLLLLLLT', 'CMS1'],
#         'TLLLLLLLLLTL': ['CALIB MAIN SHOCK 2 : TLLLLLLLLLTL', 'CMS2'],
#         'TLLLLLLLLLTT': ['CALIB MAIN SHOCK 3 : TLLLLLLLLLTT', 'CMS3'],
#         'TLLLLLLLLTLL': ['CALIB MAIN SHOCK 4 : TLLLLLLLLTLL', 'CMS4'],
#         'TLLLLLLLLTLL': ['CALIB MAIN SHOCK 4 : TLLLLLLLLTLL', 'CMS4'],
#         'TLLTLLLLLTLT': ['CALIB SHOCK FINISHED : TLLTLLLLLTLT', 'CSF'],

#         'LLLTLLLLLLLL': ['MEASURE MAIN SHOCK : LLLTLLLLLLLL', 'MMS'],
#         'LLLLLTLTLLLT': ['MEASURE SHOCK FIN OK : LLLLLTLTLLLT', 'MSFOK'],

#         'LTLLLLLLLLLL': ['NEAR BEEPING : LTLLLLLLLLLL', 'NB'],

#         'LTTLLLLLLTLT': ['MEASURE SHOCK FIN SHOCK : LTTLLLLLLTLT', 'MSFSHCK'],
# }

START, LONG, TINY, PACKET, CONT_PACKET, STATE, LED_8_ON, LED_8_OFF, LED_9_ON, LED_9_OFF, LED_STRIPE, WRONG_N_BITS = range(12)

KNOWN_STATES = {
    'TLTLLL': 'ON',                # short beep sound
    'LLTTLL': 'ON->VIB',           # vibration sound
    'LLTLLL': 'ON->VIB->SHOCK',    # electricity sound
    'TLLLLL': 'CALIB',             # calib mutter
    'TLLTLL': 'CALIB_FINISHED',    # calib finished sound
    'LLLTLL': 'MEASURE',           # dramatic sounds
    'LLLLLL': 'MEASURE_FIN_OK',    # short beep sound
    'LTTLLL': 'MEASURE_FIN_SHOCK', # scream sound
    'LTTTLL': 'MEASURE_FIN_VIB',   # scream sound
    'LTLLLL': 'BEEP',              # loud alarm sound
}

CONST_ANNS = {
    'START'       : [START, ['START', 'S']],
    'LONG'        : [LONG, ['LONG', 'L']],
    'TINY'        : [TINY, ['TINY', 'T']],
    'WRONG_N_BITS': [WRONG_N_BITS, ['Unexpected number of bits', 'n_bits mismatch']],
}


def annotate_packet_helper(packet: str) -> list:
    if len(packet) != BITS_IN_PACKET:
        return ['n_bits mismatch']

    state, led_8, led_9, reserved, led_stripe = decompose_packet(packet)
    led_8 = 'ON' if led_8 == 'T' else 'OFF'
    led_9 = 'ON' if led_9 == 'T' else 'OFF'
    # convert to binary string, then to decimal, then to string:
    led_stripe = str(int(led_stripe.replace('T', '1').replace('L', '0'), 2))
    if state in KNOWN_STATES:
        state = KNOWN_STATES[state]
    return [f'{state} : L8 {led_8} : L9 {led_9} : LED_STRP {led_stripe} : R {reserved}']

def annotate_packet(packet: str) -> list:
    return [PACKET, annotate_packet_helper(packet)]

def annotate_cont_packet(packet: str) -> list:
    return [CONT_PACKET, annotate_packet_helper(packet)]

def annotate_state(state: str) -> list:
    if state in KNOWN_STATES:
        return [STATE, [KNOWN_STATES[state]]]
    else:
        return [STATE, [state]]

def annotate_leds(leds: str) -> list:
    source_led, state = leds
    if source_led == 'led_8':
        return [LED_8_ON, ['ON']] if state == 'T' else [LED_8_OFF, ['off']]
    if source_led == 'led_9':
        return [LED_9_ON, ['ON']] if state == 'T' else [LED_9_OFF, ['off']]
    if source_led == 'led_stripe':
        led_stripe = str(int(state.replace('T', '1').replace('L', '0'), 2))
        return [LED_STRIPE, [led_stripe]]
    raise Exception('invalid source_led')

def decompose_packet(packet: str) -> tuple:
    #       state        led 8      led 9      reserve    led stripe
    return (packet[0:6], packet[6], packet[7], packet[8], packet[9:])


class Decoder(srd.Decoder):
    api_version = 3
    id = 'lie_detector_toy'
    name = 'Lie detector toy'
    longname = 'Lie detector toy from aliexpress'
    desc = 'Internal protocol of lie detector toy from aliexpress'
    license = 'gplv2+'
    inputs = ['timing']
    outputs = []
    tags = ['toy']
    options = ()
    annotations = (
        ('start', 'Start of the packet'),
        ('long', 'Long bit'),
        ('tiny', 'Tiny bit'),
        ('packet', 'Packet'),
        ('cont_packet', 'Continous packet'),
        ('state', 'State'),  # TODO: enumerate states
        ('led_8_on', 'LED 8 is ON'),
        ('led_8_off', 'LED 8 is OFF'),
        ('led_9_on', 'LED 9 is ON'),
        ('led_9_off', 'LED 9 is OFF'),
        ('led_stripe', 'LED stripe status'),
        ('wrong_n_bits', 'Number of bits from start to start is not 12'),
    )
    annotation_rows = (
        ('shapes', 'Shapes', (START, LONG, TINY)),
        ('packet', 'Packet', (PACKET,)),
        ('cont_packet', 'Continuous packet', (CONT_PACKET,)),
        ('state', 'State', (STATE,)),
        ('led_8', 'LED 8', (LED_8_ON, LED_8_OFF)),
        ('led_9', 'LED 9', (LED_9_ON, LED_9_OFF)),
        ('led_stripe', 'LED stripe', (LED_STRIPE,)),
        ('errors', 'Errors', (WRONG_N_BITS,)),
    )
    binary = ()

    def __init__(self):
        self.reset()

    def reset(self):
        self.is_first_packet = True
        self.packet = ''
        self.n_bits = 0
        self.last_start_sample = 0

        # on change annotators:
        self.cont_packet_oca = None
        self.state_oca       = None
        self.led_8_oca       = None
        self.led_9_oca       = None
        self.led_stripe_oca  = None

    def metadata(self, key, value):
        if key == srd.SRD_CONF_SAMPLERATE:
            self.samplerate = value

    def start(self):
        self.out_ann = self.register(srd.OUTPUT_ANN)
        # init on change annotators
        self.cont_packet_oca = OnChangeAnnotator(self.put, self.out_ann, annotate_cont_packet)
        self.state_oca       = OnChangeAnnotator(self.put, self.out_ann, annotate_state)
        self.led_8_oca       = OnChangeAnnotator(self.put, self.out_ann, annotate_leds)
        self.led_9_oca       = OnChangeAnnotator(self.put, self.out_ann, annotate_leds)
        self.led_stripe_oca  = OnChangeAnnotator(self.put, self.out_ann, annotate_leds)
        self.ocas = (self.cont_packet_oca, self.state_oca, self.led_8_oca, self.led_9_oca, self.led_stripe_oca)

    def decode(self, ss, es, data):
        time_us, value, _ = data
        if value == 0:  # only interested in high pulses
            return
        if time_us >= START_MIN_LENGTH_US:  # start condition
            self.put(ss, es, self.out_ann, CONST_ANNS['START'])
            if self.is_first_packet:  # do the init for the upcoming packet
                self.init_packet(ss)
                self.last_cont_packet_start_sample = ss
                self.is_first_packet = False
                # init OCAs:
                for oca in self.ocas:
                    oca.start(ss)
                return

            # if already have data from last packet, now it is time to process them
            if self.n_bits != BITS_IN_PACKET: # throw away if the packet does not make sense
                self.put(self.last_start_sample, ss, self.out_ann, CONST_ANNS['WRONG_N_BITS'])
                self.init_packet(ss)
                for oca in self.ocas:
                    oca.disrupt(ss)
                return

            self.put(self.last_start_sample, ss, self.out_ann, annotate_packet(self.packet))
            # decompose packet and put him apropriately into OCAs:
            state, led_8, led_9, reserve, led_stripe  = decompose_packet(self.packet)
            self.cont_packet_oca.handle_state(self.packet, ss)
            self.state_oca.handle_state(state, ss)
            self.led_8_oca.handle_state(('led_8', led_8), ss)
            self.led_9_oca.handle_state(('led_9', led_9), ss)
            self.led_stripe_oca.handle_state(('led_stripe', led_stripe), ss)

            # # get ready for another packet
            self.init_packet(ss)

        elif time_us >= LONG_MIN_LENGTH_US:
            self.put(ss, es, self.out_ann, CONST_ANNS['LONG'])
            self.n_bits += 1;
            self.packet += 'L'
        elif time_us >= SHORT_MIN_LENGTH_US:
            self.put(ss, es, self.out_ann, CONST_ANNS['TINY'])
            self.n_bits += 1;
            self.packet += 'T'

    def init_packet(self, start_sample):
        self.n_bits = 0
        self.last_start_sample = start_sample
        self.packet = ''
