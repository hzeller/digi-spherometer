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

#ifndef _BDFONT_DATA_FONT_SUPPORT_
#define _BDFONT_DATA_FONT_SUPPORT_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * By default, bdfont uses runlength encoding techniques where appropriate,
 * which in particular helps with large fonts. For smaller fonts with only
 * few glyphs the code overhad to support decoding RLE (~90 bytes on AVR) can
 * sometimes be larger than the data savings, so it is possible to choose
 * emitting only plain bytes at code generation time (option -p for
 * bdfont-data-gen); then you can set -DBDFONT_USE_RLE=0 at compile time of
 * the target program.
 */
#ifndef BDFONT_USE_RLE
#  define BDFONT_USE_RLE 1
#endif

#include <stdint.h>

/* Some of the fields in these types are stable and can be used in code,
 * while everything in the 'private interface' section should be treated as
 * opaque data as they might change.
 *
 * Only use the bdfont_emit_glyph() function or BDFONT_EMIT_GLYPH() macro below
 * to extract the bytes from the font, as the encoding might change.
 */
struct GlyphData {
  /* Public interface */
  uint16_t codepoint;        /* Unicode 16 code-point. */
  uint8_t width;             /* Individual width of this one. */

  /* Private interface, might change without notice */
  uint8_t left_margin  : 4;  /* Left empty space */
  uint8_t right_margin : 4;  /* Right empty space */
  uint8_t stripe_begin : 4;  /* Empty Y-stripes skipped in data. */
  uint8_t stripe_end   : 4;  /* end filled stripes */
  uint16_t data_offset : 14; /* Pointer into bits array. */
  uint8_t rle_type     : 2;  /* 0: none; 1: 2x4-bit count; 2: 4x2-bit count */
} __attribute__((packed));

struct FontData {
  /* Public interface */
  uint16_t available_glyphs; /* Number of glyphs in this font. */
  uint8_t baseline;          /* Position of baseline from rendering top */
  uint8_t stripes;           /* height in 8px high stripes. */

  /* Private interface, might change without notice */
  const uint8_t *bits;
  const struct GlyphData *glyphs;
} __attribute__((packed));

/**
 * Find glyph for given codepoint. Returns pointer to GlyphData or NULL
 * if it does not exist.
 * Note: on AVR, this returns a pointer to PROGMEM memory.
 */
const struct GlyphData *bdfont_find_glyph(const struct FontData *font,
                                          int16_t codepoint);

/**
 * Emit the bytes for a glyph with the given basic plane unicode "codepoint"
 * Returns width that has been drawn or 0 if the character was not defined.
 *
 * This calls callbacks to two functions: one to start a new stripe, providing
 * information about which stripe and the expected width.
 * Then an EmitFun() call that emits a single byte at given x-position
 * representing 8 vertical pixels.
 *
 * Both functions get passed in some void pointer with user-data.
 *
 * Returns width of the character or 0 if it does not exist in the font.
 */
typedef void (*bdf_StartStripe)(uint8_t stripe, uint8_t width, void *userdata);
typedef void (*bdf_EmitFun)(uint8_t x, uint8_t bits, void *userdata);
uint8_t bdfont_emit_glyph(const struct FontData *font, uint16_t codepoint,
                          bdf_StartStripe start_stripe, bdf_EmitFun emit,
                          void *userdata);

/* If this code is used in AVR, data is stuffed away into PROGMEM memory.
 * so needs to be dealt with specially.
 */
#ifdef __AVR__
#  include <avr/pgmspace.h>
#  define _bdfont_data_get_bits(b) pgm_read_byte(b)
#  define _bdfont_data_unpack_memory(Type, variable)                    \
  Type _unpacked_##variable;                                            \
  memcpy_P(&_unpacked_##variable, variable, sizeof(_unpacked_##variable)); \
  variable = &_unpacked_##variable
#else
#  define PROGMEM
#  define _bdfont_data_get_bits(b) *b
#  define _bdfont_data_unpack_memory(Type, variable) do {} while(0)
#endif

#define _BDFONT_INTERNAL_EMIT_GLYPH(font, codepoint,                    \
                                    emit_empty_bytes, use_rle,          \
                                    start_stripe_call, emit_call,       \
                                    end_stripe_call)                    \
  ({                                                                    \
    int _return_glyph_width = 0;                                        \
    const struct FontData *_font = (font);                              \
    _bdfont_data_unpack_memory(struct FontData, _font);                 \
    const struct GlyphData *_glyph = bdfont_find_glyph(_font, codepoint); \
    if (_glyph != NULL) {                                               \
      _bdfont_data_unpack_memory(struct GlyphData, _glyph);             \
      const uint8_t *_bits = _font->bits + _glyph->data_offset;         \
      const uint8_t _rle_mask = (_glyph->rle_type == 1) ? 0x0f : 0x03;  \
      const uint8_t _rle_shift = (_glyph->rle_type == 1) ? 4 : 2;       \
                                                                        \
      uint8_t _stripe;                                                  \
      uint8_t _x;                                                       \
      _return_glyph_width = _glyph->width;                              \
      const uint8_t glyph_width __attribute__((unused)) = _glyph->width; \
      for (_stripe = 0; _stripe < _font->stripes; ++_stripe) {          \
        const uint8_t stripe = _stripe;                                 \
        do { start_stripe_call } while(0); /* contain break/continue */ \
                                                                        \
        /* Empty data for empty stripes */                              \
        if (_stripe < _glyph->stripe_begin || _stripe >= _glyph->stripe_end) { \
          if (emit_empty_bytes) {                                       \
            for (_x = 0; _x < _glyph->width; ++_x) {                    \
              const uint8_t b = 0x00;                                   \
              const uint8_t __attribute__((unused)) x = _x;             \
              do { emit_call } while(0); /* contain break/continue */   \
            }                                                           \
          }                                                             \
          do { end_stripe_call } while(0);                              \
          continue;                                                     \
        }                                                               \
                                                                        \
        /* Stripes with data */                                         \
        _x = 0;                                                         \
        while (_x < _glyph->width) {                                    \
          /* left and right margin are empty */                         \
          if (_x < _glyph->left_margin ||                               \
              _x >= _glyph->width - _glyph->right_margin) {             \
            if (emit_empty_bytes) {                                     \
              const uint8_t b = 0x00;                                   \
              const uint8_t __attribute__((unused)) x = _x;             \
              do { emit_call } while(0); /* contain break/continue */   \
            }                                                           \
            _x++;                                                       \
            continue;                                                   \
          }                                                             \
                                                                        \
          uint8_t _data_byte = _bdfont_data_get_bits(_bits++);          \
                                                                        \
          if (!(use_rle) || _glyph->rle_type == 0) {                    \
            const uint8_t b = _data_byte;                               \
            const uint8_t x __attribute__((unused)) = _x;               \
            do { emit_call } while(0); /* contain break/continue */     \
            _x++;                                                       \
          } else {                                                      \
            uint8_t _rlcounts;                                          \
            for (_rlcounts=_data_byte; _rlcounts; _rlcounts >>= _rle_shift) { \
              uint8_t _repetition_count = _rlcounts & _rle_mask;        \
              _data_byte = _bdfont_data_get_bits(_bits++);              \
              while (_repetition_count--) {                             \
                const uint8_t b = _data_byte;                           \
                const uint8_t x __attribute__((unused)) = _x;           \
                do { emit_call } while(0); /* contain break/continue */ \
                _x++;                                                   \
              }                                                         \
            }                                                           \
          }                                                             \
        }                                                               \
        do { end_stripe_call } while(0); /* contain break/continue */   \
      }                                                                 \
    }                                                                   \
    _return_glyph_width;                                                \
  })                                                                    \


/**
 * This is a macro version of the bdfont_emit_glyph() function call above.
 * It allows for somewhat more readable code that can also be smaller (if
 * you only expand it once), which can be important in embedded situations
 * saving a few tens of bytes of code.
 *
 * Similar to the function, it allows to pass in a "font" pointer, a
 * 16Bit-"codepoint" and a block to be called with access to the data.
 * The "start_stripe_call", "emit_call" and "end_stripe_call" should be
 * {}-braced blocks of simple code to be executed for each start of a
 * new stripe, each byte in a stripe and finish of a stripe (Note that
 * the macro preprocessor might get confused with too complicated {} blocks,
 * so keep it simple).
 *
 * Unlike a function call with parameters, the code blocks have access
 * to name values visible in their scope.
 * Within "start_stripe_call" and "end_stripe_call" there is a variable
 * "stripe" and "glyph_width" visible.
 * Within "emit_call", in addition to the values above, a variable
 * named "b" (the byte to be written) and "x" (x-coordinate) are visible in
 * that scope.
 *
 * The "emit_empty_bytes" needs to evaluate to a boolean; if 'true', all
 * bytes are emitted, if 'false', zero bytes at the margins of characters
 * are omitted.
 *
 * The macro returns the width of the character or 0 if it does not exist in
 * the font (this is using a commonly available gcc and clang statement
 * expression extension).
 *
 * Simple Example (Iterating through an ASCII string and display):
   int xpos = 0;
   uint8_t *write_pos;
   for (const char *txt = "Hello World"; *txt; ++txt) {
     xpos += BDFONT_EMIT_GLYPH(&font_foo, *txt, 1,
                               { write_pos = framebuffer + stripe*128 + xpos; },
                               { *write_pos++ = b; },
                               {});
   }
 *
 * Why macro ? This is a pure C-compatible way that allows the compiler to see
 * more optimization opportunities (e.g. optimize out blocks of code if
 * "emit_empty_bytes" is set to false), so more code savings are possible.
 * It also allows for somewhat more readable code than the callback-based
 * version, but of course the usual caveats of macro-expansions apply.
 *
 * Note: the "emit_call" block is expanded multiple times, so keep it
 * short or make a function call.
 */
#define BDFONT_EMIT_GLYPH(font, codepoint, emit_empty_bytes,            \
                          start_stripe_call, emit_call, end_stripe_call) \
  _BDFONT_INTERNAL_EMIT_GLYPH(font, codepoint, emit_empty_bytes,        \
                              BDFONT_USE_RLE,                           \
                              start_stripe_call, emit_call, end_stripe_call)

#if BDFONT_USE_RLE
#  define BDFONT_RLE(x) .rle_type=x
#  define BDFONT_PLAIN(x) .rle_type=x
#else
#  define BDFONT_RLE(x) only_plain_bytes_allowed_with_DBDFONT_USE_RLE_eq_0_Please_invoke_bdfont_data_gen_with_dash_p
#  define BDFONT_PLAIN(x) .rle_type=x
#endif

#ifdef __cplusplus
}
#endif

#endif /* _BDFONT_DATA_FONT_SUPPORT_ */
