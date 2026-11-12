#include "doom/doom_port.h"

#include <setjmp.h>
#include <stdio.h>
#include <string.h>

#include "d_main.h"
#include "doom/doom_arena.h"
#include "doom/doom_files.h"
#include "doom/doom_run.h"
#include "doom/doom_wad.h"
#include "doomgeneric.h"
#include "hal/clock.h"
#include "hal/storage.h"
#include "m_argv.h"
#include "z_zone.h"

enum {
    ZONE_ALIGNMENT = 16,
    ZONE_BYTES = ECHO_DOOM_ZONE_KB * 1024,
    ARGUMENT_COUNT = 3
};

typedef struct DoomPort {
    _Alignas(ZONE_ALIGNMENT) unsigned char zone[ZONE_BYTES];
    jmp_buf exit_point;
    int exit_status;
    bool zone_ready;
    size_t zone_peak;
    void (*sleep_ms)(uint32_t milliseconds);
    char program[sizeof "echo_doom"];
    char iwad_flag[sizeof "-iwad"];
    char wad_path[DOOM_WAD_PATH_CAPACITY];
    char *arguments[ARGUMENT_COUNT + 1];
} DoomPort;

static DoomPort port;

unsigned char *doom_port_zone(int *size)
{
    *size = ZONE_BYTES;
    port.zone_ready = true;
    return port.zone;
}

FILE *doom_port_fopen(const char *path, const char *mode)
{
    (void)path;
    (void)mode;
    return NULL;
}

int doom_port_mkdir(const char *path, unsigned int mode)
{
    (void)path;
    (void)mode;
    return -1;
}

_Noreturn void doom_port_exit(int status)
{
    doom_run_sample_memory();
    port.exit_status = status;
    longjmp(port.exit_point, 1);
}

int doom_port_file_exists(const char *path)
{
    HalFile file;
    if (!hal_file_open(&file, path)) {
        return 0;
    }
    hal_file_close(&file);
    return 1;
}

uint32_t DG_GetTicksMs(void)
{
    return hal_clock_milliseconds();
}

void DG_SleepMs(uint32_t milliseconds)
{
    port.sleep_ms(milliseconds);
}

static void set_arguments(const char *wad_path)
{
    strcpy(port.program, "echo_doom");
    strcpy(port.iwad_flag, "-iwad");
    snprintf(port.wad_path, sizeof port.wad_path, "%s", wad_path);
    port.arguments[0] = port.program;
    port.arguments[1] = port.iwad_flag;
    port.arguments[2] = port.wad_path;
    port.arguments[ARGUMENT_COUNT] = NULL;
    myargc = ARGUMENT_COUNT;
    myargv = port.arguments;
}

bool doom_run_start(const DoomRunConfig *config)
{
    port.sleep_ms = config->sleep_ms;
    set_arguments(config->wad_path);
    if (setjmp(port.exit_point) != 0) {
        return false;
    }
    D_DoomMain();
    return true;
}

bool doom_run_tick(void)
{
    if (setjmp(port.exit_point) != 0) {
        return false;
    }
    doomgeneric_Tick();
    return true;
}

int doom_run_exit_status(void)
{
    return port.exit_status;
}

void doom_run_sample_memory(void)
{
    if (!port.zone_ready) {
        return;
    }
    size_t used = ZONE_BYTES - (size_t)Z_FreeMemory();
    if (used > port.zone_peak) {
        port.zone_peak = used;
    }
}

DoomMemoryReport doom_run_memory(void)
{
    return (DoomMemoryReport){.zone_peak = port.zone_peak,
                              .zone_size = ZONE_BYTES,
                              .arena_peak = doom_arena_peak(),
                              .arena_size = doom_arena_size()};
}
