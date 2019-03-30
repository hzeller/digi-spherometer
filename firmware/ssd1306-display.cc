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

#include "ssd1306-display.h"

#include <avr/pgmspace.h>
#include <stdlib.h>

#include "utf8-iterator.h"

static constexpr uint8_t REG_CONTRAST        = 0x81;
static constexpr uint8_t REG_ENTIRE_ON       = 0xa4;
static constexpr uint8_t REG_NORM_INV        = 0xa6;
static constexpr uint8_t REG_DISP            = 0xae;
static constexpr uint8_t REG_MEM_ADDR        = 0x20;
static constexpr uint8_t REG_COL_ADDR        = 0x21;
static constexpr uint8_t REG_PAGE_ADDR       = 0x22;
static constexpr uint8_t REG_DISP_START_LINE = 0x40;
static constexpr uint8_t REG_SEG_REMAP       = 0xa0;
static constexpr uint8_t REG_COM_OUT_DIR     = 0xc0;
static constexpr uint8_t REG_DISP_OFFSET     = 0xd3;
static constexpr uint8_t REG_COM_PIN_CFG     = 0xda;
static constexpr uint8_t REG_DISP_CLK_DIV    = 0xd5;
static constexpr uint8_t REG_PRECHARGE       = 0xd9;
static constexpr uint8_t REG_VCOM_DESEL      = 0xdb;
static constexpr uint8_t REG_CHARGE_PUMP     = 0x8d;

static constexpr uint8_t COMMAND_TRANSFER = 0x00;
static constexpr uint8_t DATA_TRANSFER    = 0x40;

static constexpr uint8_t SSD1306_I2C_ADDRESS = 0x78;

static constexpr bool upside_down = false;  // Rotate by 180 degrees.
static constexpr bool external_vcc = false;

#define DISP_SH1106

SSD1306Display::SSD1306Display() {
  I2CMaster::Init();
  Reset();
}

void SSD1306Display::Reset() {
  SetOn(false);

  // Init stuff.
  const uint8_t h = 64;
  static const uint8_t PROGMEM init_sequence[] = {
#ifndef DISP_SH1106
    REG_MEM_ADDR, 0x00,
#endif
    REG_DISP_START_LINE,
#ifndef DISP_SH1106
    REG_SEG_REMAP | (upside_down ? 0x01 : 0x00),
    REG_COM_OUT_DIR | (upside_down ? 0x08 : 0x00),
#endif
    REG_DISP_OFFSET, 0x00,
    REG_COM_PIN_CFG, h == 32 ? 0x02 : 0x12,
    REG_DISP_CLK_DIV, 0x80,
    REG_PRECHARGE, external_vcc ? 0x22 : 0xf1,
    REG_VCOM_DESEL, 0x30,    // 0.83*Vcc
    REG_CONTRAST, 0xff,      // maximum
    REG_ENTIRE_ON,           // output follows RAM contents
    REG_NORM_INV,            // not inverted
    REG_CHARGE_PUMP, external_vcc ? 0x10 : 0x14,
  };
  I2CMaster::StartTransmission(SSD1306_I2C_ADDRESS);
  I2CMaster::Write(COMMAND_TRANSFER);
  for (uint8_t i = 0; i < sizeof(init_sequence); ++i) {
    I2CMaster::Write(pgm_read_byte(&init_sequence[i]));
  }
  I2CMaster::FinishTransmission();

  ClearScreen();
  SetOn(true);
}

void SSD1306Display::SetOn(bool on) {
  I2CMaster::StartTransmission(SSD1306_I2C_ADDRESS);
  I2CMaster::Write(COMMAND_TRANSFER);
  I2CMaster::Write(REG_DISP | on);
  I2CMaster::FinishTransmission();
}

void SSD1306Display::ClearScreen() {
#ifdef DISP_SH1106
  for (uint8_t y = 0; y < 8; ++y) {
    I2CMaster::StartTransmission(SSD1306_I2C_ADDRESS);
    I2CMaster::Write(COMMAND_TRANSFER);
    I2CMaster::Write(0xB0 | y);  // row address
    I2CMaster::Write(0x00 | 2);  // col: 00
    I2CMaster::Write(0x10 | 0);
    I2CMaster::FinishTransmission();

    I2CMaster::StartTransmission(SSD1306_I2C_ADDRESS);
    I2CMaster::Write(DATA_TRANSFER);
    for (uint8_t i = 0; i < 128; ++i)
      I2CMaster::Write(0x00);
    I2CMaster::FinishTransmission();
  }
#else
  const uint8_t cmd[] = {
    REG_COL_ADDR, 0, 127,
    REG_PAGE_ADDR, 0, 0xff,
  };

  I2CMaster::StartTransmission(SSD1306_I2C_ADDRESS);
  I2CMaster::Write(COMMAND_TRANSFER);
  for (uint8_t i = 0; i < sizeof(cmd); ++i)
    I2CMaster::Write(cmd[i]);
  I2CMaster::FinishTransmission();

  I2CMaster::StartTransmission(SSD1306_I2C_ADDRESS);
  I2CMaster::Write(DATA_TRANSFER);
  for (uint16_t i = 0; i < 128 * 8; ++i)
    I2CMaster::Write(0x00);
  I2CMaster::FinishTransmission();
#endif
}

void SSD1306Display::DrawPageFromProgmem(uint8_t page,
                                         uint8_t from_x, uint8_t to_x,
                                         const uint8_t *progmem_buffer) {
  I2CMaster::StartTransmission(SSD1306_I2C_ADDRESS);
  I2CMaster::Write(COMMAND_TRANSFER);

#ifdef DISP_SH1106
  const uint8_t start_pos = from_x + 2;
  I2CMaster::Write(COMMAND_TRANSFER);
  I2CMaster::Write(0xB0 | page);  // row address
  I2CMaster::Write(0x00 | (start_pos & 0x0f));
  I2CMaster::Write(0x10 | (start_pos >> 4));
#else
  const uint8_t cmd[] = {
    REG_COL_ADDR, from_x, uint8_t(to_x - 1),
    REG_PAGE_ADDR, page, page,
  };

  // Set command for the following data
  for (uint8_t i = 0; i < sizeof(cmd); ++i)
    I2CMaster::Write(cmd[i]);
#endif
  I2CMaster::FinishTransmission();

  // Send data.
  uint8_t count = to_x - from_x;
  I2CMaster::StartTransmission(SSD1306_I2C_ADDRESS);
  I2CMaster::Write(DATA_TRANSFER);
  while (count--)
    I2CMaster::Write(pgm_read_byte(progmem_buffer++));
  I2CMaster::FinishTransmission();
}

static int compare(const void *key, const void *element) {
  int codepoint = pgm_read_word(element);
  int search_codepoint = *static_cast<const uint16_t*>(key);
  return search_codepoint - codepoint;
}

static const MetaGlyph* FindProgmemGlyph(uint16_t codepoint,
                                         const MetaFont &meta,
                                         const MetaFont *progmem_font) {
  const uint8_t *glyph_data = (const uint8_t*)progmem_font + sizeof(MetaFont);
  return static_cast<const MetaGlyph*>(
    bsearch(&codepoint, glyph_data, meta.available_glyphs, meta.glyph_data_size,
            compare));
}

uint8_t SSD1306Display::Print(const MetaFont *progmem_font,
                              uint8_t xpos, uint8_t ypos,
                              const char *utf8_text) {
  uint8_t ypage = ypos / 8;  // TODO: maybe do some shifting for in-between
  MetaFont unpacked_font;
  memcpy_P(&unpacked_font, progmem_font, sizeof(unpacked_font));
  while (*utf8_text) {
    const uint32_t cp = utf8_next_codepoint(utf8_text);
    const MetaGlyph *progmem_glyph = FindProgmemGlyph(cp, unpacked_font,
                                                      progmem_font);
    if (progmem_glyph == nullptr) continue;
    MetaGlyph unpacked_glyph;
    memcpy_P(&unpacked_glyph, progmem_glyph, sizeof(unpacked_glyph));
    for (uint8_t y = 0; y < unpacked_font.pages; ++y) {
      const uint8_t *data = ((const uint8_t*)(progmem_glyph) + sizeof(MetaGlyph)
                             + y * unpacked_font.font_width);
      DrawPageFromProgmem(ypage + y, xpos, xpos + unpacked_glyph.width, data);
    }
    xpos += unpacked_glyph.width;
  }
  return xpos;
}

void SSD1306Display::FillStripeRange(uint8_t x_from, uint8_t x_to, uint8_t ypos,
                                     uint8_t fill_mask) {
  uint8_t page = ypos / 8;
  I2CMaster::StartTransmission(SSD1306_I2C_ADDRESS);
  I2CMaster::Write(COMMAND_TRANSFER);

#ifdef DISP_SH1106
  const uint8_t start_pos = x_from + 2;
  I2CMaster::Write(COMMAND_TRANSFER);
  I2CMaster::Write(0xB0 | page);  // row address
  I2CMaster::Write(0x00 | (start_pos & 0x0f));
  I2CMaster::Write(0x10 | (start_pos >> 4));
#else
  const uint8_t cmd[] = {
    REG_COL_ADDR, x_from, uint8_t(x_to-1),
    REG_PAGE_ADDR, page, page,
  };

  for (uint8_t i = 0; i < sizeof(cmd); ++i)
    I2CMaster::Write(cmd[i]);
#endif
  I2CMaster::FinishTransmission();

  I2CMaster::StartTransmission(SSD1306_I2C_ADDRESS);
  I2CMaster::Write(DATA_TRANSFER);
  for (uint8_t x = x_from; x < x_to; ++x)
    I2CMaster::Write(fill_mask);
  I2CMaster::FinishTransmission();
}
