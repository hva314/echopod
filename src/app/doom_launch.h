#pragma once

#include "player.h"
#include "ui/menu_screen.h"

#ifdef ECHO_DOOM_HIDDEN
enum {
    DOOM_LAUNCH_ROWS = 0
};
#else
enum {
    DOOM_LAUNCH_ROWS = 1
};
#endif

int doom_launch_add(MenuItem *items, Player *player);
