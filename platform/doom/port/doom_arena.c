#include "doom/doom_arena.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "doom/doom_port.h"

enum {
    ARENA_ALIGNMENT = 16,
    HEADER_BYTES = 16,
    ARENA_BYTES = ECHO_DOOM_ARENA_KB * 1024
};

typedef struct Arena {
    _Alignas(ARENA_ALIGNMENT) unsigned char bytes[ARENA_BYTES];
    size_t used;
    size_t peak;
    bool has_last;
    size_t last_block;
} Arena;

static Arena arena;

static size_t rounded(size_t size)
{
    size_t at_least_one = size == 0 ? 1 : size;
    return (at_least_one + ARENA_ALIGNMENT - 1) & ~(size_t)(ARENA_ALIGNMENT - 1);
}

static size_t block_size(size_t offset)
{
    size_t size;
    memcpy(&size, &arena.bytes[offset], sizeof size);
    return size;
}

static void set_block(size_t offset, size_t size)
{
    memcpy(&arena.bytes[offset], &size, sizeof size);
    arena.used = offset + HEADER_BYTES + rounded(size);
    arena.has_last = true;
    arena.last_block = offset;
    if (arena.used > arena.peak) {
        arena.peak = arena.used;
    }
}

static bool owned(const void *pointer, size_t *offset)
{
    const unsigned char *bytes = pointer;
    if (bytes < &arena.bytes[HEADER_BYTES] || bytes >= &arena.bytes[ARENA_BYTES]) {
        return false;
    }
    *offset = (size_t)(bytes - arena.bytes) - HEADER_BYTES;
    return true;
}

static void refuse(size_t size)
{
    fprintf(stderr, "doom: the %d byte arena has no room for %zu more bytes\n", ARENA_BYTES, size);
    doom_port_exit(-1);
}

void *doom_port_malloc(size_t size)
{
    size_t offset = arena.used;
    if (HEADER_BYTES + rounded(size) > ARENA_BYTES - offset) {
        refuse(size);
    }
    set_block(offset, size);
    return &arena.bytes[offset + HEADER_BYTES];
}

void *doom_port_calloc(size_t count, size_t size)
{
    if (size != 0 && count > SIZE_MAX / size) {
        refuse(SIZE_MAX);
    }
    void *pointer = doom_port_malloc(count * size);
    memset(pointer, 0, count * size);
    return pointer;
}

static bool grows_in_place(size_t offset, size_t size)
{
    return arena.has_last && offset == arena.last_block &&
           HEADER_BYTES + rounded(size) <= ARENA_BYTES - offset;
}

void *doom_port_realloc(void *pointer, size_t size)
{
    size_t offset = 0;
    if (pointer == NULL || !owned(pointer, &offset)) {
        return doom_port_malloc(size);
    }
    if (grows_in_place(offset, size)) {
        set_block(offset, size);
        return pointer;
    }
    size_t old_size = block_size(offset);
    void *moved = doom_port_malloc(size);
    memcpy(moved, pointer, old_size < size ? old_size : size);
    return moved;
}

void doom_port_free(void *pointer)
{
    size_t offset = 0;
    if (pointer == NULL || !owned(pointer, &offset) || !arena.has_last ||
        offset != arena.last_block) {
        return;
    }
    arena.used = offset;
    arena.has_last = false;
}

char *doom_port_strdup(const char *text)
{
    size_t length = strlen(text) + 1;
    char *copy = doom_port_malloc(length);
    memcpy(copy, text, length);
    return copy;
}

size_t doom_arena_peak(void)
{
    return arena.peak;
}

size_t doom_arena_size(void)
{
    return ARENA_BYTES;
}
