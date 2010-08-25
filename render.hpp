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

#define BLOCKS_PER_X 16
#define BLOCKS_PER_Y 128
#define BLOCKS_PER_Z 16

#define DIR_NORTH 0
#define DIR_EAST 64
#define DIR_SOUTH 128
#define DIR_WEST 192

namespace cnbt {
int write_png_to_file(uint8_t *buf, size_t w, size_t h, const char *filename);

// all of the render functions expect x, z, w, and h to be in chunk coordinates
// w and h are used to indicate how many chunks wide / tall the image buffer is
void render_top_down(struct chunk *c, uint8_t *buf, coord chunk, coord dim, coord imagesize, uint8_t dir, game::entitymap &em);

enum rendertype {
    RENDER_TOP_DOWN,
    RENDER_OBLIQUE,
    RENDER_OBLIQUE_ANGLED,
};

struct renderer {
    struct chunkmanager *cm;
    rendertype rt;
    uint8_t dir; // 0 is north, increases in clockwise direction

    renderer(struct chunkmanager *cm, rendertype rt, uint8_t dir) : cm(cm), rt(rt), dir(dir) {}

    virtual coord image_size(scoord origin, coord dim) = 0;
    virtual coord image_size() = 0;
    virtual uint8_t *render(scoord origin, coord dim) = 0;
    virtual uint8_t *render_all() = 0;
};

struct topdownrenderer : renderer {
    topdownrenderer(struct chunkmanager *cm, uint8_t dir) : renderer(cm, RENDER_TOP_DOWN, dir) {
        switch (dir) {
            case DIR_NORTH:
            case DIR_EAST:
            case DIR_SOUTH:
            case DIR_WEST:
                break;
            default:
                dir = DIR_NORTH;
                break;
        }
    }

    virtual coord image_size(scoord origin, coord dim);
    virtual coord image_size();
    virtual uint8_t *render(scoord origin, coord dim);
    virtual uint8_t *render_all();
};

struct renderer *get_renderer(struct chunkmanager *cm, rendertype rt = RENDER_TOP_DOWN, uint8_t dir = 0);

} // end namespace cnbt
