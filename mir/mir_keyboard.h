// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Canonical LTD. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_MIR_KEYBOARD_H_
#define OZONE_MIR_KEYBOARD_H_

#include "base/basictypes.h"

#include <mir_toolkit/mir_client_library.h>

namespace ui
{
class EventConverterOzoneWayland;
}

namespace ozonemir {

class Keyboard {
public:
  Keyboard(ui::EventConverterOzoneWayland *sink, unsigned handle);
  ~Keyboard();

  void handle_key_event(MirKeyEvent const& kev);
    
protected:
  DISALLOW_COPY_AND_ASSIGN(Keyboard);

private:
  ui::EventConverterOzoneWayland* const sink;
  unsigned const handle;
};

}  // namespace ozonemir

#endif  // OZONE_MIR_KEYBOARD_H_
