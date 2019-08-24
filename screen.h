#ifndef __SCREEN_H__
#define __SCREEN_H__


#ifndef MAX_LOST_FPS
#define MAX_LOST_FPS 5
#endif


#include <pthread.h>
#include <raylib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "log.h"
#include "config.h"
#include "main.h"


typedef enum {
	SCREEN_TEXT,
	SCREEN_CLOCK,
	SCREEN_IMG,
} screen_element_type;

typedef enum {
	ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT,
	ALIGN_TOP,
	ALIGN_MIDDLE,
	ALIGN_BOTTOM,
} screen_align;

typedef enum {
	RESIZE_PROPER,
	RESIZE_CROP,
	RESIZE_STRETCH,
} screen_resize;


typedef struct screen_position {
	uint_fast16_t x, y, w, h;
	screen_align horizontal, vertical;
} screen_position;

typedef struct screen_attrs_text {
	uint_fast16_t font_size;
	uint_fast16_t font_id;
	char *font_name;
	char *text;
	Color color;
} screen_attrs_text;

typedef struct screen_attrs_img {
	Image image;
	screen_resize resize_type;
	Color background_color;
	char *file_name;
	Texture2D *texture;
} screen_attrs_img;

typedef struct screen_evals {
	char *lua_script;
	lua_State *lua_state;
} screen_evals;

typedef struct screen_element {
	uint_fast32_t id;
	screen_element_type type;
	screen_position position;
	void *attrs;
	void *evals;
} screen_element;

typedef struct type_frame {
	char *name;
	uint_fast32_t element_id;  // UINT_FAST32_MAX for slides or dummy data
	screen_position position;
} type_frame;


struct timeval screen_update_start;
Color screen_background_color;
Color screen_default_color;


void *screen(void *_);
uint_fast16_t screen_add_clock(const screen_position position, char *format, const uint_fast16_t font_size, const char *font, const Color color);
uint_fast16_t screen_add_text(const screen_position position, const char *text, const uint_fast16_t font_size, const char *font, Color color);
void screen_remove_element(uint_fast16_t element_id);
screen_attrs_img *screen_prepare_image(screen_position *position, screen_resize resize_type, char *file, Color *background_color);
uint_fast16_t screen_add_img(const screen_position position, const screen_attrs_img img);


#endif
