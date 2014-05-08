// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright 2014 Canonical LTD. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/mir/mir_pointer.h"
#include "ozone/ui/events/event_converter_ozone_wayland.h"

namespace om = ozonemir;

om::Pointer::Pointer(ui::EventConverterOzoneWayland *sink, unsigned handle)
  : sink(sink),
    handle(handle) {
  for (int i = 0; i < num_buttons; i++)
    button_depressed[i] = false;
}

om::Pointer::~Pointer() {
}

void om::Pointer::handle_motion_event(MirMotionEvent const& mev) {
  // TODO: Tracking the button state like this is a pain...
  static int const num_buttons = 3;
  static MirMotionButton buttons[3] = {mir_motion_button_primary,
                                       mir_motion_button_secondary,
                                       mir_motion_button_tertiary};
  static ui::EventFlags ozone_buttons[3] = {ui::EF_LEFT_MOUSE_BUTTON,
                                            ui::EF_RIGHT_MOUSE_BUTTON,
                                            ui::EF_MIDDLE_MOUSE_BUTTON};
                                    
  if (mev.action == mir_motion_action_down ||
      mev.action == mir_motion_action_up)
    {
      for (int i = 0; i < num_buttons; i++)
        {
          auto state = mev.button_state & buttons[i];
          if (state != button_depressed[i])
            {
              // TODO: Modifiers?
              ui::EventFlags flags;
              flags = ozone_buttons[i];

              ui::EventType type = mev.action == mir_motion_action_down ? ui::ET_MOUSE_PRESSED : ui::ET_MOUSE_RELEASED;

              sink->ButtonNotify(handle, type, 
                                 flags, mev.pointer_coordinates[0].x,
                                 mev.pointer_coordinates[0].y);

            }
          button_depressed[i] = state;
        }
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
