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

#include "tiny-i2c-master.h"

#include <util/delay.h>

// The delay _should_ be more, but looks like we can drive the SSD1306/SH1106
// a lot faster. Which is good (even a delay of 0 seems to work fine, but
// let's not push it too far :)).
static constexpr uint8_t DELAY_2TWI = 0;  // > 4.5μs
static constexpr uint8_t DELAY_4TWI = 0;  // > 4μs

void I2CMaster::Init() {
  Enable(true);

  USIDR = 0xFF;
  USICR = (0<<USISIE) | (0<<USIOIE)      // no interrupts
    | (1<<USIWM1) | (0<<USIWM0)        // two wire USI
    | (1<<USICS1) | (0<<USICS0) | (1<<USICLK)  // software clock source
    | (0<<USITC);

  // Clear flags and counter.
  USISR = (1<<USISIF)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)
    | (0x00<<USICNT0);
}

void I2CMaster::Enable(bool en) {
  if (en) {
    USI_PORT |= (1<<USI_PORT_SDA) | (1<<USI_PORT_SCL);  // pullups on.
    USI_DDR |= (1<<USI_PORT_SDA) | (1<<USI_PORT_SCL);  // set output
  } else {
    USI_PORT &= ~((1<<USI_PORT_SDA) | (1<<USI_PORT_SCL));  // pullups off.
    USI_DDR &= ~((1<<USI_PORT_SDA) | (1<<USI_PORT_SCL));
  }
}

void I2CMaster::StartTransmission(uint8_t address) {
  USI_PORT |= (1<<USI_PORT_SCL);           // SCL up ...
  while (!(USI_PIN & (1<<USI_PORT_SCL)))  // ... wait until it actually is.
    ;
  _delay_us(DELAY_4TWI);  // fast mode.

  USI_PORT &= ~(1<<USI_PORT_SDA);  // Pull SDA low
  _delay_us(DELAY_4TWI);
  USI_PORT &= ~(1<<USI_PORT_SCL);  // Pull SCL low
  USI_PORT |= (1<<USI_PORT_SDA);   // SDA up again.

  Write(address);
}

void I2CMaster::FinishTransmission() {
  USI_PORT &= ~(1<<USI_PORT_SDA);           // SDA low.
  USI_PORT |= (1<<USI_PORT_SCL);           // SCL up ...
  while (!(USI_PIN & (1<<USI_PORT_SCL)))  // ... wait until it actually is.
    ;
  _delay_us(DELAY_4TWI);

  USI_PORT |= (1<<USI_PORT_SDA);   // SDA up
  _delay_us(DELAY_2TWI);
}

void I2CMaster::Write(uint8_t b) {
  constexpr uint8_t USISR_8bit
    = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC)
    | (0x0 << USICNT0);  // 16 clock edges
  constexpr uint8_t USISR_1bit
    = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC)
    | (0xE << USICNT0); // set USI to shift 1 bit i.e. count 2 clock edges.

  USI_PORT &= ~(1 << USI_PORT_SCL);  // Start with SCL low
  Transfer(USISR_8bit, b);

  // Read back ACK/NACK (but ignore).
  USI_DDR &= ~(1 << USI_PORT_SDA); // Enable SDA as input.
  Transfer(USISR_1bit, 0x00);
}

uint8_t I2CMaster::Transfer(uint8_t mode, uint8_t b) {
  USIDR = b;
  USISR = mode;
  constexpr uint8_t clk_icr = (0 << USISIE) | (0 << USIOIE) // !use interrupts
    | (1 << USIWM1) | (0 << USIWM0)                   // two wire mode
    | (1 << USICS1) | (0 << USICS0) | (1 << USICLK)   // software clock
    | (1 << USITC);                                   // clock toggle.

  do {
    _delay_us(DELAY_2TWI);
    USICR = clk_icr;   // Toggle SCL up.
    while (!(USI_PIN & (1<<USI_PORT_SCL)))  // wait until SCL is high.
      ;
    _delay_us(DELAY_4TWI);
    USICR = clk_icr;   // Toggle SCL down.
  } while (!(USISR & (1<<USIOIF)));   // Until all bits are out.

  _delay_us(DELAY_2TWI);
  const uint8_t received = USIDR;
  USIDR = 0xFF; // release SDA
  USI_DDR |= (1 << USI_PORT_SDA); // .. back to SDA as output.
  return received;
}
