#include "doom_launch.h"

#include "core/i18n.h"
#include "hal/doom.h"

static const bool DOOM_READY = true;
static const bool DOOM_MISSING = false;

static ScreenAction start_doom(void *context)
{
    Player *player = context;
    if (player->state == PLAYER_PLAYING) {
        player_toggle_pause(player);
    }
    player_flush_save(player);
    hal_doom_run();
    return screen_action_none();
}

static bool doom_shown(const void *context)
{
    const bool *ready = context;
    return *ready;
}

int doom_launch_add(MenuItem *items, Player *player)
{
    items[0] = (MenuItem){.label = i18n_text(STR_MENU_DOOM),
                          .select = start_doom,
                          .context = player,
                          .shown = doom_shown,
                          .shown_context = hal_doom_ready() ? &DOOM_READY : &DOOM_MISSING};
    return DOOM_LAUNCH_ROWS;
}
