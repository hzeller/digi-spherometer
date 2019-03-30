/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*-
 * This is part of http://github.com/hzeller/bdfont.data
 *
 * Copyright (C) 2019 Henner Zeller <h.zeller@acm.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bdfont-support.h"

#include <stdlib.h>

static int glyph_compare(const void *key, const void *element) {
#ifdef __AVR__
  /* Glyph pointer is pointing to PROGMEM. First two bytes are codepoint */
  int codepoint = pgm_read_word(element);
#else
  int codepoint = ((const struct GlyphData*)element)->codepoint;
#endif
  int search_codepoint = *(const uint16_t*)(key);
  return search_codepoint - codepoint;
}

const struct GlyphData *bdfont_find_glyph(const struct FontData *font,
                                          int16_t codepoint) {
  return (const struct GlyphData*)
    (bsearch(&codepoint, font->glyphs, font->available_glyphs,
             sizeof(struct GlyphData), glyph_compare));
}

uint8_t bdfont_emit_glyph(const struct FontData *font, uint16_t codepoint,
                          bdf_StartStripe start_stripe, bdf_EmitFun emit,
                          void *userdata) {
  return BDFONT_EMIT_GLYPH(font, codepoint, 1,
                           { start_stripe(stripe, glyph_width, userdata); },
                           { emit(x, b, userdata); },
                           {});
}
