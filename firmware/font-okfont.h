// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// This is autogenerated.
#ifndef FONT_okfont_
#define FONT_okfont_

#include "font-support.h"
#include <avr/pgmspace.h>

struct ProgmemFont_okfont_t {
  struct MetaFont meta;
  struct {
    struct MetaGlyph meta;
    uint8_t data[5][35];  // page-stripes, width
  } glyphs[2];
} __attribute__((packed));

extern const struct ProgmemFont_okfont_t PROGMEM progmem_font_okfont;
#define font_okfont (&progmem_font_okfont.meta)

#endif // FONT_okfont_
