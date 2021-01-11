// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
//
// Copyright (C) 2018 Henner Zeller <h.zeller@acm.org>
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

// This is an implementation of the minimal i2c interface using USI, which
// is provided in some Atmel microcontrollers.

#include "i2c-master.h"

#include <util/twi.h>
#include <avr/io.h>

// These are for ATMega328p (Arduino Nano)
#define I2C_DDR       DDRC
#define I2C_PORT      PORTC
#define I2C_PORT_SDA  4
#define I2C_PORT_SCL  5

static constexpr uint32_t TX_SPEED = 400000; // Could be faster with higher cpu
static constexpr uint8_t PRESCALER = 1;
static constexpr uint8_t TWBR_val = (((F_CPU/TX_SPEED) / PRESCALER) - 16 ) / 2;

void I2CMaster::Init() {
  TWBR = TWBR_val;
}

void I2CMaster::Enable(bool en) {
  if (en) {
    I2C_PORT |= (1<<I2C_PORT_SDA) | (1<<I2C_PORT_SCL);  // pullups on.
    I2C_DDR |= (1<<I2C_PORT_SDA) | (1<<I2C_PORT_SCL);  // set output
  } else {
    I2C_PORT &= ~((1<<I2C_PORT_SDA) | (1<<I2C_PORT_SCL));  // pullups off.
    I2C_DDR &= ~((1<<I2C_PORT_SDA) | (1<<I2C_PORT_SCL));
  }
}

void I2CMaster::StartTransmission(uint8_t address) {
  TWCR = 0;
  TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);  // start condition
  while (!(TWCR & (1<<TWINT)))  // Wait to finish transmit
    ;

  if ((TWSR & 0xF8) != TW_START)   // Something went wrong.
    return;

  Write(address);
}

void I2CMaster::Write(uint8_t b) {
  TWDR = b;
  TWCR = (1<<TWINT) | (1<<TWEN);  // start transmission.
  while (!(TWCR & (1<<TWINT)))    // wait finish.
    ;
}

void I2CMaster::FinishTransmission() {
  TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
}
