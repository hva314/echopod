#include <stdbool.h>

#include "doom/doom_controls.h"
#include "doom/doom_port.h"
#include "doomkeys.h"
#include "hal/input.h"
#include "i_timer.h"
#include "m_controls.h"

enum {
    READY_CAPACITY = 64,
    PENDING_CAPACITY = 32,
    KEY_CODES = 256,
    MIN_HOLD_TICS = 2,
    PREVIOUS_WEAPON_KEY = '[',
    NEXT_WEAPON_KEY = ']'
};

typedef struct KeyEvent {
    unsigned char key;
    bool pressed;
} KeyEvent;

typedef struct PendingRelease {
    unsigned char key;
    int due_tic;
} PendingRelease;

typedef struct DoomInputState {
    DoomControls controls;
    KeyEvent ready[READY_CAPACITY];
    int ready_head;
    int ready_count;
    PendingRelease pending[PENDING_CAPACITY];
    int pending_count;
    int pressed_tic[KEY_CODES];
    bool draining;
    bool started;
} DoomInputState;

static DoomInputState state;

static const unsigned char INPUT_KEYS[DOOM_INPUT_COUNT] = {
    [DOOM_INPUT_FORWARD] = KEY_UPARROW,
    [DOOM_INPUT_BACKWARD] = KEY_DOWNARROW,
    [DOOM_INPUT_TURN_LEFT] = KEY_LEFTARROW,
    [DOOM_INPUT_TURN_RIGHT] = KEY_RIGHTARROW,
    [DOOM_INPUT_FIRE] = KEY_FIRE,
    [DOOM_INPUT_USE] = KEY_USE,
    [DOOM_INPUT_RUN] = KEY_RSHIFT,
    [DOOM_INPUT_PREVIOUS_WEAPON] = PREVIOUS_WEAPON_KEY,
    [DOOM_INPUT_NEXT_WEAPON] = NEXT_WEAPON_KEY,
    [DOOM_INPUT_AUTOMAP] = KEY_TAB,
    [DOOM_INPUT_ESCAPE] = KEY_ESCAPE,
    [DOOM_INPUT_MENU_UP] = KEY_UPARROW,
    [DOOM_INPUT_MENU_DOWN] = KEY_DOWNARROW,
    [DOOM_INPUT_MENU_LEFT] = KEY_LEFTARROW,
    [DOOM_INPUT_MENU_RIGHT] = KEY_RIGHTARROW,
    [DOOM_INPUT_MENU_SELECT] = KEY_ENTER,
    [DOOM_INPUT_MENU_BACK] = KEY_BACKSPACE,
    [DOOM_INPUT_CONFIRM] = 'y',
    [DOOM_INPUT_ABORT] = 'n',
};

static void push_ready(unsigned char key, bool pressed)
{
    if (state.ready_count >= READY_CAPACITY) {
        return;
    }
    int tail = (state.ready_head + state.ready_count) % READY_CAPACITY;
    state.ready[tail] = (KeyEvent){key, pressed};
    state.ready_count++;
}

static bool pop_ready(KeyEvent *event)
{
    if (state.ready_count == 0) {
        return false;
    }
    *event = state.ready[state.ready_head];
    state.ready_head = (state.ready_head + 1) % READY_CAPACITY;
    state.ready_count--;
    return true;
}

static void press_key(unsigned char key)
{
    state.pressed_tic[key] = I_GetTime();
    push_ready(key, true);
}

static void release_key(unsigned char key)
{
    if (state.pending_count >= PENDING_CAPACITY) {
        push_ready(key, false);
        return;
    }
    state.pending[state.pending_count++] =
        (PendingRelease){key, state.pressed_tic[key] + MIN_HOLD_TICS};
}

static void release_due_keys(void)
{
    int now = I_GetTime();
    int kept = 0;
    for (int index = 0; index < state.pending_count; index++) {
        PendingRelease release = state.pending[index];
        if (now >= release.due_tic) {
            push_ready(release.key, false);
        } else {
            state.pending[kept++] = release;
        }
    }
    state.pending_count = kept;
}

static void queue_events(DoomEvents *events)
{
    for (int index = 0; index < events->count; index++) {
        DoomInputEvent event = events->items[index];
        unsigned char key = INPUT_KEYS[event.input];
        if (event.edge != DOOM_EDGE_RELEASE) {
            press_key(key);
        }
        if (event.edge != DOOM_EDGE_PRESS) {
            release_key(key);
        }
    }
    events->count = 0;
}

static DoomMode current_mode(void)
{
    switch (doom_port_menu_state()) {
    case DOOM_PORT_MENU_PROMPT:
        return DOOM_MODE_PROMPT;
    case DOOM_PORT_MENU_TOP:
        return DOOM_MODE_MENU_TOP;
    case DOOM_PORT_MENU_INNER:
        return DOOM_MODE_MENU;
    default:
        return DOOM_MODE_GAME;
    }
}

static void poll_buttons(void)
{
    DoomEvents events = {.count = 0};
    doom_controls_set_mode(&state.controls, current_mode(), &events);
    queue_events(&events);
    HalKeyTransition transition;
    while (hal_input_next_transition(&transition)) {
        doom_controls_key(&state.controls, transition.key, transition.pressed, &events);
        queue_events(&events);
    }
}

static void start(void)
{
    doom_controls_init(&state.controls);
    key_prevweapon = PREVIOUS_WEAPON_KEY;
    key_nextweapon = NEXT_WEAPON_KEY;
    state.started = true;
}

int DG_GetKey(int *pressed, unsigned char *key)
{
    if (!state.started) {
        start();
    }
    if (!state.draining) {
        poll_buttons();
        state.draining = true;
    }
    release_due_keys();
    KeyEvent event;
    if (!pop_ready(&event)) {
        state.draining = false;
        return 0;
    }
    *pressed = event.pressed ? 1 : 0;
    *key = event.key;
    return 1;
}
