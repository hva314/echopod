#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct DoomRunConfig {
    const char *wad_path;
    void (*sleep_ms)(uint32_t milliseconds);
} DoomRunConfig;

typedef struct DoomMemoryReport {
    size_t zone_peak;
    size_t zone_size;
    size_t arena_peak;
    size_t arena_size;
} DoomMemoryReport;

bool doom_run_start(const DoomRunConfig *config);
bool doom_run_tick(void);
int doom_run_exit_status(void);
DoomMemoryReport doom_run_memory(void);
void doom_run_sample_memory(void);
