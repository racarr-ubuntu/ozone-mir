// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright 2014 Canonical LTD. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/mir/mir_display.h"
#include "ozone/mir/mir_window.h"

#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/ui/events/output_change_observer.h"
#include "ozone/ui/events/ime_state_change_handler.h"

namespace om = ozonemir;

// TODO: Extract
namespace
{
class NullIMEStateChangeHandler : public ui::IMEStateChangeHandler
{
  void ResetIme() {}
  void ImeCaretBoundsChanged(gfx::Rect) {}
  void ShowInputPanel() {}
  void HideInputPanel() {}
};
}

om::Display::Display() : connection_(NULL) {
  gfx::OzoneDisplay::SetInstance(this);
  // TODO: Leak
}

om::Display::~Display() {
  Terminate();
  gfx::OzoneDisplay::SetInstance(NULL);
// TODO: Need in_gpu_process
//  ui::WindowStateChangeHandler::SetInstance(NULL);
}

gfx::SurfaceFactoryOzone::HardwareState
om::Display::InitializeHardware() {
  if (connection_)
    return gfx::SurfaceFactoryOzone::INITIALIZED;
  ui::WindowStateChangeHandler::SetInstance(this);
  // TODO: Does this really belong here?
  ui::IMEStateChangeHandler::SetInstance(new NullIMEStateChangeHandler);
    
  connection_ = mir_connect_sync(NULL, "Chromium");
  if (!mir_connection_is_valid(connection_)) {
    LOG(ERROR) << "SurfaceFactoryMir failed to initialize connection to remote Mir Server: "
      << mir_connection_get_error_message(connection_);
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
  StartProcessingEvents();

  // May not be needed in Mir...we would need to use the state change handler.
  DCHECK(connection_);
  auto widget = GetWidget(w);
  DCHECK(widget);
    
  return (gfx::AcceleratedWidget)widget->egl_window();
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

// See comment in wayland/display.cc
void om::Display::LookAheadOutputGeometry() {
  ui::EventFactoryOzoneWayland* event_factory =
    ui::EventFactoryOzoneWayland::GetInstance();
  DCHECK(event_factory->GetOutputChangeObserver());

  // TODO: Implement for real.
  unsigned width = 1024;
  unsigned height = 1024;
  event_factory->GetOutputChangeObserver()->OnOutputSizeChanged(width, height);
}

void om::Display::Terminate() {
  mir_connection_release(connection_);
}

om::Window* om::Display::GetWidget(unsigned w) {
  std::unique_lock<decltype(widget_map_guard)> lg(widget_map_guard);
  auto it = widgets.find(w);
  return it == widgets.end() ? NULL : it->second;
}

void om::Display::CreateWindow(unsigned w) {
  // TODO: Error checking if w exists.
  std::unique_lock<decltype(widget_map_guard)> lg(widget_map_guard);
  widgets[w] = new om::Window(connection_, w);
}

void om::Display::DestroyWindow(unsigned w) {
  std::unique_lock<decltype(widget_map_guard)> lg(widget_map_guard);
  auto it = widgets.find(w);
  if (it == widgets.end())
    return; // TODO: Error?
  delete it->second;
  widgets.erase(it);
}

void om::Display::ForEachWidget(std::function<void(om::Window*)> const& exec) {
  std::unique_lock<decltype(widget_map_guard)> lg(widget_map_guard);
  for (auto kv : widgets)
    exec(kv.second);
}

// TODO: Consider thread safety here.
void om::Display::StartProcessingEvents() {
  ForEachWidget([](om::Window* w) {
      w->StartProcessingEvents();
    });
}

void om::Display::StopProcessingEvents() {
  ForEachWidget([](om::Window* w) {
      w->StopProcessingEvents();
    });
}

bool om::Display::AnyWindowsExist() {
  return !widgets.empty();
}

void om::Display::SetWidgetState(unsigned w, ui::WidgetState state, unsigned width, unsigned height) {
  switch (state) {
  case ui::CREATE:
    {
      CreateWindow(w);
      break;
    }
  case ui::FULLSCREEN:
    {
      om::Window* widget = GetWidget(w);
      widget->SetFullscreen();
      widget->Resize(width, height);
      break;
    }
  case ui::MAXIMIZED:
    {
      om::Window* widget = GetWidget(w);
      widget->Maximize();
      break;
    }
  case ui::MINIMIZED:
    {
      om::Window* widget = GetWidget(w);
      widget->Minimize();
      break;
    }
  case ui::RESTORE:
    {
      om::Window* widget = GetWidget(w);
      widget->Restore();
      widget->Resize(width, height);
      break;
    }
  case ui::ACTIVE:
    NOTIMPLEMENTED() << " ACTIVE " << w;
    break;
  case ui::INACTIVE:
    NOTIMPLEMENTED() << " INACTIVE " << w;
    break;
  case ui::SHOW:
    NOTIMPLEMENTED() << " SHOW " << w;
    break;
  case ui::HIDE:
    NOTIMPLEMENTED() << " HIDE " << w;
    break;
  case ui::RESIZE:
    {
      om::Window* window = GetWidget(w);
      DCHECK(window);
      window->Resize(width, height);
      break;
    }
  case ui::DESTROYED:
    {
      DestroyWindow(w);
      if (!AnyWindowsExist())
        StopProcessingEvents();
      break;
    }
  default:
    break;
  }
}

void om::Display::SetWidgetTitle(unsigned w, const base::string16& title) {
  auto widget = GetWidget(w);
  DCHECK(widget);
  widget->SetWindowTitle(title);
}

void om::Display::SetWidgetCursor(int cursor_type) {
  // TODO: Implement
}

void om::Display::SetWidgetAttributes(unsigned w, unsigned parent,
                                      unsigned x, unsigned y, ui::WidgetType type) {
  // TODO: Support popups
  auto widget = GetWidget(w);
  widget->SetWindowType(type);
}

char const* gfx::OzoneDisplay::GetEGLPlatform() {
  return "mir";
}
