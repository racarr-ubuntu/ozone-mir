// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright 2014 Canonical LTD. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/mir/mir_window.h"
#include "ozone/mir/mir_pointer.h"
#include "ozone/mir/mir_keyboard.h"

#include "base/message_loop/message_pump_ozone.h"

#include "base/memory/scoped_ptr.h"
#include "ui/events/event.h"

#include "content/child/child_thread.h"
#include "content/child/child_process.h"

#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/ui/events/event_converter_ozone_wayland.h"
// TODO: #include "ozone/ui/ime/keyboard_engine_xkb.h"

namespace om = ozonemir;

namespace
{
MirPixelFormat choose_pixel_format(MirConnection *connection) {
  MirPixelFormat format;
  unsigned int got_formats;
  mir_connection_get_available_surface_formats(connection, &format, 1, &got_formats);
  DCHECK(got_formats > 0);

  return format;
}
}

om::Window::Window(MirConnection *connection, unsigned handle)
  : connection_(connection),
    handle_(handle),
    surface_(0),
    processing_events_(false),
    motion_event_handler_(new om::Pointer(ui::EventFactoryOzoneWayland::GetInstance()->EventConverter(), handle)),
    key_event_handler_(new om::Keyboard(ui::EventFactoryOzoneWayland::GetInstance()->EventConverter(), handle)) {
  
  DCHECK(mir_connection_is_valid(connection_));

  MirSurfaceParameters parameters = {
      "Ozone", // TODO: Mir needs to support changing window title
      0, 0, // Let WM Choose initial width/height
      choose_pixel_format(connection),
      mir_buffer_usage_hardware,
      mir_display_output_id_invalid
  };
  surface_ = mir_connection_create_surface_sync(connection, &parameters);

  NotifyResize();
}

om::Window::~Window() {
  DCHECK(mir_surface_is_valid(surface_));

  mir_surface_release_sync(surface_);

  delete motion_event_handler_;
  delete key_event_handler_;
}

void om::Window::SetWindowType(ui::WidgetType type) {
  DCHECK(mir_surface_is_valid(surface_));

  switch (type)
  {
  case ui::WINDOW:
    mir_surface_set_type(surface_, mir_surface_type_normal);
    break;
  case ui::WINDOWFRAMELESS: // ?
    mir_surface_set_type(surface_, mir_surface_type_freestyle);
    break;
  case ui::POPUP:
    // TODO: ~racarr
    break;
  default:
    // TODO: ~racarr
    break;
  }
}

void om::Window::SetWindowTitle(base::string16 const& title) {
  // TODO: Mir needs to support this
}


/* TODO: Does this block of state changing need to be synchronous? ~racarr */
void om::Window::Maximize() {
  DCHECK(mir_surface_is_valid(surface_));
  mir_surface_set_state(surface_, mir_surface_state_maximized);
}

void om::Window::Minimize() {
  DCHECK(mir_surface_is_valid(surface_));
  mir_surface_set_state(surface_, mir_surface_state_minimized);
}

void om::Window::Restore() {
  DCHECK(mir_surface_is_valid(surface_));
  mir_surface_set_state(surface_, mir_surface_state_restored);
}

void om::Window::SetFullscreen() {
  DCHECK(mir_surface_is_valid(surface_));
  mir_surface_set_state(surface_, mir_surface_state_fullscreen);
}

unsigned om::Window::Handle() const {
  return handle_;
}

void om::Window::RealizeAcceleratedWidget() {
  // TODO: In Mir we do not have an explicit realize step...
}

intptr_t om::Window::egl_window() {
  DCHECK(mir_surface_is_valid(surface_));
  return reinterpret_cast<intptr_t>(mir_surface_get_egl_native_window(surface_));
}

void om::Window::Resize(unsigned width, unsigned height) {
  LOG(INFO) << __PRETTY_FUNCTION__;
  // TODO
}

gfx::Rect om::Window::GetBounds() const {
  DCHECK(mir_surface_is_valid(surface_));
  MirSurfaceParameters params;
  mir_surface_get_parameters(surface_, &params);
  // TODO: Why do we want X/Y ~racarr
  // TODO: Does chromium require any sort of guarantees on X/Y for relative windos?
  return gfx::Rect(0,0, params.width, params.height);
}

void om::Window::StartProcessingEvents() {
  processing_events_ = true;

  MirEventDelegate d = {&om::Window::HandleEvent, this};
  mir_surface_set_event_handler(surface_, &d);
}

void om::Window::StopProcessingEvents() {
  processing_events_ = false;
}

void om::Window::HandleEvent(MirSurface *surface, MirEvent const *ev, void *context) {
  om::Window *w = static_cast<om::Window*>(context);
    
  // TODO: Maybe it would be nice if Mir let us unregister the callback.
  if (w->processing_events_ == false)
    return;

  switch(ev->type)
  {
  case mir_event_type_motion:
    w->motion_event_handler_->handle_motion_event(ev->motion);
    break;
  case mir_event_type_key:
    w->key_event_handler_->handle_key_event(ev->key);
    break;
  break;
  default:
    break;
  }
}

void om::Window::NotifyResize() {
  DCHECK(mir_surface_is_valid(surface_));

  MirSurfaceParameters params;
  mir_surface_get_parameters(surface_, &params);

  ui::EventConverterOzoneWayland  *sink = ui::EventFactoryOzoneWayland::GetInstance()
      ->EventConverter();

  DCHECK(sink);

  sink->WindowResized(Handle(), params.width, params.height);
}
