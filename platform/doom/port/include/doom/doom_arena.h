#pragma once

#include <stddef.h>

void *doom_port_malloc(size_t size);
void *doom_port_calloc(size_t count, size_t size);
void *doom_port_realloc(void *pointer, size_t size);
void doom_port_free(void *pointer);
char *doom_port_strdup(const char *text);
size_t doom_arena_peak(void);
size_t doom_arena_size(void);
