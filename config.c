#include "config.h"

static const char *TOPIC = "config management";


void parse_cmd(int argc, char* argv[]) {
	log_level = 1;
	char opt;
	while( (opt = getopt(argc, argv, "vcdn")) != -1 ) {
		switch(opt) {
			case 'v':
				log_level++;
				break;
			case 'c':
				log_color = true;
				break;
			case 'd':
				do_fork = true;
				break;
			case 'n':
				log_date = false;
				break;
		}
	}
}

void parse_config(char *path) {
	LOG_INFO("parsing config file: %s", path);
	char *str_config = NULL;
	if( !read_file(&str_config, path) ) {
		LOG_WARNING("Could not read config file »%s« using default values", path);
		str_config = strdup("{}");
	}
	cJSON *cjson_config = cJSON_Parse(str_config);
	free(str_config);
	if( ! cjson_config ) {
		const char *error_ptr = cJSON_GetErrorPtr();
		if( ! error_ptr ) {
			LOG_ERROR("Failed to parse config file %s", path);
		}
		else {
			LOG_ERROR("Failed to parse config file %s just before:\n%s", path, error_ptr);
		}
		LOG_WARNING("Using default values");
	}
	cJSON *cjson_layout = cJSON_GetObjectItemCaseSensitive(cjson_config, "layout");
	cJSON *cjson_resolution = cJSON_GetObjectItemCaseSensitive(cjson_config, "resolution");
	cJSON *cjson_name = cJSON_GetObjectItemCaseSensitive(cjson_config, "name");
	cJSON *cjson_fps = cJSON_GetObjectItemCaseSensitive(cjson_config, "fps");
	cJSON *cjson_width = cJSON_GetObjectItemCaseSensitive(cjson_resolution, "width");
	cJSON *cjson_height = cJSON_GetObjectItemCaseSensitive(cjson_resolution, "height");

	CJSON_DEF_STR(config.layout, cjson_layout, "default");
	CJSON_DEF_STR(config.name, cjson_name, "");
	CJSON_DEF_INT(config.fps, cjson_fps, 60);
	CJSON_DEF_INT(config.width, cjson_width, 500);
	CJSON_DEF_INT(config.height, cjson_height, 500);

	cJSON_Delete(cjson_config);
}
