#ifndef __LOG_H__
#define __LOG_H__


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>


int log_level;
bool log_color;
bool log_date;
char log_str_date[21];


static inline void log_update_date() {
	if( log_date ) {
		time_t t = time(NULL);
		struct tm *tmp = localtime(&t);

		if( tmp == NULL ) {
			strncpy(log_str_date, "YYYY-mm-dd HH:MM:SS ", sizeof(log_str_date));
			return;
		}

		if( strftime(log_str_date, sizeof(log_str_date), "%Y-%m-%d %H:%M:%S ", tmp) == 0) {
			strncpy(log_str_date, "YYYY-mm-dd HH:MM:SS ", sizeof(log_str_date));
			return;
		}
	}
	else {
		strncpy(log_str_date, "", sizeof(log_str_date));
	}
}


#define ANSI_RESET    "\e[0m"
#define ANSI_BOLD     "\e[1m"
#define ANSI_NOBOLD   "\e[22m"

#define COLOR_VERBOSE "\e[34m"
#define COLOR_DEBUG   "\e[35m"
#define COLOR_INFO    "\e[37m"
#define COLOR_WARNING "\e[33m"
#define COLOR_ERROR   "\e[31m"
#define COLOR_FATAL   "\e[31;5m"

#define LOG_FORMAT_FILE "%15s:%-4d"

#define LOG_FATAL(_format_, ...) \
	log_update_date(); \
	if( log_level >= 0 && log_color ) \
		printf( ANSI_RESET "%s" ANSI_BOLD COLOR_FATAL   "F " \
				ANSI_RESET LOG_FORMAT_FILE \
				ANSI_RESET ANSI_BOLD COLOR_FATAL   " %20s: " ANSI_NOBOLD _format_ "\n" ANSI_RESET, log_str_date, __FILE__, __LINE__, TOPIC, ##__VA_ARGS__); \
	else if( log_level >= 0 ) \
		printf( "%sF " LOG_FORMAT_FILE " %20s: " _format_ "\n", log_str_date,  __FILE__, __LINE__, TOPIC, ##__VA_ARGS__); \
	exit(1)
#define LOG_ERROR(_format_, ...) \
	log_update_date(); \
	if( log_level >= 1 && log_color ) \
		printf( ANSI_RESET "%s" ANSI_BOLD COLOR_ERROR   "E " \
				ANSI_RESET LOG_FORMAT_FILE \
				ANSI_BOLD COLOR_ERROR   " %20s: " ANSI_NOBOLD _format_ "\n" ANSI_RESET, log_str_date,  __FILE__, __LINE__, TOPIC, ##__VA_ARGS__); \
	else if( log_level >= 1 ) \
		printf( "%sE " LOG_FORMAT_FILE " %20s: " _format_ "\n", log_str_date,  __FILE__, __LINE__, TOPIC, ##__VA_ARGS__);
#define LOG_WARNING(_format_, ...) \
	log_update_date(); \
	if( log_level >= 2 && log_color ) \
		printf( ANSI_RESET "%s" ANSI_BOLD COLOR_WARNING "W " \
				ANSI_RESET LOG_FORMAT_FILE \
				ANSI_BOLD COLOR_WARNING " %20s: " ANSI_NOBOLD _format_ "\n" ANSI_RESET, log_str_date,  __FILE__, __LINE__, TOPIC, ##__VA_ARGS__); \
	else if( log_level >= 2 ) \
		printf( "%sW " LOG_FORMAT_FILE " %20s: " _format_ "\n", log_str_date,  __FILE__, __LINE__, TOPIC, ##__VA_ARGS__);
#define LOG_INFO(_format_, ...) \
	log_update_date(); \
	if( log_level >= 3 && log_color ) \
		printf( ANSI_RESET "%s" ANSI_BOLD COLOR_INFO    "I " \
				ANSI_RESET LOG_FORMAT_FILE \
				ANSI_BOLD COLOR_INFO    " %20s: " ANSI_NOBOLD _format_ "\n" ANSI_RESET, log_str_date,  __FILE__, __LINE__, TOPIC, ##__VA_ARGS__); \
	else if( log_level >= 3 ) \
		printf( "%sI " LOG_FORMAT_FILE " %20s: " _format_ "\n", log_str_date,  __FILE__, __LINE__, TOPIC, ##__VA_ARGS__);
#define LOG_DEBUG(_format_, ...) \
	log_update_date(); \
	if( log_level >= 4 && log_color ) \
		printf( ANSI_RESET "%s" ANSI_BOLD COLOR_DEBUG   "D " \
				ANSI_RESET LOG_FORMAT_FILE \
				ANSI_BOLD COLOR_DEBUG   " %20s: " ANSI_NOBOLD _format_ "\n" ANSI_RESET, log_str_date,  __FILE__, __LINE__, TOPIC, ##__VA_ARGS__); \
	else if( log_level >= 4 ) \
		printf( "%sD " LOG_FORMAT_FILE " %20s: " _format_ "\n", log_str_date,  __FILE__, __LINE__, TOPIC, ##__VA_ARGS__);
#define LOG_VERBOSE(_format_, ...) \
	log_update_date(); \
	if( log_level >= 5 && log_color ) \
		printf( ANSI_RESET "%s" ANSI_BOLD COLOR_VERBOSE "V " \
				ANSI_RESET LOG_FORMAT_FILE \
				ANSI_BOLD COLOR_VERBOSE " %20s: " ANSI_NOBOLD _format_ "\n" ANSI_RESET, log_str_date,  __FILE__, __LINE__, TOPIC, ##__VA_ARGS__); \
	else if( log_level >= 5 ) \
		printf( "%sV " LOG_FORMAT_FILE " %20s: " _format_ "\n", log_str_date,  __FILE__, __LINE__, TOPIC, ##__VA_ARGS__);


#endif
