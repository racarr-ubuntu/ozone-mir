// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Canonical LTD. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/mir/mir_pointer.h"
#include "ozone/ui/events/event_converter_ozone_wayland.h"

namespace om = ozonemir;

om::Pointer::Pointer(ui::EventConverterOzoneWayland *sink, unsigned handle)
    : sink(sink),
      handle(handle) {
}

om::Pointer::~Pointer() {
}

void om::Pointer::handle_motion_event(MirMotionEvent const& mev) {
    if (mev.action == mir_motion_action_down ||
        mev.action == mir_motion_action_up) {
      ui::EventFlags flags = (ui::EventFlags)0;
      // TODO: Modifiers
      if (mev.button_state == mir_motion_button_primary) {
        flags = ui::EF_LEFT_MOUSE_BUTTON;
      }
      else if (mev.button_state == mir_motion_button_secondary) {
        flags = ui::EF_RIGHT_MOUSE_BUTTON; // TODO: Correct? ~racarr
      }
      else if (mev.button_state == mir_motion_button_tertiary) {
        flags = ui::EF_MIDDLE_MOUSE_BUTTON;
      }

      ui::EventType type = mev.action == mir_motion_action_down ? ui::ET_MOUSE_PRESSED : ui::ET_MOUSE_RELEASED;

      sink->ButtonNotify(handle, type, 
                         flags, mev.pointer_coordinates[0].x,
                         mev.pointer_coordinates[0].y);
    }
    else if (mev.action == mir_motion_action_move) {
      sink->MotionNotify(mev.pointer_coordinates[0].x,
                         mev.pointer_coordinates[0].y);
    }
    else if (mev.action == mir_motion_action_hover_enter) {
      sink->PointerEnter(handle, mev.pointer_coordinates[0].x, mev.pointer_coordinates[0].y);
    }
    else if (mev.action == mir_motion_action_hover_exit) {
      sink->PointerLeave(handle, mev.pointer_coordinates[0].x, mev.pointer_coordinates[0].y);
    }
}
