// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2019 Henner Zeller <h.zeller@acm.org>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://gnu.org/licenses/gpl-2.0.txt>
#ifndef SSD1306_FONT_SUPPORT_
#define SSD1306_FONT_SUPPORT_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct MetaGlyph {
    int16_t codepoint;     // Unicode 16 code-point.
    int8_t width;          // Individual width of this one.
} __attribute__((packed));

struct MetaFont {
    uint16_t available_glyphs; // Number of glyphs in this font.
    uint8_t pages;             // height in 'pages', essentially 8 bit stripes
    uint8_t font_width;        // Widest glyph in this font. Determines bytes.
    uint8_t glyph_data_size;   // Glyph size (sizeof(ProgmemGlyph<W,H>))
} __attribute__((packed));

#ifdef __cplusplus
}
#endif

#endif // SSD1306_FONT_SUPPORT_
