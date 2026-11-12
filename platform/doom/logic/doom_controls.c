#include "doom/doom_controls.h"

typedef struct LadderAction {
    DoomInput input;
    DoomEdge edge;
} LadderAction;

static const LadderAction NO_ACTION = {DOOM_INPUT_NONE, DOOM_EDGE_PRESS};

static void emit(DoomEvents *events, DoomInput input, DoomEdge edge)
{
    if (input == DOOM_INPUT_NONE || events->count >= DOOM_EVENTS_CAPACITY) {
        return;
    }
    events->items[events->count++] = (DoomInputEvent){input, edge};
}

static void press(DoomControls *controls, DoomInput input, DoomEvents *events)
{
    if (input == DOOM_INPUT_NONE || controls->held[input]) {
        return;
    }
    controls->held[input] = true;
    emit(events, input, DOOM_EDGE_PRESS);
}

static void release(DoomControls *controls, DoomInput input, DoomEvents *events)
{
    if (input == DOOM_INPUT_NONE || !controls->held[input]) {
        return;
    }
    controls->held[input] = false;
    emit(events, input, DOOM_EDGE_RELEASE);
}

static void stop_walking(DoomControls *controls, DoomEvents *events)
{
    controls->walking = false;
    release(controls, DOOM_INPUT_FORWARD, events);
}

static void toggle_walking(DoomControls *controls, DoomEvents *events)
{
    if (controls->walking) {
        stop_walking(controls, events);
        return;
    }
    controls->walking = true;
    press(controls, DOOM_INPUT_FORWARD, events);
}

static void toggle_running(DoomControls *controls, DoomEvents *events)
{
    if (controls->held[DOOM_INPUT_RUN]) {
        release(controls, DOOM_INPUT_RUN, events);
        return;
    }
    press(controls, DOOM_INPUT_RUN, events);
}

static LadderAction game_action(DoomControls *controls, Key key, DoomEvents *events)
{
    switch (key) {
    case KEY_VOLUME_DOWN:
        return (LadderAction){DOOM_INPUT_TURN_LEFT, DOOM_EDGE_PRESS};
    case KEY_VOLUME_UP:
        return (LadderAction){DOOM_INPUT_TURN_RIGHT, DOOM_EDGE_PRESS};
    case KEY_PLAY:
        return (LadderAction){DOOM_INPUT_FIRE, DOOM_EDGE_PRESS};
    case KEY_BACK:
        stop_walking(controls, events);
        return (LadderAction){DOOM_INPUT_BACKWARD, DOOM_EDGE_PRESS};
    case KEY_MENU:
        toggle_walking(controls, events);
        return NO_ACTION;
    default:
        return NO_ACTION;
    }
}

static LadderAction game_combo_action(DoomControls *controls, Key key, DoomEvents *events)
{
    switch (key) {
    case KEY_VOLUME_DOWN:
        return (LadderAction){DOOM_INPUT_PREVIOUS_WEAPON, DOOM_EDGE_TAP};
    case KEY_VOLUME_UP:
        return (LadderAction){DOOM_INPUT_NEXT_WEAPON, DOOM_EDGE_TAP};
    case KEY_PLAY:
        return (LadderAction){DOOM_INPUT_AUTOMAP, DOOM_EDGE_TAP};
    case KEY_BACK:
        return (LadderAction){DOOM_INPUT_ESCAPE, DOOM_EDGE_TAP};
    case KEY_MENU:
        toggle_running(controls, events);
        return NO_ACTION;
    default:
        return NO_ACTION;
    }
}

static DoomInput menu_back_input(DoomMode mode)
{
    return mode == DOOM_MODE_MENU_TOP ? DOOM_INPUT_ESCAPE : DOOM_INPUT_MENU_BACK;
}

static LadderAction menu_action(DoomMode mode, Key key)
{
    switch (key) {
    case KEY_VOLUME_DOWN:
        return (LadderAction){DOOM_INPUT_MENU_UP, DOOM_EDGE_PRESS};
    case KEY_VOLUME_UP:
        return (LadderAction){DOOM_INPUT_MENU_DOWN, DOOM_EDGE_PRESS};
    case KEY_PLAY:
        return (LadderAction){DOOM_INPUT_MENU_SELECT, DOOM_EDGE_PRESS};
    case KEY_BACK:
        return (LadderAction){menu_back_input(mode), DOOM_EDGE_PRESS};
    case KEY_MENU:
        return (LadderAction){DOOM_INPUT_ESCAPE, DOOM_EDGE_PRESS};
    default:
        return NO_ACTION;
    }
}

static LadderAction menu_combo_action(Key key)
{
    switch (key) {
    case KEY_VOLUME_DOWN:
        return (LadderAction){DOOM_INPUT_MENU_LEFT, DOOM_EDGE_TAP};
    case KEY_VOLUME_UP:
        return (LadderAction){DOOM_INPUT_MENU_RIGHT, DOOM_EDGE_TAP};
    default:
        return NO_ACTION;
    }
}

static LadderAction prompt_action(Key key)
{
    switch (key) {
    case KEY_PLAY:
        return (LadderAction){DOOM_INPUT_CONFIRM, DOOM_EDGE_PRESS};
    case KEY_BACK:
        return (LadderAction){DOOM_INPUT_ABORT, DOOM_EDGE_PRESS};
    default:
        return NO_ACTION;
    }
}

static LadderAction ladder_action(DoomControls *controls, Key key, DoomEvents *events)
{
    bool combined = controls->power_down;
    switch (controls->mode) {
    case DOOM_MODE_GAME:
        return combined ? game_combo_action(controls, key, events)
                        : game_action(controls, key, events);
    case DOOM_MODE_MENU:
    case DOOM_MODE_MENU_TOP:
        return combined ? menu_combo_action(key) : menu_action(controls->mode, key);
    case DOOM_MODE_PROMPT:
        return combined ? NO_ACTION : prompt_action(key);
    default:
        return NO_ACTION;
    }
}

static void ladder_release(DoomControls *controls, DoomEvents *events)
{
    if (!controls->ladder_down) {
        return;
    }
    controls->ladder_down = false;
    release(controls, controls->ladder_input, events);
    controls->ladder_input = DOOM_INPUT_NONE;
}

static void ladder_press(DoomControls *controls, Key key, DoomEvents *events)
{
    ladder_release(controls, events);
    controls->ladder_down = true;
    controls->ladder_key = key;
    controls->power_combined = controls->power_combined || controls->power_down;
    LadderAction action = ladder_action(controls, key, events);
    if (action.edge == DOOM_EDGE_TAP) {
        emit(events, action.input, DOOM_EDGE_TAP);
        return;
    }
    controls->ladder_input = action.input;
    press(controls, action.input, events);
}

static void power_key(DoomControls *controls, bool pressed, DoomEvents *events)
{
    if (pressed) {
        controls->power_down = true;
        controls->power_combined = false;
        return;
    }
    bool lone_tap = controls->power_down && !controls->power_combined;
    controls->power_down = false;
    if (lone_tap && controls->mode == DOOM_MODE_GAME) {
        emit(events, DOOM_INPUT_USE, DOOM_EDGE_TAP);
    }
}

void doom_controls_init(DoomControls *controls)
{
    *controls = (DoomControls){.mode = DOOM_MODE_GAME, .ladder_input = DOOM_INPUT_NONE};
}

void doom_controls_set_mode(DoomControls *controls, DoomMode mode, DoomEvents *events)
{
    if (mode == controls->mode) {
        return;
    }
    controls->mode = mode;
    controls->walking = false;
    controls->ladder_input = DOOM_INPUT_NONE;
    for (int input = DOOM_INPUT_NONE + 1; input < DOOM_INPUT_COUNT; input++) {
        if (input != DOOM_INPUT_RUN) {
            release(controls, (DoomInput)input, events);
        }
    }
}

void doom_controls_key(DoomControls *controls, Key key, bool pressed, DoomEvents *events)
{
    if (key == KEY_POWER) {
        power_key(controls, pressed, events);
        return;
    }
    if (pressed) {
        ladder_press(controls, key, events);
        return;
    }
    if (controls->ladder_down && controls->ladder_key == key) {
        ladder_release(controls, events);
    }
}
