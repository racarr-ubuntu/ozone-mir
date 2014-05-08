// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright 2014 Canonical LTD. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/mir/mir_keyboard.h"
#include "ozone/ui/events/event_converter_ozone_wayland.h"

#include "ozone/ui/events/keyboard_codes_ozone.h"

#include <xkbcommon/xkbcommon.h>

namespace om = ozonemir;

namespace
{
uint32_t TranslateMirModifiers(unsigned mir_modifiers)
{
  uint32_t ui_m = 0;
  
  if (mir_modifiers & mir_key_modifier_shift)
    ui_m |= ui::EF_SHIFT_DOWN;
  if (mir_modifiers & mir_key_modifier_ctrl)
    ui_m |= ui::EF_CONTROL_DOWN;
  if (mir_modifiers & mir_key_modifier_meta)
    ui_m |= ui::EF_ALT_DOWN;
  if (mir_modifiers & mir_key_modifier_caps_lock)
    ui_m |= ui::EF_CAPS_LOCK_DOWN;
  
  return ui_m;
}

xkb_keysym_t NormalizeKeysym(xkb_keysym_t keysym)
{
  if ((keysym >= XKB_KEY_A && keysym <= XKB_KEY_Z) ||
       (keysym >= XKB_KEY_a && keysym <= XKB_KEY_z) ||
         (keysym >= XKB_KEY_0 && keysym <= XKB_KEY_9))
      return keysym;

  if (keysym >= XKB_KEY_KP_0 && keysym <= XKB_KEY_KP_9) {
    // Numpad Number-keys can be represented by a keysym value of 0-9 nos.
    return  XKB_KEY_0 + (keysym - XKB_KEY_KP_0);
  } else if (keysym > 0x01000100 && keysym < 0x01ffffff) {
    // Any UCS character in this range will simply be the character's
    // Unicode number plus 0x01000000.
    return  keysym - 0x001000000;
  } else if (keysym >= XKB_KEY_F1 && keysym <= XKB_KEY_F24) {
    return  ui::OZONEACTIONKEY_F1 + (keysym - XKB_KEY_F1);
  } else if (keysym >= XKB_KEY_KP_F1 && keysym <= XKB_KEY_KP_F4) {
      return  ui::OZONEACTIONKEY_F1 + (keysym - XKB_KEY_KP_F1);
  } else {
      switch (keysym) {
        case XKB_KEY_dead_circumflex:
          return  ui::OZONECHARCODE_CARET_CIRCUMFLEX;
        case XKB_KEY_dead_diaeresis:
          return  ui::OZONECHARCODE_SPACING_DIAERESIS;
        case XKB_KEY_dead_perispomeni:
          return  ui::OZONECHARCODE_TILDE;
        case XKB_KEY_dead_acute:
          return  ui::OZONECHARCODE_SPACING_ACUTE;
        case XKB_KEY_dead_grave:
          return  ui::OZONECHARCODE_GRAVE_ASSCENT;
        case XKB_KEY_endash:
          return  ui::OZONECHARCODE_ENDASH;
        case XKB_KEY_singlelowquotemark:
          return  ui::OZONECHARCODE_SINGLE_LOW_QUOTATION_MARK;
        case XKB_KEY_dead_cedilla:
          return  ui::OZONECHARCODE_SPACING_CEDILLA;
        case XKB_KEY_KP_Equal:
          return  ui::OZONECHARCODE_EQUAL;
        case XKB_KEY_KP_Multiply:
          return  ui::OZONECHARCODE_MULTIPLY;
        case XKB_KEY_KP_Add:
          return  ui::OZONECHARCODE_PLUS;
        case XKB_KEY_KP_Separator:
          return  ui::OZONECHARCODE_COMMA;
        case XKB_KEY_KP_Subtract:
          return  ui::OZONECHARCODE_MINUS;
        case XKB_KEY_KP_Decimal:
        case XKB_KEY_period:
          return  ui::OZONECHARCODE_PERIOD;
        case XKB_KEY_KP_Divide:
          return  ui::OZONECHARCODE_DIVISION;
        case XKB_KEY_Delete:
        case XKB_KEY_KP_Delete:
          return  ui::OZONEACTIONKEY_DELETE;
        case XKB_KEY_KP_Tab:
        case XKB_KEY_ISO_Left_Tab:
        case XKB_KEY_Tab:
        case XKB_KEY_3270_BackTab:
          return  ui::OZONEACTIONKEY_TAB;
        case XKB_KEY_Sys_Req:
        case XKB_KEY_Escape:
          return  ui::OZONEACTIONKEY_ESCAPE;
        case XKB_KEY_Linefeed:
          return  ui::OZONECHARCODE_LINEFEED;
        case XKB_KEY_Return:
        case XKB_KEY_KP_Enter:
        case XKB_KEY_ISO_Enter:
          return  ui::OZONEACTIONKEY_RETURN;
        case XKB_KEY_KP_Space:
        case XKB_KEY_space:
          return  ui::OZONEACTIONKEY_SPACE;
        case XKB_KEY_dead_caron:
          return  ui::OZONECHARCODE_CARON;
        case XKB_KEY_BackSpace:
          return  ui::OZONEACTIONKEY_BACK;
        case XKB_KEY_dead_doubleacute:
          return  ui::OZONECHARCODE_DOUBLE_ACUTE_ACCENT;
        case XKB_KEY_dead_horn:
          return  ui::OZONECHARCODE_COMBINING_HORN;
        case XKB_KEY_oe:
          return  ui::OZONECHARCODE_LSMALL_OE;
        case XKB_KEY_OE:
          return  ui::OZONECHARCODE_LOE;
        case XKB_KEY_idotless:
          return  ui::OZONECHARCODE_LSMALL_DOT_LESS_I;
        case XKB_KEY_kra:
          return  ui::OZONECHARCODE_LSMALL_KRA;
        case XKB_KEY_dead_stroke:
          return  ui::OZONECHARCODE_MINUS;
        case XKB_KEY_eng:
          return  ui::OZONECHARCODE_LSMALL_ENG;
        case XKB_KEY_ENG:
          return  ui::OZONECHARCODE_LENG;
        case XKB_KEY_leftsinglequotemark:
          return  ui::OZONECHARCODE_LEFT_SINGLE_QUOTATION_MARK;
        case XKB_KEY_rightsinglequotemark:
          return  ui::OZONECHARCODE_RIGHT_SINGLE_QUOTATION_MARK;
        case XKB_KEY_dead_belowdot:
          return  ui::OZONECHARCODE_COMBINING_DOT_BELOW;
        case XKB_KEY_dead_belowdiaeresis:
          return  ui::OZONECHARCODE_COMBINING_DIAERESIS_BELOW;
        case XKB_KEY_Clear:
        case XKB_KEY_KP_Begin:
          return  ui::OZONEACTIONKEY_CLEAR;
        case XKB_KEY_Home:
        case XKB_KEY_KP_Home:
          return  ui::OZONEACTIONKEY_HOME;
        case XKB_KEY_End:
        case XKB_KEY_KP_End:
          return  ui::OZONEACTIONKEY_END;
        case XKB_KEY_Page_Up:
        case XKB_KEY_KP_Page_Up:  // aka XKB_KEY_KP_Prior
          return  ui::OZONEACTIONKEY_PRIOR;
        case XKB_KEY_Page_Down:
        case XKB_KEY_KP_Page_Down:  // aka XKB_KEY_KP_Next
          return  ui::OZONEACTIONKEY_NEXT;
        case XKB_KEY_Left:
        case XKB_KEY_KP_Left:
          return  ui::OZONEACTIONKEY_LEFT;
        case XKB_KEY_Right:
        case XKB_KEY_KP_Right:
          return  ui::OZONEACTIONKEY_RIGHT;
        case XKB_KEY_Down:
        case XKB_KEY_KP_Down:
          return  ui::OZONEACTIONKEY_DOWN;
        case XKB_KEY_Up:
        case XKB_KEY_KP_Up:
          return  ui::OZONEACTIONKEY_UP;
        case XKB_KEY_Kana_Lock:
        case XKB_KEY_Kana_Shift:
          return  ui::OZONEACTIONKEY_KANA;
        case XKB_KEY_Hangul:
          return  ui::OZONEACTIONKEY_HANGUL;
        case XKB_KEY_Hangul_Hanja:
          return  ui::OZONEACTIONKEY_HANJA;
        case XKB_KEY_Kanji:
          return  ui::OZONEACTIONKEY_KANJI;
        case XKB_KEY_Henkan:
          return  ui::OZONEACTIONKEY_CONVERT;
        case XKB_KEY_Muhenkan:
          return  ui::OZONEACTIONKEY_NONCONVERT;
        case XKB_KEY_Zenkaku_Hankaku:
          return  ui::OZONEACTIONKEY_DBE_DBCSCHAR;
        case XKB_KEY_ISO_Level5_Shift:
          return  ui::OZONEACTIONKEY_OEM_8;
        case XKB_KEY_Shift_L:
        case XKB_KEY_Shift_R:
          return  ui::OZONEACTIONKEY_SHIFT;
        case XKB_KEY_Control_L:
        case XKB_KEY_Control_R:
          return  ui::OZONEACTIONKEY_CONTROL;
        case XKB_KEY_Meta_L:
        case XKB_KEY_Meta_R:
        case XKB_KEY_Alt_L:
        case XKB_KEY_Alt_R:
          return  ui::OZONEACTIONKEY_MENU;
        case XKB_KEY_ISO_Level3_Shift:
          return  ui::OZONEACTIONKEY_ALTGR;
        case XKB_KEY_Multi_key:
          return  0xE6;
        case XKB_KEY_Pause:
          return  ui::OZONEACTIONKEY_PAUSE;
        case XKB_KEY_Caps_Lock:
          return  ui::OZONEACTIONKEY_CAPITAL;
        case XKB_KEY_Num_Lock:
          return  ui::OZONEACTIONKEY_NUMLOCK;
        case XKB_KEY_Scroll_Lock:
          return  ui::OZONEACTIONKEY_SCROLL;
        case XKB_KEY_Select:
          return  ui::OZONEACTIONKEY_SELECT;
        case XKB_KEY_Print:
          return  ui::OZONEACTIONKEY_PRINT;
        case XKB_KEY_Execute:
          return  ui::OZONEACTIONKEY_EXECUTE;
        case XKB_KEY_Insert:
        case XKB_KEY_KP_Insert:
          return  ui::OZONEACTIONKEY_INSERT;
        case XKB_KEY_Help:
          return  ui::OZONEACTIONKEY_HELP;
        case XKB_KEY_Super_L:
          return  ui::OZONEACTIONKEY_LWIN;
        case XKB_KEY_Super_R:
          return  ui::OZONEACTIONKEY_RWIN;
        case XKB_KEY_Menu:
          return  ui::OZONEACTIONKEY_APPS;
        case XKB_KEY_XF86Tools:
          return  ui::OZONEACTIONKEY_F13;
        case XKB_KEY_XF86Launch5:
          return  ui::OZONEACTIONKEY_F14;
        case XKB_KEY_XF86Launch6:
          return  ui::OZONEACTIONKEY_F15;
        case XKB_KEY_XF86Launch7:
          return  ui::OZONEACTIONKEY_F16;
        case XKB_KEY_XF86Launch8:
          return  ui::OZONEACTIONKEY_F17;
        case XKB_KEY_XF86Launch9:
          return  ui::OZONEACTIONKEY_F18;

        // For supporting multimedia buttons on a USB keyboard.
        case XKB_KEY_XF86Back:
          return  ui::OZONEACTIONKEY_BROWSER_BACK;
        case XKB_KEY_XF86Forward:
          return  ui::OZONEACTIONKEY_BROWSER_FORWARD;
        case XKB_KEY_XF86Reload:
          return  ui::OZONEACTIONKEY_BROWSER_REFRESH;
        case XKB_KEY_XF86Stop:
          return  ui::OZONEACTIONKEY_BROWSER_STOP;
        case XKB_KEY_XF86Search:
          return  ui::OZONEACTIONKEY_BROWSER_SEARCH;
        case XKB_KEY_XF86Favorites:
          return  ui::OZONEACTIONKEY_BROWSER_FAVORITES;
        case XKB_KEY_XF86HomePage:
          return  ui::OZONEACTIONKEY_BROWSER_HOME;
        case XKB_KEY_XF86AudioMute:
          return  ui::OZONEACTIONKEY_VOLUME_MUTE;
        case XKB_KEY_XF86AudioLowerVolume:
          return  ui::OZONEACTIONKEY_VOLUME_DOWN;
        case XKB_KEY_XF86AudioRaiseVolume:
          return  ui::OZONEACTIONKEY_VOLUME_UP;
        case XKB_KEY_XF86AudioNext:
          return  ui::OZONEACTIONKEY_MEDIA_NEXT_TRACK;
        case XKB_KEY_XF86AudioPrev:
          return  ui::OZONEACTIONKEY_MEDIA_PREV_TRACK;
        case XKB_KEY_XF86AudioStop:
          return  ui::OZONEACTIONKEY_MEDIA_STOP;
        case XKB_KEY_XF86AudioPlay:
          return  ui::OZONEACTIONKEY_MEDIA_PLAY_PAUSE;
        case XKB_KEY_XF86Mail:
          return  ui::OZONEACTIONKEY_MEDIA_LAUNCH_MAIL;
        case XKB_KEY_XF86LaunchA:
          return  ui::OZONEACTIONKEY_MEDIA_LAUNCH_APP1;
        case XKB_KEY_XF86LaunchB:
        case XKB_KEY_XF86Calculator:
          return  ui::OZONEACTIONKEY_MEDIA_LAUNCH_APP2;
        case XKB_KEY_XF86WLAN:
          return  ui::OZONEACTIONKEY_WLAN;
        case XKB_KEY_XF86PowerOff:
          return  ui::OZONEACTIONKEY_POWER;
        case XKB_KEY_XF86MonBrightnessDown:
          return  ui::OZONEACTIONKEY_BRIGHTNESS_DOWN;
        case XKB_KEY_XF86MonBrightnessUp:
          return  ui::OZONEACTIONKEY_BRIGHTNESS_UP;
        case XKB_KEY_XF86KbdBrightnessDown:
          return  ui::OZONEACTIONKEY_KBD_BRIGHTNESS_DOWN;
        case XKB_KEY_XF86KbdBrightnessUp:
          return  ui::OZONEACTIONKEY_KBD_BRIGHTNESS_UP;
        case XKB_KEY_emptyset:
        case XKB_KEY_NoSymbol:
          return  ui::OZONECHARCODE_NULL;
        default:
          break;
    }
  }
}
}

// TODO: Is handle really needed?
om::Keyboard::Keyboard(ui::EventConverterOzoneWayland *sink, unsigned handle)
    : sink(sink),
      handle(handle) {
}

om::Keyboard::~Keyboard() {
}

void om::Keyboard::handle_key_event(MirKeyEvent const& kev) {
    ui::EventType type = kev.action == mir_key_action_down ? ui::ET_KEY_PRESSED : ui::ET_KEY_RELEASED;
    
    sink->KeyNotify(type, NormalizeKeysym(kev.key_code),
                    TranslateMirModifiers(kev.modifiers));

}
