#include "screen.h"

static const char *TOPIC = "screen";


typedef struct screen_font {
	Font font;
	char *name;
	uint_fast16_t font_size;
} screen_font;


static screen_font *screen_fonts;
static uint_fast16_t screen_fonts_count;

static screen_element *screen_elements;
static uint_fast32_t screen_elements_count;


/*******************************************************************************
 * Returns next free element id available
 * @return free element id, UINT_FAST32_MAX if no element id is available
 ******************************************************************************/
static uint_fast32_t get_free_element_id() {
	bool unique;
	for( uint_fast32_t id = 0; id < screen_elements_count; id++ ) {
		unique = true;
		for( uint_fast32_t i = 0; i < screen_elements_count - 1; i++ ) {
			if( screen_elements[i].id == id ) {
				unique = false;
				break;
			}
		}
		if( unique ) {
			return id;
		}
	}
	LOG_WARNING("No free element ID aviable");
	return UINT_FAST32_MAX;
}

/*******************************************************************************
 * Load front into GPU memory if not loaded
 * @param *name Name of the font to be loaded
 * @param font_size max size of font to loaded
 * @return the index of the loaded font
 ******************************************************************************/
static uint_fast16_t load_font(const char *name, const uint_fast16_t font_size) {
	for( uint_fast16_t i = 0; i < screen_fonts_count; i++ ) {
		if( font_size == screen_fonts[i].font_size && strcmp(name, screen_fonts[i].name ) == 0 ) {
			LOG_VERBOSE("Font »%s:%lu« already loaded", name, font_size);
			return i;
		}
	}
	LOG_VERBOSE("Load font: %s:%lu", name, font_size);
	REALLOC(screen_fonts_new, screen_fonts, sizeof(screen_font) * ++screen_fonts_count);
	screen_fonts[screen_fonts_count-1].font_size = font_size;
	screen_fonts[screen_fonts_count-1].name = strdup(name);
	FAIL_ON_NULL(screen_fonts[screen_fonts_count-1].name, "Failed to copy font name »%s« while loading font into vRAM", name);
	screen_fonts[screen_fonts_count-1].font = LoadFontEx(name, font_size, 0, 0xff);
	LOG_DEBUG("Loaded font »%s:%lu« with id %lu", screen_fonts[screen_fonts_count-1].name, screen_fonts[screen_fonts_count-1].font_size, screen_fonts_count-1);
	return screen_fonts_count-1;
}

static void free_text_attrs(screen_attrs_text *attrs) {
	free(attrs->text);
	if( attrs->font_name ) {
		free(attrs->font_name);
	}
}

/*******************************************************************************
 * Removes element from screen and free it's memory
 * @param element_id element to remove
 ******************************************************************************/
void screen_remove_element(uint_fast16_t element_id) {
	switch( screen_elements[element_id].type ) {
		case SCREEN_CLOCK:
		case SCREEN_TEXT:
			free_text_attrs(screen_elements[element_id].attrs);
			free(screen_elements[element_id].attrs);
			break;
		default:
			LOG_FATAL("Failed to remove unknown element type %d from screen elements", screen_elements[element_id].type);
	}
	//free(&screen_elements[element_id]);
	for( uint_fast16_t i = element_id; i < screen_elements_count - 1; i++ ) {
		screen_elements[i] = screen_elements[i+1];
	}
	screen_elements_count--;
	REALLOC(new_screen_elements, screen_elements, sizeof(screen_element) * screen_elements_count);
}

void screen_resize_image(Image *image, screen_resize resize_method, uint_fast16_t max_w, uint_fast16_t max_h) {
	LOG_VERBOSE("Resize image in box (w:%lu,h:%lu)", max_w, max_h);

	float factor = 1;
	switch( resize_method ) {
		case RESIZE_PROPER:
			if( max_w < max_h ) {
				factor = (float)max_w / (float)image->width;
			}	
			else {
				factor = (float)max_h / (float)image->height;
			}
			LOG_VERBOSE("Rescale factor is: %f", factor);

			ImageResize(image, (int)((float)image->width*factor), (int)((float)image->height*factor));
			break;
		case RESIZE_STRETCH:
			ImageResize(image, max_w, max_h);
			break;
		case RESIZE_CROP:
			ImageResizeCanvas(image, (image->width>=max_w)?max_w:image->width, (image->height>=max_h)?max_h:image->height, 0, 0, (Color){0,0,0,0});
			break;
		default:
			LOG_FATAL("NYI");
	}
}

/*******************************************************************************
 * Prepares an image file to be displayed
 * This function could be called from a diffrent thread and does all the resize
 * and alignment jobs
 *
 * TODO: error check file
 * TODO: gradient
 ******************************************************************************/
screen_attrs_img *screen_prepare_image(screen_position *position, screen_resize resize_type, char *file, Color *background_color) {
	LOG_VERBOSE("prepare image »%s«", file);

	// fail if not possible
	if( file == NULL && ( position->w <= 0 || position->h <= 0 ) ) {
		LOG_ERROR("Can't prepare image, no file is specified or no size is specified");
		return NULL;
	}

	screen_attrs_img *attrs;
	MALLOC(attrs, sizeof(screen_attrs_img));

	attrs->texture = NULL;

	uint_fast16_t w = 0, h = 0;
	Image img_src;
	if( file != NULL ) {
		img_src = LoadImage(file);
		UINT_FAST16_T(w, img_src.width);
		UINT_FAST16_T(h, img_src.height);
	}
	w = ( position->w >= 0 )?position->w:w;
	h = ( position->h >= 0 )?position->h:h;
	position->w = w;
	position->h = h;

	screen_resize_image(&img_src, resize_type, w, h);

	if( background_color == NULL) {
		attrs->background_color = (Color){0,0,0,0};
	}
	else {
		attrs->background_color = (Color){background_color->r, background_color->g, background_color->b, background_color->a};
	}

	attrs->image = GenImageColor(position->w, position->h, attrs->background_color);

	uint_fast16_t x = 0, y = 0;
	switch( position->horizontal ) {
		case ALIGN_RIGHT:
			x = w - img_src.width;
			break;
		case ALIGN_CENTER:
		default:
			x = ( w - img_src.width ) / 2;
			break;
	}
	switch( position->vertical ) {
		case ALIGN_BOTTOM:
			y = h - img_src.height;
			break;
		case ALIGN_CENTER:
		default:
			y = ( h - img_src.height ) / 2;
			break;
	}
	ImageDraw(&attrs->image, img_src, (Rectangle){0,0,img_src.width,img_src.height}, (Rectangle){x,y,img_src.width,img_src.height});

	return attrs;
}

uint_fast16_t screen_add_img(const screen_position position, const screen_attrs_img attr_img_in, char *lua_script) {
	screen_elements_count++;
	REALLOC(new_screen_elements, screen_elements, sizeof(screen_element) * screen_elements_count);

	screen_attrs_img *attr_img;
	MALLOC(attr_img, sizeof(screen_attrs_img)); //TODO free

	attr_img->texture = attr_img_in.texture;

	attr_img->background_color = attr_img_in.background_color;
	if( attr_img_in.file_name != NULL ) {
		attr_img->file_name = strdup(attr_img_in.file_name);
		FAIL_ON_NULL(attr_img->file_name, "Failed to copy file name, while adding image to screen elements");
	}
	attr_img->resize_type = attr_img_in.resize_type;
	attr_img->image = ImageCopy(attr_img_in.image);

	if( lua_script != NULL ) {
		MALLOC(screen_elements[screen_elements_count-1].evals, sizeof(screen_evals));
		screen_elements[screen_elements_count-1].evals->lua_state = NULL;
		screen_elements[screen_elements_count-1].evals->lua_script = lua_script;
	}
	else {
		screen_elements[screen_elements_count-1].evals = NULL;
	}
	screen_elements[screen_elements_count-1].position = position;
	screen_elements[screen_elements_count-1].type = SCREEN_IMG;
	screen_elements[screen_elements_count-1].id = get_free_element_id();
	screen_elements[screen_elements_count-1].attrs = attr_img;

	LOG_DEBUG("Added screen_element %lu with id %lu", screen_elements_count, screen_elements[screen_elements_count-1].id);
	return screen_elements[screen_elements_count-1].id;
}

/*******************************************************************************
 * Add text to screen elements
 * @param position position and size of the text to add to the screen
 * @text text to draw
 * @font_size font size to use
 * @font name of the font to used
 * @color the color which should be used
 * @return the unique id of the element, can be later used to remove it
 ******************************************************************************/
uint_fast16_t screen_add_text(const screen_position position, const char *text, const uint_fast16_t font_size, const char *font, const Color color, char *lua_script) {
	screen_elements_count++;
	REALLOC(new_screen_elements, screen_elements, sizeof(screen_element) * screen_elements_count);

	screen_attrs_text *attr_text;
	MALLOC(attr_text, sizeof(screen_attrs_text));

	if( font_size == 0 ) { attr_text->font_size = 10; }
	else { attr_text->font_size = font_size; }

	if( font == NULL ) {
		attr_text->font_id = UINT_FAST16_MAX;
		attr_text->font_name = NULL;
	}
	else {
		attr_text->font_id = UINT_FAST16_MAX;
		attr_text->font_name = strdup(font);
		FAIL_ON_NULL(attr_text->font_name, "Failed to copy font name, while adding text to screen elements");
	}

	attr_text->color = color;
	attr_text->text = strdup(text);
	FAIL_ON_NULL(attr_text->text, "Failed to copy text, while adding text to screen elements");

	if( lua_script != NULL ) {
		MALLOC(screen_elements[screen_elements_count-1].evals, sizeof(screen_evals));
		screen_elements[screen_elements_count-1].evals->lua_state = NULL;
		screen_elements[screen_elements_count-1].evals->lua_script = lua_script;
	}
	else {
		screen_elements[screen_elements_count-1].evals = NULL;
	}
	screen_elements[screen_elements_count-1].position = position;
	screen_elements[screen_elements_count-1].type = SCREEN_TEXT;
	screen_elements[screen_elements_count-1].id = get_free_element_id();
	screen_elements[screen_elements_count-1].attrs = attr_text;

	LOG_DEBUG("Added screen_element %lu with id %lu", screen_elements_count, screen_elements[screen_elements_count-1].id);
	return screen_elements[screen_elements_count-1].id;
}

/*******************************************************************************
 * Add a clock to screen elements
 * @param position position and size of the text to add to the screen
 * @param format the strftime format string to used, defaults to %H:%M
 * @font_size font size to use
 * @font name of the font to used
 * @color the color which should be used
 * @return the unique id of the element, can be later used to remove it
 ******************************************************************************/
uint_fast16_t screen_add_clock(const screen_position position, char *format, const uint_fast16_t font_size, const char *font, const Color color, char *lua_script) {
	if( format == NULL ) {
		format = strdup("%H:%M");
	}
	uint_fast16_t id = screen_add_text(position, format, font_size, font, color, lua_script);
	screen_elements[screen_elements_count-1].type = SCREEN_CLOCK;
	return id;
}

/*******************************************************************************
 * Draw text from element on screen
 * @element the element to beed drawn
 ******************************************************************************/
static void draw_text(const screen_element *element) {
	screen_attrs_text *attr_text = (screen_attrs_text *)element->attrs;

	Vector2 text_size;
	uint_fast16_t x, y;
	if( attr_text->font_name == NULL ) {
		text_size.x = MeasureText(attr_text->text, attr_text->font_size);
		text_size.y = attr_text->font_size;
	}
	else {
		if( attr_text->font_id == UINT_FAST16_MAX ) { attr_text->font_id = load_font(attr_text->font_name, attr_text->font_size); }
		text_size = MeasureTextEx(screen_fonts[attr_text->font_id].font, attr_text->text, (float)attr_text->font_size, 0.0f);
	}

	x = element->position.x;
	y = element->position.y;

	if( element->position.w > 0 ) {
		if( element->position.horizontal == ALIGN_RIGHT ) {
			x += element->position.w - text_size.x;
		}
		else if( element->position.horizontal == ALIGN_CENTER ) {
			x += element->position.w/2 - text_size.x/2;
		}
	}
	if( element->position.h > 0 ) {
		if( element->position.vertical == ALIGN_BOTTOM ) {
			y += element->position.h - text_size.y;
		}
		else if( element->position.vertical == ALIGN_MIDDLE ) {
			y += element->position.h/2 - text_size.y/2;
		}
	}

	if( attr_text->font_name == NULL ) {
		DrawText(attr_text->text, x, y, attr_text->font_size, attr_text->color);
	}
	else {
		DrawTextEx(screen_fonts[attr_text->font_id].font, attr_text->text, (Vector2){x, y}, (float)attr_text->font_size, 0.0f, attr_text->color);
	}
}

static void draw_clock(const screen_element *element) {
	char *str_format = ((screen_attrs_text *)element->attrs)->text;
	char str_time[255];  // TODO: document max formated time size(255 characters)
	struct tm *tm_local;
	tm_local = localtime(&screen_update_start.tv_sec);
	FAIL_ON_NULL(tm_local, "Failed to get localtime!");
	if( strftime(str_time, sizeof(str_time), str_format, tm_local) <= 0 ) {
		LOG_FATAL("Failed to format time with strftime »%s«", str_format);
	}
	((screen_attrs_text *)element->attrs)->text = str_time;
	draw_text(element);
	((screen_attrs_text *)element->attrs)->text = str_format;
}

static void draw_img(const screen_element *element) {
	screen_attrs_img *attr_img = ((screen_attrs_img *)element->attrs);
	
	if( attr_img->texture == NULL ) {
		MALLOC(attr_img->texture, sizeof(Texture2D));  // TODO free
		*attr_img->texture = LoadTextureFromImage(attr_img->image);
		LOG_DEBUG("Load image into GPU RAM, element ID %lu", element->id);
	}

	DrawTexture(*attr_img->texture, element->position.x, element->position.y, (Color){255,255,255,255});
}

/*******************************************************************************
 * Actually draw the selected screen element
 * @param i the index of the screen element to draw
 ******************************************************************************/
static void draw_element(const screen_element *element) {
//static screen_element *screen_elements;
//static uint_fast32_t screen_elements_count;
	if( element->evals != NULL ) {
		printf("%s\n", element->evals->lua_script);
	}
	switch( element->type ) {
		case SCREEN_TEXT:
			draw_text(element);
			break;
		case SCREEN_CLOCK:
			draw_clock(element);
			break;
		case SCREEN_IMG:
			draw_img(element);
			break;
		default:
			LOG_ERROR("Requested to draw unknown element type %u", element->type);
	}
}

/*******************************************************************************
 * pthread which draws the InfoScreen
 ******************************************************************************/
void *screen(void *_) {
//	SetConfigFlags(FLAG_SHOW_LOGO | FLAG_WINDOW_TRANSPARENT);
	InitWindow(config.width, config.height, "info_screen");
	SetTargetFPS(config.fps+1);

	LOG_DEBUG("InfoScreen window initiated");

	while (!WindowShouldClose() && !do_stop) {
		BeginDrawing();
		gettimeofday(&screen_update_start, NULL);
		pthread_mutex_lock( &mutex_look );

			ClearBackground(screen_background_color);

			for( uint_fast32_t i = 0; i < screen_elements_count; i++ ) {
				draw_element(&screen_elements[i]);
			}

		pthread_mutex_unlock( &mutex_look );
		EndDrawing();

		int fps = GetFPS();
		if( fps < config.fps - MAX_LOST_FPS ) {
			LOG_WARNING("Warning FPS is to low %d instead of %d", fps, config.fps);
		}
	}

	CloseWindow();

	pthread_exit(NULL);
}
