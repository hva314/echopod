# EchoPod changes to doomgeneric

Each change keeps the engine talking only to `platform/doom/port`, so the same engine can later run on the player.

| File | Change | Why |
|---|---|---|
| `i_system.c` | `I_ZoneBase` returns `doom_port_zone()`. | The zone is a static array sized by `ECHO_DOOM_ZONE_KB`, not `malloc`. |
| `i_system.c` | `I_Quit` calls `doom_port_exit(0)`. `I_Error` prints its message, runs the on-error handlers, then calls `doom_port_exit(-1)`. | Quitting returns to the port's main through `longjmp` instead of `exit()`, without GUI pop-ups or `M_SaveDefaults` writing into the working directory. |
| `m_misc.c` | `M_FileExists` calls `doom_port_file_exists()`. | `-iwad /DOOM/x.wad` is a card path, checked through the HAL. |
| `m_menu.c` | `M_SaveGame` shows `DOOM_PORT_SAVE_UNAVAILABLE`. | Saving isn't wired to the HAL yet; without this a failed save ends the game with `I_Error`. |
| `m_menu.c` | Adds `doom_port_menu_state()`. | The controls need to know whether a menu, the top menu or a yes/no prompt is open. |

Every engine file is also compiled with `-include doom/doom_port_libc.h` (`platform/doom/CMakeLists.txt`). It routes `malloc`, `calloc`, `realloc`, `free` and `strdup` to the port's counted static arena (`ECHO_DOOM_ARENA_KB`), and `fopen` and `mkdir` to the port, which refuses them, so the engine never touches the host's files.
