/*
    Copyright 2010 Hans Nielsen (AgentHH)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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

int oblique_blockcoord_to_image(coord chunk, coord dim, coord imagesize, blockcoord block, uint8_t dir, size_t *offset);
void render_oblique(struct chunk *c, uint8_t *buf, coord chunk, coord dim, coord imagesize, uint8_t dir, game::entitymap &em);

int top_down_blockcoord_to_image(coord chunk, coord dim, coord imagesize, blockcoord block, uint8_t dir, size_t *offset);
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

struct obliquerenderer : renderer {
    obliquerenderer(struct chunkmanager *cm, uint8_t dir);

    virtual coord image_size(scoord origin, coord dim);
    virtual coord image_size();
    virtual uint8_t *render(scoord origin, coord dim);
    virtual uint8_t *render_all();
};

struct topdownrenderer : renderer {
    topdownrenderer(struct chunkmanager *cm, uint8_t dir);

    virtual coord image_size(scoord origin, coord dim);
    virtual coord image_size();
    virtual uint8_t *render(scoord origin, coord dim);
    virtual uint8_t *render_all();
};

struct renderer *get_renderer(struct chunkmanager *cm, rendertype rt = RENDER_TOP_DOWN, uint8_t dir = 0);

} // end namespace cnbt
