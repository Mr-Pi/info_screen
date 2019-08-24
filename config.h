#ifndef __CONFIG_H__
#define __CONFIG_H__


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>

#include "log.h"
#include "main.h"
#include "helpers.h"
#include "cJSON.h"


struct config {
	int width;
	int height;
	int fps;
	char *name;
	char *layout;
} config;


void parse_config(char *path);
void parse_cmd(int argc, char* argv[]);


#endif
