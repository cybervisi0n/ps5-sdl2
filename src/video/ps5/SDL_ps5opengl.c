/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2024 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

#if defined(SDL_VIDEO_DRIVER_PS5) && defined(SDL_VIDEO_OPENGL_EGL)

/*
 * OpenGL 4.6 and OpenGL ES 3.2 on Mesa: EGL's "videoout" platform, whose
 * window surfaces are zink's kopper swapchains on RADV's VK_KHR_display.
 * There is one window surface, the whole 3840x2160 screen; any non-zero
 * native window names it.
 *
 * libEGL is static linked
 */


#include "../SDL_egl_c.h"
#include "SDL_ps5video.h"
#include "SDL_ps5opengl.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

#define PS5_DEFAULT_EGL "libEGL.so.1"

/* Any non-zero native window is the display (platform_videoout.c). */
#define PS5_EGL_NATIVE_WINDOW ((NativeWindowType)0)

static void *ps5_egl_global_handle = NULL;

static int PS5_EGL_LoadLibrary(_THIS, const char *path)
{
    
    // EGL is static linked
    _this->egl_data = (struct SDL_EGL_VideoData *)SDL_calloc(1, sizeof(SDL_EGL_VideoData));


    // Initialize egl
    _this->egl_data->eglGetDisplay = eglGetDisplay;
    //_this->egl_data->eglGetPlatformDisplay = (void*)eglGetPlatformDisplay;
    _this->egl_data->eglInitialize = eglInitialize;
    _this->egl_data->eglTerminate = eglTerminate;
    _this->egl_data->eglGetProcAddress = (void*)eglGetProcAddress;
    _this->egl_data->eglChooseConfig = eglChooseConfig;
    _this->egl_data->eglCreateContext = eglCreateContext;
    _this->egl_data->eglDestroyContext = eglDestroyContext;
    _this->egl_data->eglCreatePbufferSurface = eglCreatePbufferSurface;
    _this->egl_data->eglCreateWindowSurface = eglCreateWindowSurface;
    _this->egl_data->eglDestroySurface = eglDestroySurface;
    _this->egl_data->eglMakeCurrent = eglMakeCurrent;
    _this->egl_data->eglSwapBuffers = eglSwapBuffers;
    _this->egl_data->eglSwapInterval = eglSwapInterval;
    _this->egl_data->eglQueryString = eglQueryString;
    _this->egl_data->eglQueryAPI = eglQueryAPI;
    _this->egl_data->eglGetConfigAttrib = eglGetConfigAttrib;
    _this->egl_data->eglWaitNative = eglWaitNative;
    _this->egl_data->eglWaitGL = eglWaitGL;
    _this->egl_data->eglBindAPI = eglBindAPI;
    _this->egl_data->eglGetError = eglGetError;

    _this->egl_data->egl_display = _this->egl_data->eglGetDisplay(EGL_DEFAULT_DISPLAY);
    _this->egl_data->egl_surfacetype = EGL_WINDOW_BIT;
    eglInitialize(_this->egl_data->egl_display, NULL, NULL);



    /* EGL_DEFAULT_DISPLAY is the videoout platform, whatever Mesa's
       default native platform was built as. */
    SDL_setenv("EGL_PLATFORM", "videoout", 0);

    /* Mesa opens VideoOut for its swapchains: SDL must have let it go. */
    PS5_VideoOut_Release(_this);

    PS5_UseGPUOutput(_this);

    return 0;
}

int PS5_EGL_CreateWindowSurface(_THIS, SDL_Window *window)
{
    SDL_WindowData *data = (SDL_WindowData *)window->driverdata;
    EGLSurface surface;

    surface = SDL_EGL_CreateSurface(_this, PS5_EGL_NATIVE_WINDOW);
    if (surface == EGL_NO_SURFACE) {
        return -1; /* SDL_EGL_CreateSurface has set the error */
    }

    data->egl_surface = surface;

    return 0;
}

void PS5_EGL_DestroyWindowSurface(_THIS, SDL_Window *window)
{
    SDL_WindowData *data = (SDL_WindowData *)window->driverdata;

    if (data && data->egl_surface != EGL_NO_SURFACE) {
        SDL_EGL_DestroySurface(_this, data->egl_surface);
        data->egl_surface = EGL_NO_SURFACE;
    }
}

static SDL_GLContext PS5_EGL_CreateContext(_THIS, SDL_Window *window)
{
    SDL_WindowData *data = (SDL_WindowData *)window->driverdata;

    if (data->egl_surface == EGL_NO_SURFACE) {
        SDL_SetError("The window was not created with SDL_WINDOW_OPENGL");
        return NULL;
    }

    return SDL_EGL_CreateContext(_this, data->egl_surface);
}

static int PS5_EGL_MakeCurrent(_THIS, SDL_Window *window, SDL_GLContext context)
{
    EGLSurface surface = EGL_NO_SURFACE;

    if (window && context) {
        surface = ((SDL_WindowData *)window->driverdata)->egl_surface;
    }

    return SDL_EGL_MakeCurrent(_this, surface, context);
}

static int PS5_EGL_SwapWindow(_THIS, SDL_Window *window)
{
    return SDL_EGL_SwapBuffers(_this, ((SDL_WindowData *)window->driverdata)->egl_surface);
}

int PS5_GL_InitDevice(SDL_VideoDevice *device)
{
    device->GL_LoadLibrary = PS5_EGL_LoadLibrary;
    device->GL_GetProcAddress = SDL_EGL_GetProcAddress;
    device->GL_UnloadLibrary = SDL_EGL_UnloadLibrary;
    device->GL_CreateContext = PS5_EGL_CreateContext;
    device->GL_MakeCurrent = PS5_EGL_MakeCurrent;
    device->GL_SetSwapInterval = SDL_EGL_SetSwapInterval;
    device->GL_GetSwapInterval = SDL_EGL_GetSwapInterval;
    device->GL_SwapWindow = PS5_EGL_SwapWindow;
    device->GL_DeleteContext = SDL_EGL_DeleteContext;

    return 0;
}

#endif /* SDL_VIDEO_DRIVER_PS5 && SDL_VIDEO_OPENGL_EGL */

/* vi: set ts=4 sw=4 expandtab: */
