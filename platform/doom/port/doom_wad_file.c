#include <stdbool.h>

#include "hal/storage.h"
#include "w_file.h"

enum {
    MAX_OPEN_WADS = 4
};

typedef struct HalWadFile {
    wad_file_t wad;
    HalFile file;
    bool open;
} HalWadFile;

static HalWadFile wads[MAX_OPEN_WADS];

extern wad_file_class_t stdc_wad_file;

static HalWadFile *free_slot(void)
{
    for (int index = 0; index < MAX_OPEN_WADS; index++) {
        if (!wads[index].open) {
            return &wads[index];
        }
    }
    return NULL;
}

static wad_file_t *open_wad(char *path)
{
    HalWadFile *slot = free_slot();
    if (slot == NULL || !hal_file_open(&slot->file, path)) {
        return NULL;
    }
    slot->open = true;
    slot->wad.file_class = &stdc_wad_file;
    slot->wad.mapped = NULL;
    slot->wad.length = slot->file.size;
    return &slot->wad;
}

static void close_wad(wad_file_t *wad)
{
    HalWadFile *slot = (HalWadFile *)wad;
    hal_file_close(&slot->file);
    slot->open = false;
}

static size_t read_wad(wad_file_t *wad, unsigned int offset, void *buffer, size_t buffer_len)
{
    HalWadFile *slot = (HalWadFile *)wad;
    if (offset >= slot->file.size) {
        return 0;
    }
    size_t available = slot->file.size - offset;
    size_t length = buffer_len < available ? buffer_len : available;
    if (!hal_file_read(slot->file, offset, buffer, (uint32_t)length)) {
        return 0;
    }
    return length;
}

wad_file_class_t stdc_wad_file = {
    .OpenFile = open_wad,
    .CloseFile = close_wad,
    .Read = read_wad,
};
