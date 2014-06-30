// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_PLATFORM_OZONE_PLATFORM_MIR_H_
#define OZONE_PLATFORM_OZONE_PLATFORM_MIR_H_

#include "ozone/ui/cursor/cursor_factory_ozone_wayland.h"
#if defined(TOOLKIT_VIEWS) && !defined(OS_CHROMEOS)
#include "ozone/ui/desktop_aura/desktop_factory_wayland.h"
#endif
#include "ozone/ui/events/event_factory_ozone_wayland.h"
//#include "ozone/ui/ime/input_method_context_factory_wayland.h"
#include "ozone/mir/mir_display.h"
#include "ui/base/cursor/ozone/cursor_factory_ozone.h"
#include "ui/ozone/ozone_platform.h"

namespace ui {

// OzonePlatform for Mir
//
// This platform is Linux with the mir display server.
class OzonePlatformMir : public OzonePlatform {
 public:
  OzonePlatformMir();
  virtual ~OzonePlatformMir();

  virtual gfx::SurfaceFactoryOzone* GetSurfaceFactoryOzone() OVERRIDE;
  virtual ui::EventFactoryOzone* GetEventFactoryOzone() OVERRIDE;
  virtual ui::CursorFactoryOzone* GetCursorFactoryOzone() OVERRIDE;
  
  void InitializeUI() OVERRIDE;
  void InitializeGPU() OVERRIDE;

 private:
  ozonemir::Display hardware_display_;
  ui::EventFactoryOzoneWayland event_factory_ozone_;
  ui::CursorFactoryOzoneWayland cursor_factory_ozone_;
#if defined(TOOLKIT_VIEWS) && !defined(OS_CHROMEOS)
  views::DesktopFactoryWayland desktop_factory_ozone_;
#endif

  DISALLOW_COPY_AND_ASSIGN(OzonePlatformMir);
};

// Constructor hook for use in ozone_platform_list.cc
// TODO: Fix
OZONE_WAYLAND_EXPORT OzonePlatform* CreateOzonePlatformWayland();

}  // namespace ui

#endif  // OZONE_PLATFORM_OZONE_PLATFORM_MIR_H_
