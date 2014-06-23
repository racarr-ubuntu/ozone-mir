// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/display.h"

#include <EGL/egl.h>
#include <string>

#include "base/files/file_path.h"
#include "base/native_library.h"
#include "base/stl_util.h"
#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/ui/events/output_change_observer.h"
#include "ozone/ui/gfx/ozone_display.h"
#include "ozone/wayland/display_poll_thread.h"
#include "ozone/wayland/egl/surface_ozone_wayland.h"
#include "ozone/wayland/input/cursor.h"
#include "ozone/wayland/input_device.h"
#include "ozone/wayland/screen.h"
#include "ozone/wayland/shell/shell.h"
#include "ozone/wayland/window.h"
#include "ui/gfx/ozone/surface_ozone_egl.h"

namespace ozonewayland {
WaylandDisplay* WaylandDisplay::instance_ = NULL;

WaylandDisplay::WaylandDisplay() : display_(NULL),
    registry_(NULL),
    compositor_(NULL),
    shell_(NULL),
    shm_(NULL),
    primary_screen_(NULL),
    primary_input_(NULL),
    display_poll_thread_(NULL),
    screen_list_(),
    input_list_(),
    widget_map_(),
    serial_(0),
    processing_events_(false) {
}

WaylandDisplay::~WaylandDisplay() {
  Terminate();
}

const std::list<WaylandScreen*>& WaylandDisplay::GetScreenList() const {
  return screen_list_;
}

WaylandWindow* WaylandDisplay::GetWindow(unsigned window_handle) const {
  return GetWidget(window_handle);
}

struct wl_text_input_manager* WaylandDisplay::GetTextInputManager() const {
  return text_input_manager_;
}

void WaylandDisplay::SyncDisplay() {
  wl_display_roundtrip(display_);
}

void WaylandDisplay::FlushDisplay() {
  wl_display_flush(display_);
}

void WaylandDisplay::DestroyWindow(unsigned w) {
  std::map<unsigned, WaylandWindow*>::const_iterator it = widget_map_.find(w);
  WaylandWindow* widget = it == widget_map_.end() ? NULL : it->second;
  DCHECK(widget);
  delete widget;
  widget_map_.erase(w);
  if (widget_map_.empty())
    StopProcessingEvents();
}

gfx::AcceleratedWidget WaylandDisplay::GetNativeWindow(unsigned window_handle) {
  // Ensure we are processing wayland event requests.
  StartProcessingEvents();
  WaylandWindow* widget = GetWidget(window_handle);
  DCHECK(widget);
  widget->RealizeAcceleratedWidget();

  return (gfx::AcceleratedWidget)widget->egl_window();
}

gfx::SurfaceFactoryOzone::HardwareState
WaylandDisplay::InitializeHardware() {
  InitializeDisplay();
  if (!display_) {
    LOG(ERROR) << "WaylandDisplay failed to initialize hardware";
    return gfx::SurfaceFactoryOzone::FAILED;
  }

  return gfx::SurfaceFactoryOzone::INITIALIZED;
}

void WaylandDisplay::ShutdownHardware() {
  Terminate();
}

intptr_t WaylandDisplay::GetNativeDisplay() {
  return (intptr_t)display_;
}

gfx::AcceleratedWidget WaylandDisplay::GetAcceleratedWidget() {
  static int opaque_handle = 0;
  opaque_handle++;
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(opaque_handle,
                                                              ui::CREATE,
                                                              0,
                                                              0);

  return (gfx::AcceleratedWidget)opaque_handle;
}

scoped_ptr<gfx::SurfaceOzoneEGL> WaylandDisplay::CreateEGLSurfaceForWidget(
    gfx::AcceleratedWidget w) {
  return make_scoped_ptr<gfx::SurfaceOzoneEGL>(new SurfaceOzoneWayland(w));
}

bool WaylandDisplay::LoadEGLGLES2Bindings(
    gfx::SurfaceFactoryOzone::AddGLLibraryCallback add_gl_library,
    gfx::SurfaceFactoryOzone::SetGLGetProcAddressProcCallback setprocaddress) {
  // The variable EGL_PLATFORM specifies native platform to be used by the
  // drivers (atleast on Mesa). When the variable is not set, Mesa uses the
  // first platform listed in --with-egl-platforms during compilation. Thus, we
  // ensure here that wayland is set as the native platform. However, we don't
  // override the EGL_PLATFORM value in case it has already been set.
  setenv("EGL_PLATFORM", "wayland", 0);
  base::NativeLibraryLoadError error;
  base::NativeLibrary gles_library = base::LoadNativeLibrary(
    base::FilePath("libGLESv2.so.2"), &error);

  if (!gles_library) {
    LOG(WARNING) << "Failed to load GLES library: " << error.ToString();
    return false;
  }

  base::NativeLibrary egl_library = base::LoadNativeLibrary(
    base::FilePath("libEGL.so.1"), &error);

  if (!egl_library) {
    LOG(WARNING) << "Failed to load EGL library: " << error.ToString();
    base::UnloadNativeLibrary(gles_library);
    return false;
  }

  GLGetProcAddressProc get_proc_address =
      reinterpret_cast<GLGetProcAddressProc>(
          base::GetFunctionPointerFromNativeLibrary(
              egl_library, "eglGetProcAddress"));

  if (!get_proc_address) {
    LOG(ERROR) << "eglGetProcAddress not found.";
    base::UnloadNativeLibrary(egl_library);
    base::UnloadNativeLibrary(gles_library);
    return false;
  }

  setprocaddress.Run(get_proc_address);
  add_gl_library.Run(egl_library);
  add_gl_library.Run(gles_library);
  return true;
}

const int32*
WaylandDisplay::GetEGLSurfaceProperties(const int32* desired_list) {
  static const EGLint kConfigAttribs[] = {
    EGL_BUFFER_SIZE, 32,
    EGL_ALPHA_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_NONE
  };

  return kConfigAttribs;
}

void WaylandDisplay::SetWidgetState(unsigned w,
                                    ui::WidgetState state,
                                    unsigned width,
                                    unsigned height) {
  switch (state) {
    case ui::CREATE:
    {
      CreateAcceleratedSurface(w);
      break;
    }
    case ui::FULLSCREEN:
    {
      WaylandWindow* widget = GetWidget(w);
      widget->SetFullscreen();
      widget->Resize(width, height);
      break;
    }
    case ui::MAXIMIZED:
    {
      WaylandWindow* widget = GetWidget(w);
      widget->Maximize();
      break;
    }
    case ui::MINIMIZED:
    {
      WaylandWindow* widget = GetWidget(w);
      widget->Minimize();
      break;
    }
    case ui::RESTORE:
    {
      WaylandWindow* widget = GetWidget(w);
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
    default:
      break;
  }
}

void WaylandDisplay::SetWidgetTitle(unsigned w,
                                    const base::string16& title) {
  WaylandWindow* widget = GetWidget(w);
  DCHECK(widget);
  widget->SetWindowTitle(title);
}

void WaylandDisplay::SetWidgetCursor(int cursor_type) {
  primary_input_->SetCursorType(cursor_type);
}

void WaylandDisplay::SetWidgetAttributes(unsigned widget,
                                         unsigned parent,
                                         unsigned x,
                                         unsigned y,
                                         ui::WidgetType type) {
  WaylandWindow* window = GetWidget(widget);
  WaylandWindow* parent_window = GetWidget(parent);
  DCHECK(window);
  switch (type) {
  case ui::WINDOW:
    window->SetShellAttributes(WaylandWindow::TOPLEVEL);
    break;
  case ui::WINDOWFRAMELESS:
    NOTIMPLEMENTED();
    break;
  case ui::POPUP:
    DCHECK(parent_window);
    window->SetShellAttributes(WaylandWindow::POPUP,
                               parent_window->ShellSurface(),
                               x,
                               y);
    break;
  default:
    break;
  }
}

void WaylandDisplay::InitializeDisplay() {
  DCHECK(!display_);
  display_ = wl_display_connect(NULL);
  if (!display_)
    return;

  instance_ = this;
  static const struct wl_registry_listener registry_all = {
    WaylandDisplay::DisplayHandleGlobal
  };

  registry_ = wl_display_get_registry(display_);
  wl_registry_add_listener(registry_, &registry_all, this);
  shell_ = new WaylandShell();

  if (wl_display_roundtrip(display_) < 0) {
    Terminate();
    return;
  }

  ui::WindowStateChangeHandler::SetInstance(this);
  display_poll_thread_ = new WaylandDisplayPollThread(display_);
}

WaylandWindow* WaylandDisplay::CreateAcceleratedSurface(unsigned w) {
  WaylandWindow* window = new WaylandWindow(w);
  widget_map_[w] = window;

  return window;
}

void WaylandDisplay::StartProcessingEvents() {
  DCHECK(display_poll_thread_);
  // Start polling for wayland events.
  if (!processing_events_) {
    display_poll_thread_->StartProcessingEvents();
    processing_events_ = true;
  }
}

void WaylandDisplay::StopProcessingEvents() {
  DCHECK(display_poll_thread_);
  // Start polling for wayland events.
  if (processing_events_) {
    display_poll_thread_->StopProcessingEvents();
    processing_events_ = false;
  }
}

void WaylandDisplay::Terminate() {
  if (!widget_map_.empty()) {
    STLDeleteValues(&widget_map_);
    widget_map_.clear();
  }

  for (std::list<WaylandInputDevice*>::iterator i = input_list_.begin();
      i != input_list_.end(); ++i) {
      delete *i;
  }

  for (std::list<WaylandScreen*>::iterator i = screen_list_.begin();
      i != screen_list_.end(); ++i) {
      delete *i;
  }

  screen_list_.clear();
  input_list_.clear();

  WaylandCursor::Clear();

  if (compositor_)
    wl_compositor_destroy(compositor_);

  delete shell_;
  if (shm_)
    wl_shm_destroy(shm_);

  if (registry_)
    wl_registry_destroy(registry_);

  delete display_poll_thread_;

  if (display_) {
    wl_display_flush(display_);
    wl_display_disconnect(display_);
    display_ = NULL;
  }

  instance_ = NULL;
}

WaylandWindow* WaylandDisplay::GetWidget(unsigned w) const {
  std::map<unsigned, WaylandWindow*>::const_iterator it = widget_map_.find(w);
  return it == widget_map_.end() ? NULL : it->second;
}


// static
void WaylandDisplay::DisplayHandleGlobal(void *data,
    struct wl_registry *registry,
    uint32_t name,
    const char *interface,
    uint32_t version) {

  WaylandDisplay* disp = static_cast<WaylandDisplay*>(data);

  if (strcmp(interface, "wl_compositor") == 0) {
    disp->compositor_ = static_cast<wl_compositor*>(
        wl_registry_bind(registry, name, &wl_compositor_interface, 1));
  } else if (strcmp(interface, "wl_output") == 0) {
    WaylandScreen* screen = new WaylandScreen(disp->registry(), name);
    if (!disp->screen_list_.empty())
      NOTIMPLEMENTED() << "Multiple screens support is not implemented";

    disp->screen_list_.push_back(screen);
    // (kalyan) Support extended output.
    disp->primary_screen_ = disp->screen_list_.front();
  } else if (strcmp(interface, "wl_seat") == 0) {
    WaylandInputDevice *input_device = new WaylandInputDevice(disp, name);
    disp->input_list_.push_back(input_device);
    disp->primary_input_ = disp->input_list_.front();
  } else if (strcmp(interface, "wl_shm") == 0) {
    disp->shm_ = static_cast<wl_shm*>(
        wl_registry_bind(registry, name, &wl_shm_interface, 1));
  } else if (strcmp(interface, "wl_text_input_manager") == 0) {
    disp->text_input_manager_ = static_cast<wl_text_input_manager*>(
        wl_registry_bind(registry, name, &wl_text_input_manager_interface, 1));
  } else {
    disp->shell_->Initialize(registry, name, interface, version);
  }
}

}  // namespace ozonewayland

char const* gfx::OzoneDisplay::GetEGLPlatform() {
  return "wayland";
}
