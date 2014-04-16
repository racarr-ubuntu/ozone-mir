// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_MIR_OZONE_DISPLAY_H_
#define OZONE_MIR_OZONE_DISPLAY_H_

#include "base/basictypes.h"
#include "ozone/ui/events/window_state_change_handler.h"
#include "ozone/ui/gfx/ozone_display.h"

#include <mir_toolkit/mir_client_library.h>

#include <mutex>
#include <map>

namespace ozonemir {

class Display : public gfx::OzoneDisplay, public ui::WindowStateChangeHandler {
public:
  Display();
  virtual ~Display();

  // gfx::OzoneDisplay
  virtual gfx::SurfaceFactoryOzone::HardwareState InitializeHardware() OVERRIDE;
  virtual void ShutdownHardware() OVERRIDE;
  virtual intptr_t GetNativeDisplay() OVERRIDE;
  virtual void FlushDisplay() OVERRIDE;

  virtual gfx::AcceleratedWidget GetAcceleratedWidget() OVERRIDE;
  virtual gfx::AcceleratedWidget RealizeAcceleratedWidget(
      gfx::AcceleratedWidget w) OVERRIDE;
  virtual bool AttemptToResizeAcceleratedWidget(
      gfx::AcceleratedWidget w, const gfx::Size& bounds) OVERRIDE;
  virtual void DestroyWidget(gfx::AcceleratedWidget w) OVERRIDE;
  virtual void LookAheadOutputGeometry() OVERRIDE;
  
  // ui::WindowStateChangeHandler
  virtual void SetWidgetState(unsigned widget, ui::WidgetState state, unsigned width = 0, unsigned height = 0) OVERRIDE;
  virtual void SetWidgetTitle(unsigned w,
                              const base::string16& title) OVERRIDE;
  virtual void SetWidgetCursor(int cursor_type) OVERRIDE;
  virtual void SetWidgetAttributes(unsigned widget,
                                   unsigned parent,
                                   unsigned x,
                                   unsigned y,
                                   ui::WidgetType type) OVERRIDE;

private:
  void Terminate();

  MirConnection *connection_;

  std::mutex widget_map_guard;
  std::map<unsigned, MirSurface*> widgets;

  DISALLOW_COPY_AND_ASSIGN(Display);
};

}  // namespace ozonemir

#endif  // OZONE_MIR_OZONE_DISPLAY_H_
