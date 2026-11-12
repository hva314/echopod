#include "doom/doom_wad.h"

#include "hal/storage.h"

const char DOOM_WAD_FOLDER[] = "/DOOM";

static const char *const IWAD_NAMES[] = {
    "doom.wad", "doom2.wad", "doom1.wad", "freedoom1.wad", "freedoom2.wad",
};

enum {
    IWAD_COUNT = sizeof IWAD_NAMES / sizeof IWAD_NAMES[0],
    NO_RANK = -1
};

static char lower(char character)
{
    return character >= 'A' && character <= 'Z' ? (char)(character - 'A' + 'a') : character;
}

static bool same_name(const char *name, const char *expected)
{
    size_t index = 0;
    while (expected[index] != '\0') {
        if (lower(name[index]) != expected[index]) {
            return false;
        }
        index++;
    }
    return name[index] == '\0';
}

int doom_wad_rank(const char *name)
{
    for (int index = 0; index < (int)IWAD_COUNT; index++) {
        if (same_name(name, IWAD_NAMES[index])) {
            return index;
        }
    }
    return NO_RANK;
}

void doom_wad_choice_init(DoomWadChoice *choice)
{
    *choice = (DoomWadChoice){.found = false, .rank = NO_RANK};
}

static void copy_name(char *target, const char *name)
{
    size_t index = 0;
    while (name[index] != '\0' && index + 1 < DOOM_WAD_NAME_CAPACITY) {
        target[index] = name[index];
        index++;
    }
    target[index] = '\0';
}

void doom_wad_choice_offer(DoomWadChoice *choice, const char *name)
{
    int rank = doom_wad_rank(name);
    if (rank == NO_RANK || (choice->found && rank >= choice->rank)) {
        return;
    }
    choice->found = true;
    choice->rank = rank;
    copy_name(choice->name, name);
}

static size_t append(char *path, size_t length, size_t capacity, const char *text)
{
    for (size_t index = 0; text[index] != '\0'; index++) {
        if (length + 1 >= capacity) {
            return capacity;
        }
        path[length++] = text[index];
    }
    path[length] = '\0';
    return length;
}

bool doom_wad_choice_path(const DoomWadChoice *choice, char *path, size_t capacity)
{
    if (!choice->found || capacity == 0) {
        return false;
    }
    path[0] = '\0';
    size_t length = append(path, 0, capacity, DOOM_WAD_FOLDER);
    length = append(path, length, capacity, "/");
    length = append(path, length, capacity, choice->name);
    return length < capacity;
}

bool doom_wad_find(char *path, size_t capacity)
{
    static HalDirEntry entry;
    HalDir dir;
    if (!hal_dir_open(&dir, DOOM_WAD_FOLDER)) {
        return false;
    }
    DoomWadChoice choice;
    doom_wad_choice_init(&choice);
    while (hal_dir_next(&dir, &entry)) {
        if (!entry.directory) {
            doom_wad_choice_offer(&choice, entry.name);
        }
    }
    hal_dir_close(&dir);
    return doom_wad_choice_path(&choice, path, capacity);
}
