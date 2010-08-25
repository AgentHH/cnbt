#pragma once
// {{{ #includes
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <png.h>

#include "chunk.hpp"
#include "minecraft.hpp"
// }}}
#define ERR(args...) fprintf(stderr, args)

namespace cnbt {
int write_png_to_file(uint8_t *buf, size_t w, size_t h, const char *filename);

// all of the render functions expect x, z, w, and h to be in chunk coordinates
// w and h are used to indicate how many chunks wide / tall the image buffer is
void render_top_down(struct chunk *c, uint8_t *buf, int32_t x, int32_t z, int32_t w, int32_t h, game::entitymap &em);

enum rendertype {
    RENDER_TOP_DOWN,
    RENDER_OBLIQUE,
    RENDER_OBLIQUE_ANGLED,
};

struct renderer {
    struct chunkmanager *cm;
    uint8_t dir; // 0 is north, increases in clockwise direction

    renderer(struct chunkmanager *cm, rendertype rt, uint8_t dir);

    std::pair<size_t, size_t> image_bounds(int32_t x, int32_t z, size_t w, size_t h);
    uint8_t *render(int32_t x, int32_t z, size_t w, size_t h);
};

} // end namespace cnbt
