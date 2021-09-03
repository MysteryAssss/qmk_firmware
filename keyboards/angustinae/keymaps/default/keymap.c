#include "angustinae.h"

#include QMK_KEYBOARD_H

#define _QWERTY 0
#define _LOWER 1
#define _RAISE 2


#define ALT_TAB     M(KC_ALT_TAB)  // Macro for Alt-Tab
enum macro_keycodes {
  KC_ALT_TAB
};

#define LOWER  MO(_LOWER)
#define RAISE MO(_RAISE)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

[_QWERTY] = LAYOUT_ortho_4x12(
      KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,          KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSPC, \
      KC_LCTL, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,          KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, \
      KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,          KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_ENT , \
      KC_ESC,  KC_LWIN, ALT_TAB, KC_LALT, RAISE,   KC_SPC,        KC_SPC,  LOWER,   KC_LEFT, KC_UP,   KC_DOWN, KC_RGHT \
),

[_LOWER] = LAYOUT_ortho_4x12(
      _______, KC_1,    KC_2,    KC_3,    KC_4,    KC_5,        KC_6,     KC_7,    KC_8,    KC_9,    KC_0,    KC_DEL,  \
      _______, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,       KC_F6,    KC_F7,   KC_F8,   KC_F9,   KC_F10,  _______, \
      _______, KC_F11,  KC_F12,  KC_F13,  KC_F14,  KC_F15,      KC_F16,   KC_F17,  KC_F18,  KC_F19,  KC_F20,  _______, \
      _______, _______, _______, _______, _______, _______,     _______,  _______, KC_VOLD, KC_MPRV, KC_MNXT, KC_VOLU \
),

[_RAISE] = LAYOUT_ortho_4x12(
      KC_TILD, KC_EXLM, KC_AT,  KC_HASH, KC_DLR,  KC_PERC,     KC_CIRC, KC_AMPR, KC_ASTR, KC_LPRN, KC_RPRN, KC_DEL, \
      _______, KC_MINS, KC_EQL, KC_LBRC, KC_RBRC, KC_BSLS,     KC_MINS, KC_UNDS, KC_PLUS,  KC_LCBR, KC_RCBR, KC_PIPE, \
      _______, KC_PSCR, _______, _______, _______, _______,    _______, _______, _______, KC_HOME, KC_END,  KC_ENT , \
      _______, _______, _______, _______, _______, _______,    _______, _______, _______, _______, _______, _______ \
)
};

const macro_t *action_get_macro(keyrecord_t *record, uint8_t id, uint8_t opt) {
    if (!eeconfig_is_enabled()) {
        eeconfig_init();
    }

    switch (id) {
        case KC_ALT_TAB:
            return (record->event.pressed ? MACRO( D(LALT), D(TAB), END ) : MACRO( U(TAB), END ));
    }
    return MACRO_NONE;
}
