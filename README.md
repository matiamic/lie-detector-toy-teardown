# [Lie Detector Toy](https://www.amazon.com/BRXY-Electric-Shocking-Interesting-Polygraph/dp/B07ZJ18FWG/) Teardown and Reimplementation

This repository contains results of the teardown of a lie detector toy and arduino reimplementation of it's firmware. The purpose of the reimplementation was to achieve predictability for use in a theater production.

## Overview

- **Schematic**: Detailed circuit diagram of the original toy (`schematic.pdf`), some decoupling capacitors might be omitted.
- **Sigrok Decoders**: Two custom decoders for analyzing signals. Modified timing decoder and a lie detector toy decoder, that is written to be stacked on top of the timing decoder
- **Arduino Reimplementation**: An implementation of modified firmware written for Arduino nano. Arduino replaces the original main MCU and communicates with other ICs on the board.
