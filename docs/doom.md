# DOOM

EchoPod can run DOOM in the simulator. It does not run on the player yet: this page says what works, how it is built, and what the player still needs.

## Playing it in the simulator

1. Put an IWAD in a folder called `DOOM` at the root of the card directory: `doom.wad`, `doom2.wad`, `doom1.wad` (the shareware episode), `freedoom1.wad` or `freedoom2.wad`. Case doesn't matter in the file name; the folder must be `DOOM`. If there are several, the first in that order is used. No game data comes with EchoPod. [Freedoom](https://freedoom.github.io) is free.
2. Run the simulator with that card: `./build/dev/platform/sim/echo_sim --card /path/to/card`.
3. Home shows **DOOM** above Settings. Choosing it pauses the music, saves the position and starts the DOOM program in its own window. Quitting DOOM brings the player back.

The DOOM program also runs on its own, with the simulator's options (`--card`, `--scale`, `--headless`, `--frames`, `--keys`, `--screenshot`):

```sh
./build/dev/platform/doom/echo_doom --card /path/to/card
./build/dev/platform/doom/echo_doom --headless --card /path/to/card --frames 240 \
    --keys play,play,play,play,menu,down:long --screenshot e1m1.bmp
```

Headless runs use the virtual clock, one game tic per frame, so a script and its screenshot are the same every time. At exit it prints the memory it used, for example `doom: zone peak 1468576/2097152 B, arena peak 126960/196608 B`.

Saving games isn't available yet: Save Game shows a message instead. DOOM's settings aren't kept between runs.

## Controls

The player has six buttons. Volume up, volume down, Play, Menu and Back share one resistor ladder, so only one of them can be read at a time. Power is separate and can be held with any of them. The controls are built around that: walking forward is a toggle, so you can turn and fire while you walk, and the less frequent actions are Power combinations.

| In the game | Does |
|---|---|
| Vol− / Vol+ (hold) | Turn left / right |
| Menu | Start or stop walking forward |
| Back (hold) | Walk backward (stops walking forward) |
| Play (hold) | Fire |
| Power (tap) | Use: open doors, press switches |
| Power + Vol− / Vol+ | Previous / next weapon |
| Power + Play | Automap |
| Power + Menu | Run on or off |
| Power + Back | DOOM's menu |

| In DOOM's menus | Does |
|---|---|
| Vol− / Vol+ | Up / down |
| Play | Choose; "yes" to a question |
| Back | Back one menu (closes the main menu); "no" to a question |
| Menu | Close the menu |
| Power + Vol− / Vol+ | Left / right on sliders |

On the title screen any button opens the menu. To leave, choose Quit Game and answer with Play. There is no strafing. In the simulator the buttons are the usual keys (arrows, Return, M, Backspace, P); a second ladder key pressed while one is held releases the first, as on the player.

## Picture

DOOM draws 320×200 with a 256-colour palette. The panel is 320×170, so every frame is squeezed to 170 rows by taking the nearest source row (each panel row samples the centre of its share of the frame). Everything stays visible and looks about 15 % flatter. Frames go to the panel in 34-row strips of RGB565 through `hal_display_write`.

## How it is built

DOOM runs as its own program, not inside the player, because on the player it can't fit beside the music player (see below). The pieces:

| Path | What |
|---|---|
| `third_party/doomgeneric` | The engine: [doomgeneric](https://github.com/ozkl/doomgeneric) at `e257a09`, on Chocolate Doom (GPL-2.0-or-later). Only the engine files are kept; EchoPod's few changes are in its `ECHOPOD_PATCHES.md`. |
| `platform/doom/logic` | Pure, host-tested logic: the controls (`doom_controls`), the frame conversion (`doom_frame`) and the WAD choice (`doom_wad`). |
| `platform/doom/port` | The engine's platform layer, written against the HAL only: the zone and allocation arena, quitting, video, input and WAD reads through `hal_file`. |
| `platform/doom/sim` | `echo_doom`, the simulator's `main()` for the program, on the simulator's display, input, clock and storage (`echo_sim_hal`, `echo_sim_storage`). |
| `src/hal/include/hal/doom.h` | `hal_doom_ready()` and `hal_doom_run()`. The simulator starts `echo_doom` as a child process with the same card and scale and waits (`platform/sim/sim_doom.c`); a headless simulator gives it `--headless --frames 35`. |
| `src/app/doom_launch.c` | The Home row. Device builds compile it out (`ECHO_DOOM_HIDDEN`, `doom_launch_none.c`), so the firmware is unchanged. |

The engine never uses the host's files or heap directly. Every engine file is compiled with `-include doom/doom_port_libc.h`, which sends `malloc`, `calloc`, `realloc`, `free` and `strdup` to a counted static arena and refuses `fopen` and `mkdir`. The zone is a static array. Both are sized at build time:

| CMake cache variable | Default | |
|---|---|---|
| `ECHO_DOOM_ZONE_KB` | 2048 | The engine's zone |
| `ECHO_DOOM_ARENA_KB` | 192 | Everything else the engine allocates (mostly the WAD's lump directory) |
| `ECHO_DOOM_CARD` | unset | A card directory with `DOOM/<iwad>`; when set, ctest runs `doom_plays_headless`, a scripted new game into E1M1 |

Input goes from `hal_input_next_transition` through `doom_controls` to DOOM key events. A key's release reaches the engine at least two tics after its press, so a tap (Power for Use, the Power combinations) is always seen by the tic that builds the player's command.

## Memory: what the player would need

Measured on 2026-11-12 with Freedoom 0.13.0 `freedoom1.wad`.

On the simulator (64-bit Linux host, so pointers are 8 bytes):

| | Bytes |
|---|---|
| Arena peak (lump directory and small allocations) | 126,960 |
| Zone peak, title screen and demo (non-purgeable blocks) | 859,808 |
| Zone peak, new game into E1M1, walking, turning, firing | 1,468,576 |
| Smallest zone that plays that E1M1 script | between 1,536 KB (fails) and 1,664 KB (plays) |

Cross-compiled for the Cortex-M3 (`arm-none-eabi-gcc` 14.2.1, `-Os -mthumb`, totals of the object files before linking, so an upper bound):

| | Code | Initialised data | Zeroed data |
|---|---|---|---|
| Engine | 216,439 (66,852 of it the trig tables in `tables.c`) | 59,971 (40,236 of it the state and thing tables in `info.c`) | 242,504 (134,152 of it `visplanes` in `r_plane.c`) |
| Port and logic | 2,854 | 0 | 24,876, plus the zone and arena |

The player has about 636 KB of RAM on core 0 (`0x03000000`–`0x0309EFFF`, including the 16 KB stack) and 176 KB on core 1. The unchanged engine's code and static data come to about 546 KB before any zone, and the zone a level needs is well over 1 MB on the host. The 32-bit figure will be lower, but it hasn't been measured yet. So the stock engine does not fit, and playing on the player needs memory work first, the kind other microcontroller ports of DOOM have done:

- fewer and smaller `visplanes`, `drawsegs` and openings;
- the constant tables and states kept out of RAM, or made smaller;
- WAD lumps read from the card when needed instead of cached, and compact level structures;
- a smaller IWAD: Freedoom 1 has 3,163 lumps, the shareware `doom1.wad` far fewer (not measured here).

## Not done yet

- **On the player:** a loader that releases audio, loads the program from the image or the card over the player's RAM, runs it and restarts the player when it quits; the memory work above; a small libc for the engine; and a check of the stack against the engine's recursion (BSP walk, sight checks). Nothing here has run on hardware.
- **Saving games and DOOM's settings,** in `/.echo/doom/` through the HAL.
- **Sound effects** through the audio engine. Music is further off.
