// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright 2014 Canonical LTD. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_MIR_WINDOW_H_
#define OZONE_MIR_WINDOW_H_

#include "ui/gfx/rect.h"
#include "base/strings/string16.h"
#include "ui/gfx/ozone/surface_ozone_egl.h" // TODO: Forward decl?

#include "ozone/ui/events/window_constants.h"

#include <mir_toolkit/mir_client_library.h>

namespace ozonemir {
class Pointer;
class Keyboard;

class Window {
public:
  Window(MirConnection *connection, unsigned handle);
  ~Window();
    
  void Maximize();
  void Minimize();
  void Restore();
  void SetFullscreen();
    
  unsigned Handle() const;
    
  void RealizeAcceleratedWidget();
    
  intptr_t egl_window();
   
  void Resize(unsigned width, unsigned height);
  gfx::Rect GetBounds() const;

  void StartProcessingEvents();
  void StopProcessingEvents();

  void SetWindowTitle(const base::string16& title);

  void SetWindowType(ui::WidgetType type);
  
  scoped_ptr<gfx::SurfaceOzoneEGL> CreateOzoneEGLSurface();

protected:
  DISALLOW_COPY_AND_ASSIGN(Window);

private:
  MirConnection *connection_;
  unsigned handle_;

  MirSurface *surface_;
  
  bool processing_events_;

  ozonemir::Pointer *motion_event_handler_;
  ozonemir::Keyboard *key_event_handler_;

  static void HandleEvent(MirSurface *surface, MirEvent const* ev, void *context);

  void NotifyResize();
};

}  // namespace ozonemir

#endif  // OZONE_MIR_WINDOW_H_
