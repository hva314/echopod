#include "sim_doom.h"

#include "hal/doom.h"

void sim_doom_configure(const SimOptions *options)
{
    (void)options;
}

bool hal_doom_ready(void)
{
    return false;
}

void hal_doom_run(void)
{
}
