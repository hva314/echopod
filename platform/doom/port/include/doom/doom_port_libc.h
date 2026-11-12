#pragma once

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "doom/doom_arena.h"
#include "doom/doom_files.h"

#define malloc(size) doom_port_malloc(size)
#define calloc(count, size) doom_port_calloc(count, size)
#define realloc(pointer, size) doom_port_realloc(pointer, size)
#define free(pointer) doom_port_free(pointer)
#define strdup(text) doom_port_strdup(text)
#define fopen(path, mode) doom_port_fopen(path, mode)
#define mkdir(path, mode) doom_port_mkdir(path, mode)
