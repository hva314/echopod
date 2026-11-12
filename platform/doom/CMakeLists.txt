add_library(echo_doom_logic STATIC
    logic/doom_controls.c
    logic/doom_frame.c
    logic/doom_wad.c
)
target_include_directories(echo_doom_logic PUBLIC logic/include)
target_link_libraries(echo_doom_logic PUBLIC echo_hal PRIVATE echo_options)

if(NOT ECHO_BUILD_SIM OR WIN32)
    return()
endif()

set(ECHO_DOOMGENERIC_DIR ${PROJECT_SOURCE_DIR}/third_party/doomgeneric)
set(ECHO_DOOMGENERIC_SOURCES
    am_map.c d_event.c d_items.c d_iwad.c d_loop.c d_main.c d_mode.c d_net.c doomdef.c doomstat.c
    dstrings.c dummy.c f_finale.c f_wipe.c g_game.c hu_lib.c hu_stuff.c i_cdmus.c i_endoom.c
    i_input.c i_joystick.c i_scale.c i_sound.c i_system.c i_timer.c info.c m_argv.c m_bbox.c
    m_cheat.c m_config.c m_controls.c m_fixed.c m_menu.c m_misc.c m_random.c memio.c p_ceilng.c
    p_doors.c p_enemy.c p_floor.c p_inter.c p_lights.c p_map.c p_maputl.c p_mobj.c p_plats.c
    p_pspr.c p_saveg.c p_setup.c p_sight.c p_spec.c p_switch.c p_telept.c p_tick.c p_user.c
    r_bsp.c r_data.c r_draw.c r_main.c r_plane.c r_segs.c r_sky.c r_things.c s_sound.c sha1.c
    sounds.c st_lib.c st_stuff.c statdump.c tables.c v_video.c w_checksum.c w_file.c w_main.c
    w_wad.c wi_stuff.c z_zone.c
)
list(TRANSFORM ECHO_DOOMGENERIC_SOURCES PREPEND ${ECHO_DOOMGENERIC_DIR}/)

add_library(echo_doomgeneric OBJECT ${ECHO_DOOMGENERIC_SOURCES})
target_include_directories(echo_doomgeneric PRIVATE ${ECHO_DOOMGENERIC_DIR} port/include)
target_compile_options(echo_doomgeneric PRIVATE -w
    -include ${CMAKE_CURRENT_SOURCE_DIR}/port/include/doom/doom_port_libc.h)
target_compile_definitions(echo_doomgeneric PRIVATE NORMALUNIX LINUX _DEFAULT_SOURCE)
set_target_properties(echo_doomgeneric PROPERTIES C_EXTENSIONS ON)

add_library(echo_doom_port OBJECT
    port/doom_arena.c
    port/doom_input.c
    port/doom_port.c
    port/doom_video.c
    port/doom_wad_file.c
)
target_include_directories(echo_doom_port PUBLIC port/include)
target_include_directories(echo_doom_port SYSTEM PRIVATE ${ECHO_DOOMGENERIC_DIR})
target_compile_definitions(echo_doom_port PRIVATE
    ECHO_DOOM_ZONE_KB=${ECHO_DOOM_ZONE_KB}
    ECHO_DOOM_ARENA_KB=${ECHO_DOOM_ARENA_KB}
)
target_link_libraries(echo_doom_port PUBLIC echo_doom_logic echo_hal PRIVATE echo_options)

add_executable(echo_doom sim/doom_main.c)
target_compile_definitions(echo_doom PRIVATE SDL_MAIN_HANDLED)
target_link_libraries(echo_doom PRIVATE
    echo_doomgeneric echo_doom_port echo_doom_logic echo_sim_hal echo_sim_storage echo_sim_options
    echo_options m
)
