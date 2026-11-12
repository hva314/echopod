#pragma once

#include <stdint.h>

#define DOOM_PORT_SAVE_UNAVAILABLE "saving comes in a later\nversion of echopod.\n\npress a key."

enum {
    DOOM_PORT_MENU_CLOSED,
    DOOM_PORT_MENU_INNER,
    DOOM_PORT_MENU_TOP,
    DOOM_PORT_MENU_PROMPT
};

unsigned char *doom_port_zone(int *size);
_Noreturn void doom_port_exit(int status);
int doom_port_file_exists(const char *path);
int doom_port_menu_state(void);

void D_DoomMain(void);
void doomgeneric_Tick(void);
uint32_t DG_GetTicksMs(void);
void DG_SleepMs(uint32_t milliseconds);
int DG_GetKey(int *pressed, unsigned char *key);
