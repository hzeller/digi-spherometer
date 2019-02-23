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

#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <util/delay.h>

#include "ssd1306-display.h"

#include "font-bignumber.h"
#include "font-smalltext.h"

static const uint8_t PROGMEM raster[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, };

int main() {
    _delay_ms(100);  // Let display warm up and get ready before the first i2c
    SSD1306Display display;
    display.SetOn(true);

    for (int y=0; y < 8; ++y) {
        for (int x = 0; x < 128; x+=8) {
            display.DrawPageFromProgmem(y, x, x+8, raster);
        }
    }

    display.WriteString(&progmem_font_smalltext.meta, 0, 0, "200μm ±1μm");
    display.WriteString(&progmem_font_smalltext.meta, 0, 0, ".0001\" ±.0001\"");
    display.WriteString(&progmem_font_smalltext.meta, 0, 48, "[1233mm ... 1235mm]");
    display.WriteString(&progmem_font_bignumber.meta, 0, 16, "133.7");

    for (;;) {
        display.WriteString(&progmem_font_bignumber.meta, 0, 16, "133.7");
        display.WriteString(&progmem_font_bignumber.meta, 0, 16, "133.8");
        display.WriteString(&progmem_font_bignumber.meta, 0, 16, "133.9");
        display.WriteString(&progmem_font_bignumber.meta, 0, 16, "134.0");
    }

    // Powersave testing.
#if 0
    //i2c.Enable(false);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_cpu();
#endif
}
