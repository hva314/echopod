#pragma once

#include <stdbool.h>

#include "core/key.h"

typedef enum DoomInput {
    DOOM_INPUT_NONE,
    DOOM_INPUT_FORWARD,
    DOOM_INPUT_BACKWARD,
    DOOM_INPUT_TURN_LEFT,
    DOOM_INPUT_TURN_RIGHT,
    DOOM_INPUT_FIRE,
    DOOM_INPUT_USE,
    DOOM_INPUT_RUN,
    DOOM_INPUT_PREVIOUS_WEAPON,
    DOOM_INPUT_NEXT_WEAPON,
    DOOM_INPUT_AUTOMAP,
    DOOM_INPUT_ESCAPE,
    DOOM_INPUT_MENU_UP,
    DOOM_INPUT_MENU_DOWN,
    DOOM_INPUT_MENU_LEFT,
    DOOM_INPUT_MENU_RIGHT,
    DOOM_INPUT_MENU_SELECT,
    DOOM_INPUT_MENU_BACK,
    DOOM_INPUT_CONFIRM,
    DOOM_INPUT_ABORT,
    DOOM_INPUT_COUNT
} DoomInput;

typedef enum DoomMode {
    DOOM_MODE_GAME,
    DOOM_MODE_MENU,
    DOOM_MODE_MENU_TOP,
    DOOM_MODE_PROMPT
} DoomMode;

typedef enum DoomEdge {
    DOOM_EDGE_PRESS,
    DOOM_EDGE_RELEASE,
    DOOM_EDGE_TAP
} DoomEdge;

typedef struct DoomInputEvent {
    DoomInput input;
    DoomEdge edge;
} DoomInputEvent;

enum {
    DOOM_EVENTS_CAPACITY = DOOM_INPUT_COUNT + 4
};

typedef struct DoomEvents {
    DoomInputEvent items[DOOM_EVENTS_CAPACITY];
    int count;
} DoomEvents;

typedef struct DoomControls {
    DoomMode mode;
    bool held[DOOM_INPUT_COUNT];
    bool ladder_down;
    Key ladder_key;
    DoomInput ladder_input;
    bool power_down;
    bool power_combined;
    bool walking;
} DoomControls;

void doom_controls_init(DoomControls *controls);
void doom_controls_set_mode(DoomControls *controls, DoomMode mode, DoomEvents *events);
void doom_controls_key(DoomControls *controls, Key key, bool pressed, DoomEvents *events);
