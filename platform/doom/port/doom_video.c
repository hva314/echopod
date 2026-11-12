#include <limits.h>
#include <string.h>

#include "doom/doom_frame.h"
#include "doom/doom_run.h"
#include "doomtype.h"
#include "hal/display.h"
#include "i_video.h"
#include "tables.h"
#include "z_zone.h"

enum {
    STRIP_ROWS = 34,
    CHANNEL_MAX = 255
};

typedef struct DoomVideo {
    uint16_t palette[DOOM_FRAME_COLORS];
    uint16_t strip[DOOM_FRAME_WIDTH * STRIP_ROWS];
} DoomVideo;

static DoomVideo video;

byte *I_VideoBuffer = NULL;
boolean screensaver_mode = false;
boolean screenvisible = false;
float mouse_acceleration = 2.0f;
int mouse_threshold = 10;
int usegamma = 0;
int usemouse = 0;

void I_GetEvent(void);
void I_InitInput(void);

void I_InitGraphics(void)
{
    I_VideoBuffer = Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);
    screenvisible = true;
    hal_display_set_rotation(HAL_DISPLAY_LANDSCAPE);
    I_InitInput();
}

void I_ShutdownGraphics(void)
{
    Z_Free(I_VideoBuffer);
}

void I_StartFrame(void)
{
}

void I_StartTic(void)
{
    I_GetEvent();
}

void I_UpdateNoBlit(void)
{
}

void I_FinishUpdate(void)
{
    doom_run_sample_memory();
    DoomFrameSource source = {.pixels = I_VideoBuffer, .palette = video.palette};
    for (int first = 0; first < HAL_DISPLAY_HEIGHT; first += STRIP_ROWS) {
        int count =
            HAL_DISPLAY_HEIGHT - first < STRIP_ROWS ? HAL_DISPLAY_HEIGHT - first : STRIP_ROWS;
        doom_frame_strip(&source, (DoomFrameRows){first, count}, HAL_DISPLAY_HEIGHT, video.strip);
        hal_display_write(0, first, DOOM_FRAME_WIDTH, count, video.strip);
    }
    hal_display_present();
}

void I_ReadScreen(byte *scr)
{
    memcpy(scr, I_VideoBuffer, SCREENWIDTH * SCREENHEIGHT);
}

void I_SetPalette(byte *palette)
{
    doom_frame_palette(palette, gammatable[usegamma], video.palette);
}

static int channel(uint16_t color, int shift, int bits)
{
    int level = (color >> shift) & ((1 << bits) - 1);
    return level * CHANNEL_MAX / ((1 << bits) - 1);
}

static int distance(uint16_t color, int red, int green, int blue)
{
    int dr = channel(color, 11, 5) - red;
    int dg = channel(color, 5, 6) - green;
    int db = channel(color, 0, 5) - blue;
    return dr * dr + dg * dg + db * db;
}

int I_GetPaletteIndex(int r, int g, int b)
{
    int best = 0;
    int best_distance = INT_MAX;
    for (int index = 0; index < DOOM_FRAME_COLORS; index++) {
        int candidate = distance(video.palette[index], r, g, b);
        if (candidate < best_distance) {
            best = index;
            best_distance = candidate;
        }
    }
    return best;
}

void I_BeginRead(void)
{
}

void I_EndRead(void)
{
}

void I_SetWindowTitle(char *title)
{
    (void)title;
}

void I_GraphicsCheckCommandLine(void)
{
}

void I_SetGrabMouseCallback(grabmouse_callback_t func)
{
    (void)func;
}

void I_EnableLoadingDisk(void)
{
}

void I_BindVideoVariables(void)
{
}

void I_DisplayFPSDots(boolean dots_on)
{
    (void)dots_on;
}

void I_CheckIsScreensaver(void)
{
}

void I_InitWindowTitle(void)
{
}

void I_InitWindowIcon(void)
{
}
