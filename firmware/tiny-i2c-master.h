// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
//
// Copyright (C) 2019 Henner Zeller <h.zeller@acm.org>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <stdint.h>
#include <avr/io.h>

// These are for Attiny{2,4,8}5.
#define USI_DDR       DDRB
#define USI_PORT      PORTB
#define USI_PIN       PINB
#define USI_PORT_SDA  0
#define USI_PORT_SCL  2

// A simple I2C master that works with ATTiny devices.
class I2CMaster {
public:
  I2CMaster();

  void Enable(bool b);
  void StartTransmission(uint8_t address);
  void Write(uint8_t b);
  void FinishTransmission();

private:
  uint8_t Transfer(uint8_t mode, uint8_t b);
};
