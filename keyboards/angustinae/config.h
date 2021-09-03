/*
Copyright 2012 Jun Wako <wakojun@gmail.com>
Copyright 2015 Jack Humbert

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "config_common.h"

/* USB Device descriptor parameter */
#define VENDOR_ID       0xFEED
#define PRODUCT_ID      0xCA49
#define DEVICE_VER      0x0001
#define MANUFACTURER    SmollChungus
#define PRODUCT         angustinae

#define USE_SERIAL
#define SOFT_SERIAL_PIN D0

/* Select hand configuration */

#define SPLIT_USB_DECTECT

#define MASTER_LEFT

// Rows are doubled-up
#define MATRIX_ROWS 8
#define MATRIX_ROW_PINS { B6, B2, B3, B1 }

// wiring of each half
#define MATRIX_COLS 6
#define MATRIX_COL_PINS { F7, B4, E6, D7, C6, D4 }

#define DIODE_DIRECTION COL2ROW

/* Set 0 if debouncing isn't needed */
#define DEBOUNCE 5
