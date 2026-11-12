#include "check.h"
#include "doom/doom_controls.h"

typedef struct Rig {
    DoomControls controls;
    DoomEvents events;
} Rig;

static Rig rig_in(DoomMode mode)
{
    Rig rig;
    doom_controls_init(&rig.controls);
    rig.events.count = 0;
    doom_controls_set_mode(&rig.controls, mode, &rig.events);
    rig.events.count = 0;
    return rig;
}

static void key(Rig *rig, Key pressed_key, bool pressed)
{
    rig->events.count = 0;
    doom_controls_key(&rig->controls, pressed_key, pressed, &rig->events);
}

static void check_event(const Rig *rig, int index, DoomInput input, DoomEdge edge)
{
    CHECK(index < rig->events.count);
    CHECK_EQUAL(input, rig->events.items[index].input);
    CHECK_EQUAL(edge, rig->events.items[index].edge);
}

static void check_hold(DoomMode mode, Key held_key, DoomInput input)
{
    Rig rig = rig_in(mode);
    key(&rig, held_key, true);
    CHECK_EQUAL(1, rig.events.count);
    check_event(&rig, 0, input, DOOM_EDGE_PRESS);
    key(&rig, held_key, false);
    CHECK_EQUAL(1, rig.events.count);
    check_event(&rig, 0, input, DOOM_EDGE_RELEASE);
}

static void check_combo_tap(DoomMode mode, Key combo_key, DoomInput input)
{
    Rig rig = rig_in(mode);
    key(&rig, KEY_POWER, true);
    key(&rig, combo_key, true);
    CHECK_EQUAL(1, rig.events.count);
    check_event(&rig, 0, input, DOOM_EDGE_TAP);
    key(&rig, combo_key, false);
    CHECK_EQUAL(0, rig.events.count);
    key(&rig, KEY_POWER, false);
    CHECK_EQUAL(0, rig.events.count);
}

static void volume_keys_turn_while_held_in_game(void)
{
    check_hold(DOOM_MODE_GAME, KEY_VOLUME_DOWN, DOOM_INPUT_TURN_LEFT);
    check_hold(DOOM_MODE_GAME, KEY_VOLUME_UP, DOOM_INPUT_TURN_RIGHT);
}

static void play_fires_while_held_in_game(void)
{
    check_hold(DOOM_MODE_GAME, KEY_PLAY, DOOM_INPUT_FIRE);
}

static void back_walks_backward_while_held_in_game(void)
{
    check_hold(DOOM_MODE_GAME, KEY_BACK, DOOM_INPUT_BACKWARD);
}

static void menu_starts_and_stops_walking_forward(void)
{
    Rig rig = rig_in(DOOM_MODE_GAME);
    key(&rig, KEY_MENU, true);
    CHECK_EQUAL(1, rig.events.count);
    check_event(&rig, 0, DOOM_INPUT_FORWARD, DOOM_EDGE_PRESS);
    key(&rig, KEY_MENU, false);
    CHECK_EQUAL(0, rig.events.count);
    key(&rig, KEY_MENU, true);
    CHECK_EQUAL(1, rig.events.count);
    check_event(&rig, 0, DOOM_INPUT_FORWARD, DOOM_EDGE_RELEASE);
}

static void turning_keeps_walking(void)
{
    Rig rig = rig_in(DOOM_MODE_GAME);
    key(&rig, KEY_MENU, true);
    key(&rig, KEY_MENU, false);
    key(&rig, KEY_VOLUME_UP, true);
    CHECK_EQUAL(1, rig.events.count);
    check_event(&rig, 0, DOOM_INPUT_TURN_RIGHT, DOOM_EDGE_PRESS);
    CHECK(rig.controls.held[DOOM_INPUT_FORWARD]);
}

static void back_stops_walking_before_walking_backward(void)
{
    Rig rig = rig_in(DOOM_MODE_GAME);
    key(&rig, KEY_MENU, true);
    key(&rig, KEY_MENU, false);
    key(&rig, KEY_BACK, true);
    CHECK_EQUAL(2, rig.events.count);
    check_event(&rig, 0, DOOM_INPUT_FORWARD, DOOM_EDGE_RELEASE);
    check_event(&rig, 1, DOOM_INPUT_BACKWARD, DOOM_EDGE_PRESS);
    CHECK(!rig.controls.walking);
}

static void a_second_ladder_key_releases_the_first(void)
{
    Rig rig = rig_in(DOOM_MODE_GAME);
    key(&rig, KEY_PLAY, true);
    key(&rig, KEY_VOLUME_DOWN, true);
    CHECK_EQUAL(2, rig.events.count);
    check_event(&rig, 0, DOOM_INPUT_FIRE, DOOM_EDGE_RELEASE);
    check_event(&rig, 1, DOOM_INPUT_TURN_LEFT, DOOM_EDGE_PRESS);
    key(&rig, KEY_PLAY, false);
    CHECK_EQUAL(0, rig.events.count);
    key(&rig, KEY_VOLUME_DOWN, false);
    CHECK_EQUAL(1, rig.events.count);
    check_event(&rig, 0, DOOM_INPUT_TURN_LEFT, DOOM_EDGE_RELEASE);
}

static void a_lone_power_tap_is_use(void)
{
    Rig rig = rig_in(DOOM_MODE_GAME);
    key(&rig, KEY_POWER, true);
    CHECK_EQUAL(0, rig.events.count);
    key(&rig, KEY_POWER, false);
    CHECK_EQUAL(1, rig.events.count);
    check_event(&rig, 0, DOOM_INPUT_USE, DOOM_EDGE_TAP);
}

static void power_with_a_ladder_key_is_not_use(void)
{
    Rig rig = rig_in(DOOM_MODE_GAME);
    key(&rig, KEY_POWER, true);
    key(&rig, KEY_VOLUME_UP, true);
    key(&rig, KEY_VOLUME_UP, false);
    key(&rig, KEY_POWER, false);
    CHECK_EQUAL(0, rig.events.count);
}

static void power_pressed_while_firing_is_still_use(void)
{
    Rig rig = rig_in(DOOM_MODE_GAME);
    key(&rig, KEY_PLAY, true);
    key(&rig, KEY_POWER, true);
    key(&rig, KEY_POWER, false);
    CHECK_EQUAL(1, rig.events.count);
    check_event(&rig, 0, DOOM_INPUT_USE, DOOM_EDGE_TAP);
    CHECK(rig.controls.held[DOOM_INPUT_FIRE]);
}

static void power_with_volume_switches_weapons(void)
{
    check_combo_tap(DOOM_MODE_GAME, KEY_VOLUME_DOWN, DOOM_INPUT_PREVIOUS_WEAPON);
    check_combo_tap(DOOM_MODE_GAME, KEY_VOLUME_UP, DOOM_INPUT_NEXT_WEAPON);
}

static void power_with_play_toggles_the_automap(void)
{
    check_combo_tap(DOOM_MODE_GAME, KEY_PLAY, DOOM_INPUT_AUTOMAP);
}

static void power_with_back_opens_the_doom_menu(void)
{
    check_combo_tap(DOOM_MODE_GAME, KEY_BACK, DOOM_INPUT_ESCAPE);
}

static void power_with_menu_toggles_running(void)
{
    Rig rig = rig_in(DOOM_MODE_GAME);
    key(&rig, KEY_POWER, true);
    key(&rig, KEY_MENU, true);
    CHECK_EQUAL(1, rig.events.count);
    check_event(&rig, 0, DOOM_INPUT_RUN, DOOM_EDGE_PRESS);
    key(&rig, KEY_MENU, false);
    key(&rig, KEY_MENU, true);
    CHECK_EQUAL(1, rig.events.count);
    check_event(&rig, 0, DOOM_INPUT_RUN, DOOM_EDGE_RELEASE);
}

static void volume_keys_move_through_menus(void)
{
    check_hold(DOOM_MODE_MENU, KEY_VOLUME_DOWN, DOOM_INPUT_MENU_UP);
    check_hold(DOOM_MODE_MENU, KEY_VOLUME_UP, DOOM_INPUT_MENU_DOWN);
}

static void play_selects_in_menus(void)
{
    check_hold(DOOM_MODE_MENU, KEY_PLAY, DOOM_INPUT_MENU_SELECT);
}

static void back_goes_up_a_menu_and_closes_the_top_one(void)
{
    check_hold(DOOM_MODE_MENU, KEY_BACK, DOOM_INPUT_MENU_BACK);
    check_hold(DOOM_MODE_MENU_TOP, KEY_BACK, DOOM_INPUT_ESCAPE);
}

static void menu_closes_the_doom_menu(void)
{
    check_hold(DOOM_MODE_MENU, KEY_MENU, DOOM_INPUT_ESCAPE);
}

static void power_with_volume_moves_sliders(void)
{
    check_combo_tap(DOOM_MODE_MENU, KEY_VOLUME_DOWN, DOOM_INPUT_MENU_LEFT);
    check_combo_tap(DOOM_MODE_MENU, KEY_VOLUME_UP, DOOM_INPUT_MENU_RIGHT);
}

static void a_power_tap_does_nothing_in_menus(void)
{
    Rig rig = rig_in(DOOM_MODE_MENU);
    key(&rig, KEY_POWER, true);
    key(&rig, KEY_POWER, false);
    CHECK_EQUAL(0, rig.events.count);
}

static void play_answers_yes_and_back_answers_no(void)
{
    check_hold(DOOM_MODE_PROMPT, KEY_PLAY, DOOM_INPUT_CONFIRM);
    check_hold(DOOM_MODE_PROMPT, KEY_BACK, DOOM_INPUT_ABORT);
}

static void a_mode_change_releases_held_inputs_but_keeps_running(void)
{
    Rig rig = rig_in(DOOM_MODE_GAME);
    key(&rig, KEY_POWER, true);
    key(&rig, KEY_MENU, true);
    key(&rig, KEY_MENU, false);
    key(&rig, KEY_POWER, false);
    key(&rig, KEY_MENU, true);
    key(&rig, KEY_MENU, false);
    key(&rig, KEY_PLAY, true);
    rig.events.count = 0;
    doom_controls_set_mode(&rig.controls, DOOM_MODE_MENU, &rig.events);
    CHECK_EQUAL(2, rig.events.count);
    check_event(&rig, 0, DOOM_INPUT_FORWARD, DOOM_EDGE_RELEASE);
    check_event(&rig, 1, DOOM_INPUT_FIRE, DOOM_EDGE_RELEASE);
    CHECK(rig.controls.held[DOOM_INPUT_RUN]);
    CHECK(!rig.controls.walking);
}

static void a_key_held_across_a_mode_change_releases_nothing_later(void)
{
    Rig rig = rig_in(DOOM_MODE_MENU);
    key(&rig, KEY_PLAY, true);
    rig.events.count = 0;
    doom_controls_set_mode(&rig.controls, DOOM_MODE_GAME, &rig.events);
    CHECK_EQUAL(1, rig.events.count);
    check_event(&rig, 0, DOOM_INPUT_MENU_SELECT, DOOM_EDGE_RELEASE);
    key(&rig, KEY_PLAY, false);
    CHECK_EQUAL(0, rig.events.count);
}

static void setting_the_same_mode_emits_nothing(void)
{
    Rig rig = rig_in(DOOM_MODE_GAME);
    key(&rig, KEY_PLAY, true);
    rig.events.count = 0;
    doom_controls_set_mode(&rig.controls, DOOM_MODE_GAME, &rig.events);
    CHECK_EQUAL(0, rig.events.count);
    CHECK(rig.controls.held[DOOM_INPUT_FIRE]);
}

int main(void)
{
    static const TestCase cases[] = {
        TEST_CASE(volume_keys_turn_while_held_in_game),
        TEST_CASE(play_fires_while_held_in_game),
        TEST_CASE(back_walks_backward_while_held_in_game),
        TEST_CASE(menu_starts_and_stops_walking_forward),
        TEST_CASE(turning_keeps_walking),
        TEST_CASE(back_stops_walking_before_walking_backward),
        TEST_CASE(a_second_ladder_key_releases_the_first),
        TEST_CASE(a_lone_power_tap_is_use),
        TEST_CASE(power_with_a_ladder_key_is_not_use),
        TEST_CASE(power_pressed_while_firing_is_still_use),
        TEST_CASE(power_with_volume_switches_weapons),
        TEST_CASE(power_with_play_toggles_the_automap),
        TEST_CASE(power_with_back_opens_the_doom_menu),
        TEST_CASE(power_with_menu_toggles_running),
        TEST_CASE(volume_keys_move_through_menus),
        TEST_CASE(play_selects_in_menus),
        TEST_CASE(back_goes_up_a_menu_and_closes_the_top_one),
        TEST_CASE(menu_closes_the_doom_menu),
        TEST_CASE(power_with_volume_moves_sliders),
        TEST_CASE(a_power_tap_does_nothing_in_menus),
        TEST_CASE(play_answers_yes_and_back_answers_no),
        TEST_CASE(a_mode_change_releases_held_inputs_but_keeps_running),
        TEST_CASE(a_key_held_across_a_mode_change_releases_nothing_later),
        TEST_CASE(setting_the_same_mode_emits_nothing),
    };
    return test_run_all(cases, TEST_CASE_COUNT(cases));
}
