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

#include "sh1106-display.h"

#include <avr/pgmspace.h>
#include <stdlib.h>

#include "utf8-iterator.h"

// SH1106 Datasheet pp19...31                          Command Table Entry
// Configuration options.
static constexpr uint8_t REG_DISP_START_LINE = 0x40;  // 4.   // memory layout
static constexpr uint8_t REG_DISP_OFFSET     = 0xd3;  // 14.

static constexpr uint8_t REG_SEG_REMAP       = 0xa0;  // 6.   // display flip
static constexpr uint8_t REG_COM_OUT_DIR     = 0xc0;  // 13.

static constexpr uint8_t REG_CHARGE_PUMP     = 0x8d;  // 10.  // hardware opts
static constexpr uint8_t REG_DISP_CLK_DIV    = 0xd5;  // 15.
static constexpr uint8_t REG_PRECHARGE       = 0xd9;  // 16.
static constexpr uint8_t REG_COMMON_PADS_CFG = 0xda;  // 17.
static constexpr uint8_t REG_VCOM_DESEL      = 0xdb;  // 18.

static constexpr uint8_t REG_CONTRAST        = 0x81;  // 5.   // Visuals
static constexpr uint8_t REG_ENTIRE_ON       = 0xa4;  // 7.
static constexpr uint8_t REG_NORM_INV        = 0xa6;  // 8.
static constexpr uint8_t REG_DISP_ONOFF      = 0xae;  // 11.

static constexpr uint8_t REG_COLUMN_LOWER    = 0x00;  // 1.  // Updating
static constexpr uint8_t REG_COLUMN_UPPER    = 0x10;  // 2.
static constexpr uint8_t REG_PAGE_ADDRESS    = 0xB0;  // 12.

// I2C command/data switch command
static constexpr uint8_t COMMAND_TRANSFER = 0x00;
static constexpr uint8_t DATA_TRANSFER    = 0x40;

#ifdef DISPLAY_I2C
static constexpr uint8_t SH1106_I2C_ADDRESS = DISPLAY_I2C;
#else
static constexpr uint8_t SH1106_I2C_ADDRESS = 0x78;
#endif

static constexpr bool upside_down = true;     // Rotate by 180 degrees.
static constexpr bool external_vcc = false;

#ifdef DISP_SSD1306
static constexpr uint8_t x_offset = 0;
#else
static constexpr uint8_t x_offset = 2;   // 128px wide, but 2pixel border
#endif

SH1106Display::SH1106Display() {
  I2CMaster::Init();
  Reset();
}

static void StartCommandTransmission() {
  I2CMaster::StartTransmission(SH1106_I2C_ADDRESS);
  I2CMaster::Write(COMMAND_TRANSFER);
}

void SH1106Display::Reset() {
  SetOn(false);

  // Init stuff.
  constexpr uint8_t h = 64;
  static constexpr uint8_t PROGMEM init_sequence[] = {
    REG_DISP_START_LINE,
    REG_DISP_OFFSET, 0x00,
    REG_SEG_REMAP | (upside_down ? 0x01 : 0x00),
    REG_COM_OUT_DIR | (upside_down ? 0x08 : 0x00),
    REG_COMMON_PADS_CFG, h == 32 ? 0x02 : 0x12,
    REG_DISP_CLK_DIV, 0x80,
    REG_PRECHARGE, external_vcc ? 0x22 : 0xf1,
    REG_VCOM_DESEL, 0x30,    // 0.83*Vcc
    REG_CONTRAST, 0xff,      // maximum
    REG_ENTIRE_ON,           // output follows RAM contents
    REG_NORM_INV,            // not inverted
    REG_CHARGE_PUMP, external_vcc ? 0x10 : 0x14,
  };
  StartCommandTransmission();
  for (uint8_t i = 0; i < sizeof(init_sequence); ++i) {
    I2CMaster::Write(pgm_read_byte(&init_sequence[i]));
  }
  I2CMaster::FinishTransmission();

  ClearScreen();
  SetOn(true);
}

void SH1106Display::SetOn(bool on) {
  StartCommandTransmission();
  I2CMaster::Write(REG_DISP_ONOFF | on);
  I2CMaster::FinishTransmission();
}

static void StartPageTransmission(uint8_t x, uint8_t y_page) {
  StartCommandTransmission();
  const uint8_t start_pos = x + x_offset;
  I2CMaster::Write(REG_PAGE_ADDRESS | y_page);              // row address
  I2CMaster::Write(REG_COLUMN_LOWER | (start_pos & 0x0f));  // column nibbles
  I2CMaster::Write(REG_COLUMN_UPPER | (start_pos >> 4));
  I2CMaster::FinishTransmission();

  // Open data transmission, to be finished by the caller.
  I2CMaster::StartTransmission(SH1106_I2C_ADDRESS);
  I2CMaster::Write(DATA_TRANSFER);
}

void SH1106Display::ClearScreen() {
  for (uint8_t y = 0; y < 8; ++y) {
    StartPageTransmission(0, y);
    for (uint8_t i = 0; i < 128; ++i)
      I2CMaster::Write(0x00);
    I2CMaster::FinishTransmission();
  }
}

uint8_t SH1106Display::Print(const FontData &font,
                             uint8_t xpos, const uint8_t ypos,
                             const char *utf8_text,
                             bool inverse) {
  const uint8_t y_page = ypos/8;
  while (*utf8_text) {
    const uint16_t cp = utf8_next_codepoint(utf8_text);
    xpos += BDFONT_EMIT_GLYPH(&font, cp, true,
                              { StartPageTransmission(xpos, y_page + stripe); },
                              { I2CMaster::Write(inverse ? ~b : b); },
                              { I2CMaster::FinishTransmission(); });
  }
  return xpos;
}

void SH1106Display::FillStripeRange(uint8_t x_from, uint8_t x_to, uint8_t ypos,
                                     uint8_t fill_mask) {
  StartPageTransmission(x_from, ypos / 8);
  for (uint8_t x = x_from; x < x_to; ++x)
    I2CMaster::Write(fill_mask);
  I2CMaster::FinishTransmission();
}
