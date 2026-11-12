#include "doom/doom_frame.h"

enum {
    RGB_CHANNELS = 3
};

int doom_frame_source_row(int display_row, int display_height)
{
    return (2 * display_row + 1) * DOOM_FRAME_HEIGHT / (2 * display_height);
}

uint16_t doom_frame_rgb565(uint8_t red, uint8_t green, uint8_t blue)
{
    return (uint16_t)(((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3));
}

void doom_frame_palette(const uint8_t *rgb, const uint8_t *gamma, uint16_t *palette)
{
    for (int index = 0; index < DOOM_FRAME_COLORS; index++) {
        const uint8_t *color = &rgb[index * RGB_CHANNELS];
        palette[index] = doom_frame_rgb565(gamma[color[0]], gamma[color[1]], gamma[color[2]]);
    }
}

static void convert_row(const DoomFrameSource *source, int source_row, uint16_t *out)
{
    const uint8_t *pixels = &source->pixels[source_row * DOOM_FRAME_WIDTH];
    for (int column = 0; column < DOOM_FRAME_WIDTH; column++) {
        out[column] = source->palette[pixels[column]];
    }
}

void doom_frame_strip(const DoomFrameSource *source, DoomFrameRows rows, int display_height,
                      uint16_t *out)
{
    for (int row = 0; row < rows.count; row++) {
        int source_row = doom_frame_source_row(rows.first + row, display_height);
        convert_row(source, source_row, &out[row * DOOM_FRAME_WIDTH]);
    }
}
