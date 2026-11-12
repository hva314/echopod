#pragma once

#include <stdio.h>

FILE *doom_port_fopen(const char *path, const char *mode);
int doom_port_mkdir(const char *path, unsigned int mode);
