// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
//
// Copyright (C) 2026 Henner Zeller <h.zeller@acm.org>
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

// This is an implementation of the minimal i2c interface using the TWI0
// peripheral found in modern tinyAVR 0/1/2-series and megaAVR 0-series
// microcontrollers (such as the ATtiny1604).

#include "i2c-master.h"

#include <avr/io.h>

// Default TWI0 pins on ATtiny1604: PB0 = SCL, PB1 = SDA
static constexpr uint8_t SCL_BM = PIN0_bm;
static constexpr uint8_t SDA_BM = PIN1_bm;

static constexpr uint32_t TX_SPEED = 400000;
static constexpr uint8_t MBAUD_val = ((F_CPU / TX_SPEED) - 10) / 2;

void I2CMaster::Init() {
  TWI0.MBAUD = MBAUD_val;
  Enable(true);
}

void I2CMaster::Enable(bool en) {
  if (en) {
    PORTB.PIN0CTRL |= PORT_PULLUPEN_bm;
    PORTB.PIN1CTRL |= PORT_PULLUPEN_bm;
    TWI0.MCTRLA = TWI_ENABLE_bm;
    TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;
  } else {
    TWI0.MCTRLA = 0;
    PORTB.PIN0CTRL &= ~PORT_PULLUPEN_bm;
    PORTB.PIN1CTRL &= ~PORT_PULLUPEN_bm;
    PORTB.DIRCLR = SCL_BM | SDA_BM;
    PORTB.OUTCLR = SCL_BM | SDA_BM;
  }
}

void I2CMaster::StartTransmission(uint8_t address) {
  TWI0.MADDR = address;
  while (!(TWI0.MSTATUS & (TWI_WIF_bm | TWI_RIF_bm)))
    ;
}

void I2CMaster::Write(uint8_t b) {
  TWI0.MDATA = b;
  while (!(TWI0.MSTATUS & TWI_WIF_bm))
    ;
}

void I2CMaster::FinishTransmission() {
  TWI0.MCTRLB = TWI_MCMD_STOP_gc;
}
