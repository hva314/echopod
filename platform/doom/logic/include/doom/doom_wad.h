#pragma once

#include <stdbool.h>
#include <stddef.h>

enum {
    DOOM_WAD_NAME_CAPACITY = 16,
    DOOM_WAD_PATH_CAPACITY = 32
};

typedef struct DoomWadChoice {
    bool found;
    int rank;
    char name[DOOM_WAD_NAME_CAPACITY];
} DoomWadChoice;

extern const char DOOM_WAD_FOLDER[];

int doom_wad_rank(const char *name);
void doom_wad_choice_init(DoomWadChoice *choice);
void doom_wad_choice_offer(DoomWadChoice *choice, const char *name);
bool doom_wad_choice_path(const DoomWadChoice *choice, char *path, size_t capacity);
bool doom_wad_find(char *path, size_t capacity);
