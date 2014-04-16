// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_MIR_WINDOW_H_
#define OZONE_MIR_WINDOW_H_

#include "ui/gfx/rect.h"
#include "base/strings/string16.h"

#include "ozone/ui/events/window_constants.h"

#include <mir_toolkit/mir_client_library.h>

namespace ozonemir {

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

protected:
  DISALLOW_COPY_AND_ASSIGN(Window);

private:
  MirConnection *connection_;
  unsigned handle_;

  MirSurface *surface_;
  
  bool processing_events_;

  static void HandleEvent(MirSurface *surface, MirEvent const* ev, void *context);

  void NotifyResize();
};

}  // namespace ozonemir

#endif  // OZONE_MIR_WINDOW_H_
