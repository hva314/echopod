#include "check.h"
#include "doom/doom_frame.h"

enum {
    PANEL_HEIGHT = 170,
    STRIP_ROWS = 2
};

static void the_first_and_last_panel_rows_show_the_first_and_last_frame_rows(void)
{
    CHECK_EQUAL(0, doom_frame_source_row(0, PANEL_HEIGHT));
    CHECK_EQUAL(DOOM_FRAME_HEIGHT - 1, doom_frame_source_row(PANEL_HEIGHT - 1, PANEL_HEIGHT));
}

static void panel_rows_step_down_the_frame_by_one_or_two_rows(void)
{
    for (int row = 1; row < PANEL_HEIGHT; row++) {
        int step =
            doom_frame_source_row(row, PANEL_HEIGHT) - doom_frame_source_row(row - 1, PANEL_HEIGHT);
        CHECK(step == 1 || step == 2);
    }
}

static void a_full_height_panel_shows_every_frame_row(void)
{
    for (int row = 0; row < DOOM_FRAME_HEIGHT; row++) {
        CHECK_EQUAL(row, doom_frame_source_row(row, DOOM_FRAME_HEIGHT));
    }
}

static void colors_pack_into_rgb565(void)
{
    CHECK_EQUAL(0x0000, doom_frame_rgb565(0, 0, 0));
    CHECK_EQUAL(0xFFFF, doom_frame_rgb565(255, 255, 255));
    CHECK_EQUAL(0xF800, doom_frame_rgb565(255, 0, 0));
    CHECK_EQUAL(0x07E0, doom_frame_rgb565(0, 255, 0));
    CHECK_EQUAL(0x001F, doom_frame_rgb565(0, 0, 255));
}

static void the_palette_goes_through_the_gamma_table(void)
{
    static uint8_t rgb[DOOM_FRAME_COLORS * 3];
    static uint8_t gamma[DOOM_FRAME_COLORS];
    static uint16_t palette[DOOM_FRAME_COLORS];
    for (int level = 0; level < DOOM_FRAME_COLORS; level++) {
        gamma[level] = (uint8_t)(255 - level);
    }
    rgb[3] = 255;
    doom_frame_palette(rgb, gamma, palette);
    CHECK_EQUAL(0xFFFF, palette[0]);
    CHECK_EQUAL(0x07FF, palette[1]);
}

static void a_strip_maps_panel_rows_to_frame_rows_through_the_palette(void)
{
    static uint8_t pixels[DOOM_FRAME_WIDTH * DOOM_FRAME_HEIGHT];
    static uint16_t palette[DOOM_FRAME_COLORS];
    static uint16_t out[DOOM_FRAME_WIDTH * STRIP_ROWS];
    int first = 100;
    int second_source = doom_frame_source_row(first + 1, PANEL_HEIGHT);
    pixels[doom_frame_source_row(first, PANEL_HEIGHT) * DOOM_FRAME_WIDTH + 5] = 7;
    pixels[second_source * DOOM_FRAME_WIDTH + DOOM_FRAME_WIDTH - 1] = 9;
    palette[7] = 0x1234;
    palette[9] = 0xABCD;
    DoomFrameSource source = {.pixels = pixels, .palette = palette};
    doom_frame_strip(&source, (DoomFrameRows){first, STRIP_ROWS}, PANEL_HEIGHT, out);
    CHECK_EQUAL(0x1234, out[5]);
    CHECK_EQUAL(0, out[6]);
    CHECK_EQUAL(0xABCD, out[DOOM_FRAME_WIDTH * 2 - 1]);
}

int main(void)
{
    static const TestCase cases[] = {
        TEST_CASE(the_first_and_last_panel_rows_show_the_first_and_last_frame_rows),
        TEST_CASE(panel_rows_step_down_the_frame_by_one_or_two_rows),
        TEST_CASE(a_full_height_panel_shows_every_frame_row),
        TEST_CASE(colors_pack_into_rgb565),
        TEST_CASE(the_palette_goes_through_the_gamma_table),
        TEST_CASE(a_strip_maps_panel_rows_to_frame_rows_through_the_palette),
    };
    return test_run_all(cases, TEST_CASE_COUNT(cases));
}
