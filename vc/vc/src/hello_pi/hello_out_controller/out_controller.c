#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include "VG/openvg.h"
#include "VG/vgu.h"
#include <EGL/egl.h>

#include "bcm_host.h"

#define WIDTH 500
#define HEIGHT 400

typedef struct
{
	// Screen dimentions
	int32_t screen_width;
	uint32_t screen_height;
	// Window dimentions
	int32_t window_x;
	int32_t window_y;
	uint32_t window_width;
	uint32_t window_height;
	// dispman window 
	DISPMANX_ELEMENT_HANDLE_T element;
					
	// EGL data
	EGLDisplay display;
						
	EGLSurface surface;
	EGLContext context;

} STATE_T;

static STATE_T _state, *state = &_state;
static int init_x = 0;		// Initial window position and size
static int init_y = 0;
static unsigned int init_w = 0;
static unsigned int init_h = 0;

// setWindowParams sets the window's position, adjusting if need be to
// prevent it from going fully off screen. Also sets the dispman rects
// for displaying.
static void setWindowParams(STATE_T * state, int x, int y, VC_RECT_T * src_rect, VC_RECT_T * dst_rect) {
	uint32_t dx, dy, w, h, sx, sy;

	// Set source & destination rectangles so that the image is
	// clipped if it goes off screen (else dispman won't show it properly)
	if (x < (1 - (int)state->window_width)) {	   // Too far off left
		x = 1 - (int)state->window_width;
		dx = 0;
		sx = state->window_width - 1;
		w = 1;
	} else if (x < 0) {				   // Part of left is off
		dx = 0;
		sx = -x;
		w = state->window_width - sx;
	} else if (x < (state->screen_width - state->window_width)) {	// On
		dx = x;
		sx = 0;
		w = state->window_width;
	} else if (x < state->screen_width) {		   // Part of right is off
		dx = x;
		sx = 0;
		w = state->screen_width - x;
	} else {					   // Too far off right
		x = state->screen_width - 1;
		dx = state->screen_width - 1;
		sx = 0;
		w = 1;
	}

	if (y < (1 - (int)state->window_height)) {	   // Too far off top
		y = 1 - (int)state->window_height;
		dy = 0;
		sy = state->window_height - 1;
		h = 1;
	} else if (y < 0) {				   // Part of top is off
		dy = 0;
		sy = -y;
		h = state->window_height - sy;
	} else if (y < (state->screen_height - state->window_height)) {	// On
		dy = y;
		sy = 0;
		h = state->window_height;
	} else if (y < state->screen_height) {		   // Part of bottom is off
		dy = y;
		sy = 0;
		h = state->screen_height - y;
	} else {					   // Wholly off bottom
		y = state->screen_height - 1;
		dy = state->screen_height - 1;
		sy = 0;
		h = 1;
	}

	state->window_x = x;
	state->window_y = y;

	vc_dispmanx_rect_set(dst_rect, dx, dy, w, h);
	vc_dispmanx_rect_set(src_rect, sx << 16, sy << 16, w << 16, h << 16);
}

void oglinit() {
	int32_t success = 0;
	EGLBoolean result;
	EGLint num_config;

	static EGL_DISPMANX_WINDOW_T nativewindow;

	DISPMANX_ELEMENT_HANDLE_T dispman_element;
	DISPMANX_DISPLAY_HANDLE_T dispman_display;
	DISPMANX_UPDATE_HANDLE_T dispman_update;
	VC_RECT_T dst_rect;
	VC_RECT_T src_rect;
	static VC_DISPMANX_ALPHA_T alpha = {
									DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS,
									255, 0
									};

	static const EGLint attribute_list[] = {
									EGL_RED_SIZE, 8,
									EGL_GREEN_SIZE, 8,
									EGL_BLUE_SIZE, 8,
									EGL_ALPHA_SIZE, 8,
									EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
									EGL_NONE
									};

	EGLConfig config;

	// get an EGL display connection
	state->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(state->display != EGL_NO_DISPLAY);

	// initialize the EGL display connect
	result = eglInitialize(state->display, NULL, NULL);
	assert(EGL_FALSE != result);

	// bind OpenVG API
	eglBindAPI(EGL_OPENVG_API);

	// get an appropriate EGL frame buffer configuration
	result = eglChooseConfig(state->display, attribute_list, &config, 1, &num_config);
	assert(EGL_FALSE != result);

	// create an EGL rendering context
	state->context = eglCreateContext(state->display, config, EGL_NO_CONTEXT, NULL);
	assert(state->context != EGL_NO_CONTEXT);

	// create an EGL window surface
	success = graphics_get_display_size(0 /* LCD */ , &state->screen_width,&state->screen_height);
	assert(success >= 0);

	state->window_width = 500;
	state->window_height = 400;
	state->window_x = 710;
	state->window_y = 340;

	if ((state->window_width == 0) || (state->window_width > state->screen_width))
		state->window_width = state->screen_width;
	if ((state->window_height == 0) || (state->window_height > state->screen_height))
		state->window_height = state->screen_height;
	// adjust position so that at least one pixel is on screen and
	// set up the dispman rects
	setWindowParams(state, state->window_x, state->window_y, &src_rect, &dst_rect);

	dispman_display = vc_dispmanx_display_open(0 /* LCD */ );
	dispman_update = vc_dispmanx_update_start(0);

	dispman_element = vc_dispmanx_element_add(dispman_update, dispman_display, 10 /*layer */ , &dst_rect, 0 /*src */ ,
											&src_rect, DISPMANX_PROTECTION_NONE, &alpha, 0 /*clamp */ ,
											0 /*transform */ );

  	state->element = dispman_element;
	nativewindow.element = dispman_element;
	nativewindow.width = state->window_width;
	nativewindow.height = state->window_height;
	vc_dispmanx_update_submit_sync(dispman_update);

	state->surface = eglCreateWindowSurface(state->display, config, &nativewindow, NULL);
	assert(state->surface != EGL_NO_SURFACE);

	// preserve the buffers on swap
	result = eglSurfaceAttrib(state->display, state->surface, EGL_SWAP_BEHAVIOR, EGL_BUFFER_PRESERVED);
	assert(EGL_FALSE != result);

	// connect the context to the surface
	result = eglMakeCurrent(state->display, state->surface, state->surface, state->context);
	assert(EGL_FALSE != result);
}

// RGBA fills a color vectors from a RGBA quad.
void RGBA(unsigned int r, unsigned int g, unsigned int b, VGfloat a, VGfloat color[4]) {
	if (r > 255) {
		r = 0;
	}
	if (g > 255) {
		g = 0;
	}
	if (b > 255) {
		b = 0;
	}
	if (a < 0.0 || a > 1.0) {
		a = 1.0;
	}
	color[0] = (VGfloat) r / 255.0f;
	color[1] = (VGfloat) g / 255.0f;
	color[2] = (VGfloat) b / 255.0f;
	color[3] = a;
}

// setfill sets the fill color
void setfill(VGfloat color[4]) {
	VGPaint fillPaint = vgCreatePaint();
	vgSetParameteri(fillPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
	vgSetParameterfv(fillPaint, VG_PAINT_COLOR, 4, color);
	vgSetPaint(fillPaint, VG_FILL_PATH);
	vgDestroyPaint(fillPaint);
}

int main(void)
{
	//VC_RECT_T			src_rect;
	//VC_RECT_T			dst_rect;
	//VC_IMAGE_TYPE_T type = VC_IMAGE_RGB565;

	//int width = WIDTH, height = HEIGHT;

	//VC_DISPMANX_ALPHA_T alpha = { DISPMANX_FLAGS_ALPHA_FROM_SOURCE | DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS, 
	//	                         	120, /*alpha 0->255*/
	//								0 };
	
	
	bcm_host_init();
	memset(state, 0, sizeof(*state));
	state->window_x = init_x;
	state->window_y = init_y;
	state->window_width = init_w;
	state->window_height = init_h;

	oglinit();

	VGPath path = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 0, 0, VG_PATH_CAPABILITY_APPEND_TO);

	VGfloat color[4];
	RGBA(255, 0, 0, 1, color);
	setfill(color);
	
	vguRect(path, 0, 0, 20, 20);
	vgDrawPath(path, VG_FILL_PATH | VG_STROKE_PATH);
	vgDestroyPath(path);

	assert(vgGetError() == VG_NO_ERROR);
	eglSwapBuffers(state->display, state->surface);
	assert(eglGetError() == EGL_SUCCESS);

	sleep(10);
}
