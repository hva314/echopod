# doomgeneric

The DOOM engine behind the EchoPod DOOM program (`platform/doom`, `docs/doom.md`).

- Source: https://github.com/ozkl/doomgeneric at commit `e257a09527062bfb986b6e5e0780839b6cbd5b71`.
- Licence: GPL-2.0-or-later (`LICENSE`, and the header of every source file), compatible with EchoPod's GPL-3.0-or-later.
- Only the engine is kept: the platform back ends (`doomgeneric.c`, `doomgeneric_*.c`), `i_video.c`, `w_file_stdc.c`, the SDL and Allegro sound and music files, and the build files are left out. `platform/doom/port` replaces them.
- EchoPod's changes to the kept files are listed in `ECHOPOD_PATCHES.md`.
