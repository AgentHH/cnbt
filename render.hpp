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
#include "platform.hpp"

#include <png.h>

#include "chunk.hpp"
#include "minecraft.hpp"

namespace cnbt {
enum {
    BLOCKS_PER_X = 16,
    BLOCKS_PER_Y = 128,
    BLOCKS_PER_Z = 16,
};

enum { // directions are an unsigned 8-bit quantity
    DIR_NORTH     = 0,
    DIR_NORTHEAST = 32,
    DIR_EAST      = 64,
    DIR_SOUTHEAST = 96,
    DIR_SOUTH     = 128,
    DIR_SOUTHWEST = 160,
    DIR_WEST      = 192,
    DIR_NORTHWEST = 224,
};

enum rendertype {
    RENDER_TOP_DOWN,
    RENDER_ANGLED,
    RENDER_OBLIQUE,
};

int write_png_to_file(uint8_t *buf, size_t w, size_t h, const char *filename);

int oblique_blockcoord_to_image(coord chunk, coord dim, coord imagesize, blockcoord block, uint8_t dir, size_t *offset);
void render_oblique(struct chunk *c, uint8_t *buf, coord chunk, coord dim, coord imagesize, uint8_t dir, game::blockcolors *bc);

int top_down_blockcoord_to_image(coord chunk, coord dim, coord imagesize, blockcoord block, uint8_t dir, size_t *offset);
void render_top_down(struct chunk *c, uint8_t *buf, coord chunk, coord dim, coord imagesize, uint8_t dir, game::blockcolors *bc);

int angled_blockcoord_to_image(coord chunk, coord dim, coord imagesize, blockcoord block, uint8_t dir, size_t *offset);
void render_angled(struct chunk *c, uint8_t *buf, coord chunk, coord dim, coord imagesize, uint8_t dir, game::blockcolors *bc);

struct renderer {
    struct chunkmanager *cm;
    rendertype rt;
    uint8_t dir; // 0 is north, increases in clockwise direction
    bool alternate_level_colors;

    renderer(struct chunkmanager *cm, rendertype rt, uint8_t dir, bool alternate_level_colors) : cm(cm), rt(rt), dir(dir), alternate_level_colors(alternate_level_colors) {}

    virtual coord image_size(scoord origin, coord dim) = 0;
    virtual coord image_size() = 0;
    virtual uint8_t *render(scoord origin, coord dim) = 0;
    virtual uint8_t *render_all() = 0;
};

struct obliquerenderer : renderer {
    obliquerenderer(struct chunkmanager *cm, uint8_t dir, bool alc);

    virtual coord image_size(scoord origin, coord dim);
    virtual coord image_size();
    virtual uint8_t *render(scoord origin, coord dim);
    virtual uint8_t *render_all();
};

struct topdownrenderer : renderer {
    topdownrenderer(struct chunkmanager *cm, uint8_t dir, bool alc);

    virtual coord image_size(scoord origin, coord dim);
    virtual coord image_size();
    virtual uint8_t *render(scoord origin, coord dim);
    virtual uint8_t *render_all();
};

struct angledrenderer : renderer {
    angledrenderer(struct chunkmanager *cm, uint8_t dir, bool alc);

    virtual coord image_size(scoord origin, coord dim);
    virtual coord image_size();
    virtual uint8_t *render(scoord origin, coord dim);
    virtual uint8_t *render_all();
};

struct renderer *get_renderer(struct chunkmanager *cm, rendertype rt, uint8_t dir, bool alc);

} // end namespace cnbt
