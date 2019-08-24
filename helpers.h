#ifndef __HELPERS_H__
#define __HELPERS_H__


#ifndef READ_AT_ONCE
#define READ_AT_ONCE 1024
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <raylib.h>

#include "log.h"


#define MALLOC(_ptr_, _size_) \
	_ptr_ = malloc(_size_); \
	if( _ptr_ == NULL ) { \
		LOG_FATAL("Failed to allocate %lu bytes for %s", _size_, #_ptr_); \
	}

#define REALLOC(_tmp_var_, _ptr_, _new_size_) \
	void *_tmp_var_ = realloc(_ptr_, _new_size_); \
	if( _tmp_var_ == NULL ) { \
		LOG_FATAL("Failed to reallocated memory for %s with new size %lu of bytes", #_ptr_, _new_size_); \
	} \
	_ptr_ = _tmp_var_;

#define FAIL_ON_NULL(_var_, _error_msg_, ...) \
	if( _var_ == NULL ) { \
		LOG_FATAL(_error_msg_ ": %s", ##__VA_ARGS__, strerror(errno)); \
	}

#define PTHREAD_CREATE(_thread_) \
	pthread_t pth_ ##_thread_; \
	if( pthread_create(&pth_ ##_thread_,  NULL, _thread_, NULL) ) { \
		LOG_FATAL("Failed to start thread " #_thread_); \
	} \
	LOG_DEBUG("Started thread " #_thread_);

#define PTHREAD_JOIN(_thread_) \
	pthread_join(pth_ ##_thread_, NULL);

#define CJSON_DEF_INT(_dest_, _cjson_, _default_) \
	if( _cjson_ && cJSON_IsNumber(_cjson_) ) { \
		_dest_ = _cjson_->valueint; \
		LOG_VERBOSE("Parsed value »%d« for %s from %s", _dest_, #_dest_, #_cjson_); \
	} \
	else { \
		_dest_ = _default_; \
		LOG_VERBOSE("Default value »%d« for %s from %s", _dest_, #_dest_, #_cjson_); \
	}

#define CJSON_DEF_STR(_dest_, _cjson_, _default_) \
	if( _cjson_ && cJSON_IsString(_cjson_) && _cjson_->valuestring ) { \
		_dest_ = strdup(_cjson_->valuestring); \
		LOG_VERBOSE("Parsed value »%s« for %s from %s", _dest_, #_dest_, #_cjson_); \
	} \
	else { \
		_dest_ = _default_; \
		LOG_VERBOSE("Default value »%s« for %s from %s", _dest_, #_dest_, #_cjson_); \
	}

#define UINT_FAST16_T(_dest_, _val_) \
	if( _val_ < 0 ) _dest_ = 0; \
	else if( _val_ > UINT_FAST16_MAX ) _dest_ = UINT_FAST16_MAX; \
	else _dest_ = (uint_fast16_t)_val_;

int pstrcmp(const void *a, const void *b);
Color parse_color_str(char *str_color);
int test_filename_extension(char *filename, char *ext);
int read_file(char **ptr, char *path);


#endif
