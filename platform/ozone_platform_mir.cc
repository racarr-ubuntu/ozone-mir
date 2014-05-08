// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/platform/ozone_platform_mir.h"

namespace ui {

OzonePlatformMir::OzonePlatformMir() {}

OzonePlatformMir::~OzonePlatformMir() {}

gfx::SurfaceFactoryOzone* OzonePlatformMir::GetSurfaceFactoryOzone() {
  return &surface_factory_ozone_;
}

ui::EventFactoryOzone* OzonePlatformMir::GetEventFactoryOzone() {
  return &event_factory_ozone_;
}

ui::InputMethodContextFactoryOzone*
OzonePlatformMir::GetInputMethodContextFactoryOzone() {
  return &input_method_context_factory_;
}

ui::CursorFactoryOzone* OzonePlatformMir::GetCursorFactoryOzone() {
  return &cursor_factory_ozone_;
}

OzonePlatform* CreateOzonePlatformWayland() { return new OzonePlatformMir; }

}  // namespace ui
