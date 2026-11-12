#include "sim_doom.h"

#include <SDL.h>
#include <limits.h>
#include <spawn.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#include "doom/doom_wad.h"
#include "hal/doom.h"
#include "hal/input.h"
#include "sim_display.h"

extern char **environ;

enum {
    SCALE_TEXT_CAPACITY = 12,
    ARGUMENT_CAPACITY = 10
};

typedef struct SimDoom {
    const char *card_directory;
    bool headless;
    char scale[SCALE_TEXT_CAPACITY];
} SimDoom;

static SimDoom doom;

static char PROGRAM[] = ECHO_DOOM_PROGRAM;
static char CARD_FLAG[] = "--card";
static char SCALE_FLAG[] = "--scale";
static char HEADLESS_FLAG[] = "--headless";
static char FRAMES_FLAG[] = "--frames";
static char HEADLESS_FRAMES[] = "35";

void sim_doom_configure(const SimOptions *options)
{
    doom.card_directory = options->card_directory;
    doom.headless = options->headless;
    snprintf(doom.scale, sizeof doom.scale, "%d", options->scale);
}

bool hal_doom_ready(void)
{
    char wad_path[DOOM_WAD_PATH_CAPACITY];
    return doom.card_directory != NULL && access(PROGRAM, X_OK) == 0 &&
           doom_wad_find(wad_path, sizeof wad_path);
}

static void fill_arguments(char **arguments, char *card)
{
    int count = 0;
    arguments[count++] = PROGRAM;
    arguments[count++] = CARD_FLAG;
    arguments[count++] = card;
    arguments[count++] = SCALE_FLAG;
    arguments[count++] = doom.scale;
    if (doom.headless) {
        arguments[count++] = HEADLESS_FLAG;
        arguments[count++] = FRAMES_FLAG;
        arguments[count++] = HEADLESS_FRAMES;
    }
    arguments[count] = NULL;
}

static int run_program(char **arguments)
{
    pid_t child = 0;
    if (posix_spawn(&child, PROGRAM, NULL, NULL, arguments, environ) != 0) {
        return -1;
    }
    int status = 0;
    if (waitpid(child, &status, 0) != child || !WIFEXITED(status)) {
        return -1;
    }
    return WEXITSTATUS(status);
}

void hal_doom_run(void)
{
    if (doom.card_directory == NULL) {
        return;
    }
    static char card[PATH_MAX];
    snprintf(card, sizeof card, "%s", doom.card_directory);
    char *arguments[ARGUMENT_CAPACITY];
    fill_arguments(arguments, card);
    sim_display_set_visible(false);
    int status = run_program(arguments);
    sim_display_set_visible(true);
    SDL_Log("DOOM exited with status %d", status);
    hal_input_drop_presses();
}
