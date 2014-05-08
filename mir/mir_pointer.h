// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright 2014 Canonical LTD. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_MIR_POINTER_H_
#define OZONE_MIR_POINTER_H_

#include "base/basictypes.h"

#include <mir_toolkit/mir_client_library.h>

namespace ui
{
class EventConverterOzoneWayland;
}

namespace ozonemir {

class Pointer {
public:
  Pointer(ui::EventConverterOzoneWayland *sink, unsigned handle);
  ~Pointer();

  void handle_motion_event(MirMotionEvent const& mev);
    
protected:
  DISALLOW_COPY_AND_ASSIGN(Pointer);

private:
  ui::EventConverterOzoneWayland* const sink;
  unsigned const handle;
  
  // We only support the basic 3 buttons in ozone...
  static int const num_buttons = 3;
  bool button_depressed[num_buttons];
};

}  // namespace ozonemir

#endif  // OZONE_MIR_POINTER_H_
