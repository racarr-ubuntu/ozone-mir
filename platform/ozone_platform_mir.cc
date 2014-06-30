// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/platform/ozone_platform_mir.h"

namespace ui {

OzonePlatformMir::OzonePlatformMir() {}

OzonePlatformMir::~OzonePlatformMir() {}

gfx::SurfaceFactoryOzone* OzonePlatformMir::GetSurfaceFactoryOzone() {
    return &hardware_display_;
}

ui::EventFactoryOzone* OzonePlatformMir::GetEventFactoryOzone() {
  return &event_factory_ozone_;
}

ui::CursorFactoryOzone* OzonePlatformMir::GetCursorFactoryOzone() {
  return &cursor_factory_ozone_;
}

void OzonePlatformMir::InitializeUI() {
    // TODO
}

void OzonePlatformMir::InitializeGPU() {
    // TODO
}

OzonePlatform* CreateOzonePlatformWayland() { return new OzonePlatformMir; }
    
}  // namespace ui
