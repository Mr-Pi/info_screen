#ifndef __MAIN_H__
#define __MAIN_H__


#include <pthread.h>
#include <stdbool.h>
#include <raylib.h>
#include <unistd.h>

#include "config.h"
#include "log.h"
#include "screen.h"
#include "layout.h"


bool do_stop;
bool do_fork;
pthread_mutex_t mutex_look;


#endif
