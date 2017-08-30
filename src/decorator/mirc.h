/* Copyright (C) 2016-2017 Shengyu Zhang <srain@srain.im>
 *
 * This file is part of Srain.
 *
 * Srain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __MIRC_H
#define __MIRC_H

/* mIRC color control characters */
#define MIRC_BOLD       0x02
#define MIRC_ITALICS    0x1D
#define MIRC_UNDERLINE  0x1F
#define MIRC_REVERSE    0x16
#define MIRC_BLINK      0x06
#define MIRC_PLAIN      0x0F
#define MIRC_COLOR      0x03

/* mIRC color code */
enum {
    MIRC_COLOR_WHITE        = 0,
    MIRC_COLOR_BLACK        = 1,
    MIRC_COLOR_NAVY         = 2,
    MIRC_COLOR_GREEN        = 3,
    MIRC_COLOR_RED          = 4,
    MIRC_COLOR_MAROON       = 5,
    MIRC_COLOR_PURPLE       = 6,
    MIRC_COLOR_OLIVE        = 7,
    MIRC_COLOR_YELLOW       = 8,
    MIRC_COLOR_LIGHT_GREEN  = 9,
    MIRC_COLOR_TEAL         = 10,
    MIRC_COLOR_CYAN         = 11,
    MIRC_COLOR_ROYAL_BLUE   = 12,
    MIRC_COLOR_MAGENTA      = 13,
    MIRC_COLOR_GRAY         = 14,
    MIRC_COLOR_LIGHT_GRAY   = 15,
    MIRC_COLOR_UNKNOWN      = 16, // Not a part of protocol, just for convenience
};

#endif /* __MIRC_H */
