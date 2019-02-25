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

SSD1306Display::SSD1306Display() {
  Reset();
}

void SSD1306Display::Reset() {
  SetOn(false);

  // Init stuff.
  const uint8_t h = 64;
  const bool external_vcc = false;

  static const uint8_t PROGMEM init_sequence[] = {
    REG_MEM_ADDR, 0x00,
    REG_DISP_START_LINE,
    REG_SEG_REMAP | 0x01,  // column addr 127 mapped to SEG0
    REG_COM_OUT_DIR | 0x08, // scan from COM[N] to COM0
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
  i2c_.StartTransmission(SSD1306_I2C_ADDRESS);
  i2c_.Write(COMMAND_TRANSFER);
  for (uint8_t i = 0; i < sizeof(init_sequence); ++i) {
    i2c_.Write(pgm_read_byte(&init_sequence[i]));
  }
  i2c_.FinishTransmission();

  ClearScreen();
  SetOn(true);
}

void SSD1306Display::SetOn(bool on) {
  i2c_.StartTransmission(SSD1306_I2C_ADDRESS);
  i2c_.Write(COMMAND_TRANSFER);
  i2c_.Write(REG_DISP | on);
  i2c_.FinishTransmission();
}

void SSD1306Display::ClearScreen() {
  const uint8_t cmd[] = {
    REG_COL_ADDR, 0, 127,
    REG_PAGE_ADDR, 0, 0xff,
  };

  i2c_.StartTransmission(SSD1306_I2C_ADDRESS);
  i2c_.Write(COMMAND_TRANSFER);
  for (uint8_t i = 0; i < sizeof(cmd); ++i)
    i2c_.Write(cmd[i]);
  i2c_.FinishTransmission();

  i2c_.StartTransmission(SSD1306_I2C_ADDRESS);
  i2c_.Write(DATA_TRANSFER);
  for (uint16_t i = 0; i < 128 * 8; ++i)
    i2c_.Write(0x00);
  i2c_.FinishTransmission();
}

void SSD1306Display::DrawPageFromProgmem(uint8_t page,
                                         uint8_t from_x, uint8_t to_x,
                                         const uint8_t *progmem_buffer) {
  const uint8_t cmd[] = {
    REG_COL_ADDR, from_x, uint8_t(to_x - 1),
    REG_PAGE_ADDR, page, page,
  };

  // Set command for the following data
  i2c_.StartTransmission(SSD1306_I2C_ADDRESS);
  i2c_.Write(COMMAND_TRANSFER);
  for (uint8_t i = 0; i < sizeof(cmd); ++i)
    i2c_.Write(cmd[i]);
  i2c_.FinishTransmission();

  // Send data.
  uint8_t count = to_x - from_x;
  i2c_.StartTransmission(SSD1306_I2C_ADDRESS);
  i2c_.Write(DATA_TRANSFER);
  while (count--)
    i2c_.Write(pgm_read_byte(progmem_buffer++));
  i2c_.FinishTransmission();
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
