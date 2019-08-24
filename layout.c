#include "layout.h"

static const char *TOPIC = "layout";


static type_frame *layout_frames;
static uint_fast16_t layout_frames_count;
static char *str_default_color;


static void layout_frame_init() {
	if( layout_frames_count > 0 ) {
		free(layout_frames);
	}
	layout_frames_count = 0;
}

static void layout_frame_add(screen_position position, char *name, uint_fast32_t element_id) {
	layout_frames_count++;
	LOG_DEBUG("Add frame %lu with name %s and element id %lu", layout_frames_count, name, element_id);
	REALLOC(tmp_layout_frames, layout_frames, sizeof(type_frame) * layout_frames_count);
	layout_frames[layout_frames_count-1].name = name;
	layout_frames[layout_frames_count-1].element_id = element_id;
}

static void layout_default_init() {
	LOG_INFO("Using default layout");
//	screen_add_text((screen_position){20,20}, "test2", 101, NULL, (Color){0,0,255,255});
}

static void layout_parse_main_attrs(cJSON *layout) {
	char *str_color;

	cJSON *cjson_default_color = cJSON_GetObjectItemCaseSensitive(layout, "default-color");
	CJSON_DEF_STR(str_default_color, cjson_default_color, "#000000ff");
	screen_background_color = parse_color_str(str_default_color);

	cJSON *cjson_background_color = cJSON_GetObjectItemCaseSensitive(layout, "background-color");
	CJSON_DEF_STR(str_color, cjson_background_color, "#ffffff");
	screen_background_color = parse_color_str(str_color);
}

static screen_attrs_text layout_parse_attrs_text(cJSON *attrs) {
	screen_attrs_text attrs_text;

	cJSON *cjson_font_size = cJSON_GetObjectItemCaseSensitive(attrs, "font-size");
	int font_size_in;
	CJSON_DEF_INT(font_size_in, cjson_font_size, 12);
	UINT_FAST16_T(attrs_text.font_size, font_size_in);

	cJSON *cjson_color = cJSON_GetObjectItemCaseSensitive(attrs, "color");
	char *str_color;
	CJSON_DEF_STR(str_color, cjson_color, str_default_color);
	attrs_text.color = parse_color_str(str_color);

	cJSON *cjson_font = cJSON_GetObjectItemCaseSensitive(attrs, "font-name");
	CJSON_DEF_STR(attrs_text.font_name, cjson_font, NULL);

	return attrs_text;
}

static void layout_add_img(screen_position *position, cJSON *attrs) {
	char *str_src;
	cJSON *cjson_src = cJSON_GetObjectItemCaseSensitive(attrs, "src");
	CJSON_DEF_STR(str_src, cjson_src, NULL);

	char *str_background_color;
	Color *background_color;
	MALLOC(background_color, sizeof(Color));
	cJSON *cjson_background_color = cJSON_GetObjectItemCaseSensitive(attrs, "background-color");
	CJSON_DEF_STR(str_background_color, cjson_background_color, NULL);
	if( str_background_color != NULL ) {
		*background_color = parse_color_str(str_background_color);
		LOG_DEBUG("Set background color for image »%s« to %s", str_src, str_background_color);
	}
	else {
		background_color = NULL;
	}

	char *str_evals;
	cJSON *cjson_evals = cJSON_GetObjectItemCaseSensitive(attrs, "evals");
	CJSON_DEF_STR(str_evals, cjson_evals, NULL);

	LOG_DEBUG("Add image to layout: %s", str_src);
	
	screen_resize resize_type;
	char *str_resize_type;
	cJSON *cjson_resize_type = cJSON_GetObjectItemCaseSensitive(attrs, "format");
	CJSON_DEF_STR(str_resize_type, cjson_resize_type, "proper");
	switch( str_resize_type[0] ) {
		case 'p':  resize_type = RESIZE_PROPER; break;
		case 'c':  resize_type = RESIZE_CROP; break;
		case 's':  resize_type = RESIZE_STRETCH; break;
		default:   resize_type = RESIZE_PROPER;
	}

	screen_attrs_img *attr_img = screen_prepare_image(position, resize_type, str_src, background_color);
	if( attr_img != NULL ) {
		screen_add_img(*position, *attr_img, str_evals);
	}
	else {
		LOG_ERROR("Failed to add image in layout!!!");
	}
}

static void layout_add_text(screen_position *position, cJSON *attrs) {
	cJSON *cjson_text = cJSON_GetObjectItemCaseSensitive(attrs, "text");
	char *text;
	CJSON_DEF_STR(text, cjson_text, "");
	LOG_DEBUG("Add text to layout: %s", text);

	screen_attrs_text attrs_text = layout_parse_attrs_text(attrs);

	char *str_evals;
	cJSON *cjson_evals = cJSON_GetObjectItemCaseSensitive(attrs, "evals");
	CJSON_DEF_STR(str_evals, cjson_evals, NULL);

	screen_add_text(*position, text, attrs_text.font_size, attrs_text.font_name, attrs_text.color, str_evals);
}

static void layout_add_clock(screen_position *position, cJSON *attrs) {
	cJSON *cjson_format = cJSON_GetObjectItemCaseSensitive(attrs, "format");
	char *format;
	CJSON_DEF_STR(format, cjson_format, "%H:%M");
	LOG_DEBUG("Add clock with format to layout: %s", format);

	screen_attrs_text attrs_text = layout_parse_attrs_text(attrs);

	char *str_evals;
	cJSON *cjson_evals = cJSON_GetObjectItemCaseSensitive(attrs, "evals");
	CJSON_DEF_STR(str_evals, cjson_evals, NULL);

	screen_add_clock(*position, format, attrs_text.font_size, attrs_text.font_name, attrs_text.color, str_evals);
}

static inline screen_position layout_parse_position(cJSON *cjson_position) {
	screen_position position;

	cJSON *cjson_x = cJSON_GetObjectItemCaseSensitive(cjson_position, "x");
	cJSON *cjson_y = cJSON_GetObjectItemCaseSensitive(cjson_position, "y");
	cJSON *cjson_w = cJSON_GetObjectItemCaseSensitive(cjson_position, "w");
	cJSON *cjson_h = cJSON_GetObjectItemCaseSensitive(cjson_position, "h");
	int ix, iy, iw, ih;
	CJSON_DEF_INT(ix, cjson_x, 0);
	CJSON_DEF_INT(iy, cjson_y, 0);
	CJSON_DEF_INT(iw, cjson_w, 0);
	CJSON_DEF_INT(ih, cjson_h, 0);
	if( ix < 0 ) { ix = config.width + ix; }
	if( iy < 0 ) { iy = config.height + iy; }

	cJSON *cjson_align_h = cJSON_GetObjectItemCaseSensitive(cjson_position, "align");
	cJSON *cjson_align_v = cJSON_GetObjectItemCaseSensitive(cjson_position, "valign");
	char *align_h, *align_v;
	CJSON_DEF_STR(align_h, cjson_align_h, "left");
	CJSON_DEF_STR(align_v, cjson_align_v, "top");

	UINT_FAST16_T(position.x, ix);
	UINT_FAST16_T(position.y, iy);
	UINT_FAST16_T(position.w, iw);
	UINT_FAST16_T(position.h, ih);

	if( strcmp("center", align_h) == 0 )     { position.horizontal = ALIGN_CENTER; }
	else if( strcmp("right", align_h) == 0 ) { position.horizontal = ALIGN_RIGHT; }
	else                                     { position.horizontal = ALIGN_LEFT; }
	if( strcmp("middle", align_v) == 0 )     { position.vertical = ALIGN_MIDDLE; }
	else if( strcmp("top", align_v) == 0)    { position.vertical = ALIGN_TOP; }
	else                                     { position.vertical = ALIGN_BOTTOM; }

	return position;
}

static void parse_frame(cJSON *cjson_frame) {
	cJSON *cjson_type = cJSON_GetObjectItemCaseSensitive(cjson_frame, "type");
	char *type;
	CJSON_DEF_STR(type, cjson_type, "dummy");

	screen_position position = layout_parse_position(cjson_frame);

	cJSON *attrs = cJSON_GetObjectItemCaseSensitive(cjson_frame, "attrs");
	if( strcmp("text", type) == 0 ) {
		layout_add_text(&position, attrs);
	}
	else if( strcmp("clock", type) == 0 ) {
		layout_add_clock(&position, attrs);
	}
	else if( strcmp("img", type) == 0 ) {
		layout_add_img(&position, attrs);
	}
}

void layout_init(char *layout_name) {
	LOG_INFO("Load layout »%s«", layout_name);
	layout_frame_init();
	char *layout_path;
	MALLOC(layout_path, strlen(layout_name) + sizeof("layouts/.json"));
	sprintf(layout_path, "layouts/%s.json", layout_name);
	LOG_VERBOSE("Parsing layout file »%s«", layout_path);
	char *str_layout = NULL;
	if( !read_file(&str_layout, layout_path) ) {
		LOG_WARNING("Could not read layout file »%s« using default layout", layout_path);
		layout_default_init();
	}
	free(layout_path);
	const char *str_error;
	cJSON *cjson_layout = cJSON_ParseWithOpts(str_layout, &str_error, false);
	// TODO: error check, log, call layout_default_init();

	layout_parse_main_attrs(cjson_layout);

	cJSON *cjson_frames = cJSON_GetObjectItemCaseSensitive(cjson_layout, "frames");
	cJSON *cjson_frame;
	cJSON_ArrayForEach(cjson_frame, cjson_frames) {
		parse_frame(cjson_frame);
	}
}
