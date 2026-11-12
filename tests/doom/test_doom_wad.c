#include <string.h>

#include "check.h"
#include "doom/doom_wad.h"
#include "hal/storage.h"

enum {
    MAX_LISTED = 8
};

typedef struct FakeFolder {
    bool exists;
    const char *names[MAX_LISTED];
    bool directories[MAX_LISTED];
    int count;
    int next;
    bool closed;
} FakeFolder;

static FakeFolder folder;

bool hal_dir_open(HalDir *dir, const char *path)
{
    dir->handle = 1;
    folder.next = 0;
    folder.closed = false;
    return folder.exists && strcmp(path, "/DOOM") == 0;
}

bool hal_dir_next(HalDir *dir, HalDirEntry *entry)
{
    (void)dir;
    if (folder.next >= folder.count) {
        return false;
    }
    entry->directory = folder.directories[folder.next];
    strcpy(entry->name, folder.names[folder.next]);
    folder.next++;
    return true;
}

void hal_dir_close(HalDir *dir)
{
    (void)dir;
    folder.closed = true;
}

static void list(const char *first, const char *second, const char *third)
{
    folder = (FakeFolder){.exists = true, .names = {first, second, third}, .count = 3};
}

static void names_match_without_case(void)
{
    CHECK(doom_wad_rank("DOOM1.WAD") >= 0);
    CHECK(doom_wad_rank("FreeDoom1.Wad") >= 0);
}

static void other_names_are_not_iwads(void)
{
    CHECK(doom_wad_rank("doom1.wad.bak") < 0);
    CHECK(doom_wad_rank("doom") < 0);
    CHECK(doom_wad_rank("mymod.wad") < 0);
    CHECK(doom_wad_rank("") < 0);
}

static void the_full_game_wins_over_the_shareware_and_freedoom(void)
{
    CHECK(doom_wad_rank("doom.wad") < doom_wad_rank("doom2.wad"));
    CHECK(doom_wad_rank("doom2.wad") < doom_wad_rank("doom1.wad"));
    CHECK(doom_wad_rank("doom1.wad") < doom_wad_rank("freedoom1.wad"));
    CHECK(doom_wad_rank("freedoom1.wad") < doom_wad_rank("freedoom2.wad"));
}

static void the_best_listed_wad_is_found_with_its_listed_name(void)
{
    char path[DOOM_WAD_PATH_CAPACITY];
    list("freedoom1.wad", "DOOM1.WAD", "notes.txt");
    CHECK(doom_wad_find(path, sizeof path));
    CHECK_EQUAL(0, strcmp(path, "/DOOM/DOOM1.WAD"));
    CHECK(folder.closed);
}

static void a_folder_named_like_a_wad_is_skipped(void)
{
    char path[DOOM_WAD_PATH_CAPACITY];
    list("doom.wad", "freedoom2.wad", "readme.txt");
    folder.directories[0] = true;
    CHECK(doom_wad_find(path, sizeof path));
    CHECK_EQUAL(0, strcmp(path, "/DOOM/freedoom2.wad"));
}

static void no_wad_in_the_folder_finds_nothing(void)
{
    char path[DOOM_WAD_PATH_CAPACITY];
    list("a.mp3", "b.flac", "mod.wad");
    CHECK(!doom_wad_find(path, sizeof path));
    CHECK(folder.closed);
}

static void no_folder_finds_nothing(void)
{
    char path[DOOM_WAD_PATH_CAPACITY];
    folder = (FakeFolder){.exists = false};
    CHECK(!doom_wad_find(path, sizeof path));
}

static void a_path_that_does_not_fit_is_refused(void)
{
    char path[8];
    list("freedoom1.wad", "x", "y");
    CHECK(!doom_wad_find(path, sizeof path));
}

int main(void)
{
    static const TestCase cases[] = {
        TEST_CASE(names_match_without_case),
        TEST_CASE(other_names_are_not_iwads),
        TEST_CASE(the_full_game_wins_over_the_shareware_and_freedoom),
        TEST_CASE(the_best_listed_wad_is_found_with_its_listed_name),
        TEST_CASE(a_folder_named_like_a_wad_is_skipped),
        TEST_CASE(no_wad_in_the_folder_finds_nothing),
        TEST_CASE(no_folder_finds_nothing),
        TEST_CASE(a_path_that_does_not_fit_is_refused),
    };
    return test_run_all(cases, TEST_CASE_COUNT(cases));
}
