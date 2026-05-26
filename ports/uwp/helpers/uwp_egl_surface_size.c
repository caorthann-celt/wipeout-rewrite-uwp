#include "uwp_egl_surface_size.h"

#include "SDL_loadso.h"

bool uwp_egl_surface_size(int *width, int *height) {
	typedef unsigned int EGLBoolean;
	typedef void *EGLDisplay;
	typedef int EGLint;
	typedef void *EGLSurface;

	enum {
		EGL_WIDTH = 0x3057,
		EGL_HEIGHT = 0x3056,
		EGL_DRAW = 0x3059
	};

	typedef EGLDisplay (*egl_get_current_display_func)(void);
	typedef EGLSurface (*egl_get_current_surface_func)(EGLint);
	typedef EGLBoolean (*egl_query_surface_func)(EGLDisplay, EGLSurface, EGLint, EGLint *);

	static void *lib_egl = NULL;
	if (!lib_egl) {
		lib_egl = SDL_LoadObject("libEGL.dll");
	}
	if (!lib_egl) {
		return false;
	}

	egl_get_current_display_func egl_get_current_display =
		(egl_get_current_display_func)SDL_LoadFunction(lib_egl, "eglGetCurrentDisplay");
	egl_get_current_surface_func egl_get_current_surface =
		(egl_get_current_surface_func)SDL_LoadFunction(lib_egl, "eglGetCurrentSurface");
	egl_query_surface_func egl_query_surface =
		(egl_query_surface_func)SDL_LoadFunction(lib_egl, "eglQuerySurface");

	if (!egl_get_current_display || !egl_get_current_surface || !egl_query_surface) {
		return false;
	}

	EGLDisplay display = egl_get_current_display();
	EGLSurface surface = egl_get_current_surface(EGL_DRAW);
	if (!display || !surface) {
		return false;
	}

	EGLint egl_width = 0;
	EGLint egl_height = 0;
	if (
		!egl_query_surface(display, surface, EGL_WIDTH, &egl_width) ||
		!egl_query_surface(display, surface, EGL_HEIGHT, &egl_height) ||
		egl_width <= 0 ||
		egl_height <= 0
	) {
		return false;
	}

	*width = egl_width;
	*height = egl_height;
	return true;
}
