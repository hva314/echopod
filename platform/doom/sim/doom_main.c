#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "doom/doom_run.h"
#include "doom/doom_wad.h"
#include "hal/clock.h"
#include "sim_clock.h"
#include "sim_display.h"
#include "sim_input.h"
#include "sim_options.h"
#include "sim_script.h"
#include "sim_storage.h"

enum {
    EXIT_USAGE = 2,
    VIRTUAL_START_MS = 1000,
    FIRST_SCRIPTED_FRAME = 1
};

typedef struct DoomSession {
    SimOptions options;
    SimScriptEvent events[SIM_SCRIPT_MAX_EVENTS];
    int event_count;
    int next_event;
    uint32_t frames;
} DoomSession;

static void print_usage(void)
{
    fputs("usage: echo_doom --card DIR_WITH_DOOM_FOLDER [--scale N] [--frames N] [--headless]\n"
          "                 [--screenshot FILE.bmp] [--keys "
          "up,down,play,menu,back,power[:long],...]\n",
          stderr);
}

static bool prepare_session(int argument_count, char **arguments, DoomSession *session)
{
    *session = (DoomSession){.event_count = 0};
    if (!sim_options_parse(argument_count, arguments, &session->options)) {
        return false;
    }
    if (session->options.key_script == NULL) {
        return true;
    }
    SimScript script;
    if (!sim_script_parse(session->options.key_script, &script)) {
        return false;
    }
    session->event_count = sim_script_schedule(&script, FIRST_SCRIPTED_FRAME, session->events);
    return true;
}

static void sleep_virtual(uint32_t milliseconds)
{
    sim_clock_advance(milliseconds);
}

static void sleep_real(uint32_t milliseconds)
{
    SDL_Delay(milliseconds);
}

static bool start_sdl(const SimOptions *options)
{
    if (SDL_Init(options->headless ? SDL_INIT_TIMER : SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }
    if (options->headless) {
        sim_clock_use_virtual_time(VIRTUAL_START_MS);
        return true;
    }
    if (!sim_display_open(options->scale)) {
        SDL_Log("could not open the display: %s", SDL_GetError());
        return false;
    }
    sim_display_set_title("EchoPod DOOM");
    return true;
}

static bool pump_events(void)
{
    bool keep_running = true;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            keep_running = false;
        }
        sim_input_handle_event(&event);
    }
    return keep_running;
}

static void inject_scripted_keys(DoomSession *session)
{
    while (session->next_event < session->event_count &&
           session->events[session->next_event].frame == session->frames) {
        const SimScriptEvent *event = &session->events[session->next_event++];
        sim_input_inject(event->key, event->pressed, hal_clock_milliseconds());
    }
}

static bool frame_limit_reached(const DoomSession *session)
{
    return session->options.frame_limit > 0 && session->frames >= session->options.frame_limit;
}

static int play(DoomSession *session, const char *wad_path)
{
    DoomRunConfig config = {.wad_path = wad_path,
                            .sleep_ms = session->options.headless ? sleep_virtual : sleep_real};
    if (!doom_run_start(&config)) {
        return doom_run_exit_status();
    }
    while (pump_events() && !frame_limit_reached(session)) {
        inject_scripted_keys(session);
        if (!doom_run_tick()) {
            return doom_run_exit_status();
        }
        session->frames++;
    }
    return 0;
}

static void report_memory(void)
{
    DoomMemoryReport memory = doom_run_memory();
    printf("doom: zone peak %zu/%zu B, arena peak %zu/%zu B\n", memory.zone_peak, memory.zone_size,
           memory.arena_peak, memory.arena_size);
}

static int finish(const SimOptions *options, int status)
{
    if (options->screenshot_path != NULL && !sim_display_save_bmp(options->screenshot_path)) {
        SDL_Log("could not save %s: %s", options->screenshot_path, SDL_GetError());
        status = EXIT_FAILURE;
    }
    report_memory();
    sim_display_close();
    sim_storage_close_all();
    SDL_Quit();
    return status == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argument_count, char **arguments)
{
    static DoomSession session;
    if (!prepare_session(argument_count, arguments, &session)) {
        print_usage();
        return EXIT_USAGE;
    }
    SDL_SetMainReady();
    if (!start_sdl(&session.options)) {
        SDL_Quit();
        return EXIT_FAILURE;
    }
    sim_storage_configure(session.options.card_directory, session.options.system_directory);
    char wad_path[DOOM_WAD_PATH_CAPACITY];
    if (!doom_wad_find(wad_path, sizeof wad_path)) {
        fprintf(stderr, "echo_doom: no DOOM WAD in %s on the card\n", DOOM_WAD_FOLDER);
        return finish(&session.options, EXIT_FAILURE);
    }
    printf("echo_doom: playing %s\n", wad_path);
    return finish(&session.options, play(&session, wad_path));
}
