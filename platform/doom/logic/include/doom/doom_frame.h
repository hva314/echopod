#pragma once

#include <stdint.h>

enum {
    DOOM_FRAME_WIDTH = 320,
    DOOM_FRAME_HEIGHT = 200,
    DOOM_FRAME_COLORS = 256
};

typedef struct DoomFrameSource {
    const uint8_t *pixels;
    const uint16_t *palette;
} DoomFrameSource;

typedef struct DoomFrameRows {
    int first;
    int count;
} DoomFrameRows;

int doom_frame_source_row(int display_row, int display_height);
uint16_t doom_frame_rgb565(uint8_t red, uint8_t green, uint8_t blue);
void doom_frame_palette(const uint8_t *rgb, const uint8_t *gamma, uint16_t *palette);
void doom_frame_strip(const DoomFrameSource *source, DoomFrameRows rows, int display_height,
                      uint16_t *out);
