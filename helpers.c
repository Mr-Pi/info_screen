#include "helpers.h"

static const char *TOPIC = "helper functions";


int pstrcmp(const void *a, const void *b) {
	return strcmp(*(const char**)a, *(const char**)b);
}

Color parse_color_str(char *str_color) {
	unsigned int r, g, b, a;
	int c = sscanf(str_color, "#%2x%2x%2x%2x", &r, &g, &b, &a);
	if( c < 3 ) {
		LOG_ERROR("Failed to parse color string »%s« using default color black", str_color);
		return (Color){ 0x00, 0x00, 0x00, 0xff };
	}
	else if( c == 3 ) {
		LOG_VERBOSE("Parsed color string »%s« as rgb", str_color);
		return (Color){ r, g, b, 0xff };
	}
	else {
		LOG_VERBOSE("Parsed color string »%s« as rgba", str_color);
		return (Color){ r, g, b, a };
	}
}

int test_filename_extension(char *filename, char *ext) {
	int len_ext = strlen(ext);
	int len_filename = strlen(filename);
	if( len_filename <= len_ext ) {
		return -1;
	}
	return strcmp(ext, filename+(len_filename-len_ext));
}

/*******************************************************************************
 * Reads a whole file into memory
 * @param **buffer: the char pointer, the file context is stored in
 * @param *path: the path to the file to read
 * @return the number of characters read, -1 on error
 ******************************************************************************/
int read_file(char **buffer, char *path) {
	LOG_VERBOSE("Reading whole file %s", path);
	FILE *fd = fopen(path, "r");
	if( !fd ) {
		LOG_ERROR("Failed to open %s for reading: %s", path, strerror(errno));
		return -1;
	}

	if( ! *buffer ) {
		MALLOC(*buffer, (sizeof(char) * READ_AT_ONCE));
	}
	else {
		REALLOC(buffer_tmp, *buffer, sizeof(char) * READ_AT_ONCE);
	}
	
	size_t c = 0;
	int length = 0;

	while( (c = fread(*buffer+length, sizeof(char), READ_AT_ONCE, fd)) ) {
		length += c;
		LOG_VERBOSE("Read %d(%d) characters from file %s", (int)c, (int)length, path);
		if( fseek(fd, length, SEEK_SET) < 0 ) {
			LOG_FATAL("Failed to seek in file %s: %s", path, strerror(errno));
		}
		REALLOC(new_buffer, *buffer, length * sizeof(char) + 1 + READ_AT_ONCE);
	}
	REALLOC(buffer_clean, *buffer, length * sizeof(char) + 1);
	if( feof(fd) ) {
		LOG_VERBOSE("Reached end of file while reading %s", path);
	}
	if( ferror(fd) ) {
		LOG_ERROR("Error while reading file %s: %s", path, strerror(errno));
		if( fclose(fd) != 0 ) {
			LOG_ERROR("Failed to close file %s: %s", path, strerror(errno));
		}
		return -1;
	}
	else if( fclose(fd) != 0 ) {
		LOG_ERROR("Failed to close file %s: %s", path, strerror(errno));
	}
	char *new_buffer = realloc(*buffer, sizeof(char) * ( length + 1 ) );
	if( !new_buffer ) {
		LOG_FATAL("Failed to reallocated memory while reading file %s: %s", path, strerror(errno));
	}
	*buffer = new_buffer;
	return length;
}
