// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/mir/mir_display.h"

namespace om = ozonemir;

om::Display::Display() : connection_(NULL) {
  gfx::OzoneDisplay::SetInstance(this);
}

om::Display::~Display() {
  Terminate();
  gfx::OzoneDisplay::SetInstance(NULL);
}

gfx::SurfaceFactoryOzone::HardwareState
om::Display::InitializeHardware() {
    
    connection_ = mir_connect_sync(NULL, "Chromium");
    
    if (!connection_)
    {
        // TODO: Mir error
        LOG(ERROR) << "SurfaceFactoryMir failed to initialize connection to remote Mir Server";
        return gfx::SurfaceFactoryOzone::FAILED;
    }

    return gfx::SurfaceFactoryOzone::INITIALIZED;
}

void om::Display::ShutdownHardware() {
  Terminate();
}

intptr_t om::Display::GetNativeDisplay() {
    DCHECK(connection_);
    return reinterpret_cast<intptr_t>(mir_connection_get_egl_native_display(connection_));
}

void om::Display::FlushDisplay() {
    // TODO: ?
}

gfx::AcceleratedWidget om::Display::GetAcceleratedWidget() {
    static int opaque_handle = 0; // TODO: Threadsafe?
    opaque_handle++;
    ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(opaque_handle,
                                                                ui::CREATE,
                                                                0,
                                                                0);
    
    return (gfx::AcceleratedWidget)opaque_handle;
}

gfx::AcceleratedWidget om::Display::RealizeAcceleratedWidget(
    gfx::AcceleratedWidget w) {
    // May not be needed in Mir...we would need to use the state change handler.
    DCHECK(connection_);
}

bool om::Display::AttemptToResizeAcceleratedWidget(
    gfx::AcceleratedWidget w, const gfx::Size& bounds) {
    ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(
        w, ui::RESIZE, bounds.width(), bounds.height());

    return true;
}

void om::Display::DestroyWidget(gfx::AcceleratedWidget w) {
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(w,
      ui::DESTROYED);
}

void om::Display::LookAheadOutputGeometry() {
    // TODO
}

void om::Display::Terminate() {
    mir_connection_release(connection_);
}

void SetWidgetState(unsigned widget, ui::WidgetState state, unsigned width, unsigned height) {
}

void SetWidgetTitle(unsigned w, const base::string16& title) {
}

void SetWidgetCursor(int cursor_type) {
}

void SetWidgetAttributes(unsigned widget, unsigned parent,
                         unsigned x, unsigned y, ui::WidgetType type) {
}
