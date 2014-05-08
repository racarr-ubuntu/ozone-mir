// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Canonical LTD. All rights reserved.
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
};

}  // namespace ozonemir

#endif  // OZONE_MIR_POINTER_H_
