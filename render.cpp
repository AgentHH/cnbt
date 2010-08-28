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
#include "render.hpp"

namespace cnbt {
// {{{ PNG writing functions
void magic_write_to_file(png_structp png_ptr, png_bytep data, png_size_t length) {
    FILE *fp = (FILE*)png_ptr->io_ptr;
    fwrite(data, sizeof(png_byte), length, fp);
}
int write_png_to_file(uint8_t *buf, size_t w, size_t h, const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        ERR("Unable to open file \"%s\" for writing\n", filename);
        return 1;
    }

    png_structp pngp;
    png_infop infop;

    pngp = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!pngp) {
        ERR("png_create_write_struct failed\n");
        goto cleanup_write_png_pcws;
    }
    infop = png_create_info_struct(pngp);
    if (!infop) {
        ERR("png_create_write_struct failed\n");
        goto cleanup_write_png_pcis;
    }
    if (setjmp(png_jmpbuf(pngp))) {
        ERR("error during png_init_io\n");
        goto cleanup_write_png_error;
    }
    //png_init_io(pngp, fp);
    png_set_write_fn(pngp, fp, magic_write_to_file, NULL);
    if (setjmp(png_jmpbuf(pngp))) {
        ERR("error during png_set_IHDR\n");
        goto cleanup_write_png_error;
    }
    png_set_IHDR(pngp, infop, w, h, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(pngp, infop);
    if (setjmp(png_jmpbuf(pngp))) {
        ERR("error during png_write_image\n");
        goto cleanup_write_png_error;
    }
    for (size_t i = 0; i < h; i++) {
        png_write_row(pngp, buf + i*w*3);
    }
    if (setjmp(png_jmpbuf(pngp))) {
        ERR("error during png_write_end\n");
        goto cleanup_write_png_error;
    }
    png_write_end(pngp, NULL);

    printf("write succeeded\n");
    png_destroy_write_struct(&pngp, &infop);
    fclose(fp);

    return 0;

cleanup_write_png_error:
cleanup_write_png_pcis:
    //png_destroy_write_struct(pngp, infop);
cleanup_write_png_pcws:
    fclose(fp);
    return 1;
}
// }}}
struct renderer *get_renderer(struct chunkmanager *cm, rendertype rt, uint8_t dir) {
    switch (rt) {
        case RENDER_TOP_DOWN:
            return new topdownrenderer(cm, dir);
        case RENDER_OBLIQUE:
            return new obliquerenderer(cm, dir);
        default:
            return NULL;
    }
}
// {{{ oblique renderer
int oblique_blockcoord_to_image(coord chunk, coord dim, coord imagesize, blockcoord block, uint8_t dir, size_t *offsetside, size_t *offsettop) {
    // x, z are chunk coordinates; 0,0 is the origin
    size_t x = chunk.first, z = chunk.second;
    // w, h are number of chunks in the x and z directions
    size_t nx = dim.first, nz = dim.second;
    // pw, ph are the pixel widths and heights of the image buffer
    size_t pi = imagesize.first, pj = imagesize.second;
    // bx, by, bz are block x, y, and z
    size_t bx = block.x, by = block.y, bz = block.z;

    size_t i, j, ci, cj;

    switch (dir) {
        case DIR_NORTH:
            ci = nz - z - 1;
            cj = x;
            i = BLOCKS_PER_Z - bz - 1;
            j = bx;
            break;
        case DIR_EAST:
            ci = x;
            cj = z;
            i = bx;
            j = bz;
            break;
        case DIR_SOUTH:
            ci = z;
            cj = nx - x - 1;
            i = bz;
            j = BLOCKS_PER_X - bx - 1;
            break;
        case DIR_WEST:
            ci = nx - x - 1;
            cj = nz - z - 1;
            i = BLOCKS_PER_X - bx - 1;
            j = BLOCKS_PER_Z - bz - 1;
            break;
        default:
            *offsetside = 0;
            *offsettop = 0;
            return 1;
    }
    j += BLOCKS_PER_Y - by - 1;

    ci *= BLOCKS_PER_Z;
    cj *= BLOCKS_PER_X;

    *offsetside = (ci + pi * cj) + (i + pi * (j + 1));
    *offsettop = (ci + pi * cj) + (i + pi * j);

    return 0;
}

void render_oblique(struct chunk *c, uint8_t *buf, coord chunk, coord dim, coord imagesize, uint8_t dir, game::entitymap &em) {
    //printf("looking at chunk %d, %d (%d, %d) %p %p\n", x, z, w, h, c, buf);
    // x and z are chunk coordinates, starting at 0,0 and going to w-1, h-1
    // w and h are the number of chunks along the x- and z-axis
    //size_t x = chunk.first, z = chunk.second;
    //size_t w = dim.first, h = dim.second;
    //size_t pi = imagesize.first, pj = imagesize.second;

    for (int _x = 0; _x < BLOCKS_PER_X; _x++) { // 16 blocks in the x-dir
        for (int _z = 0; _z < BLOCKS_PER_Z; _z++) { // 16 blocks in the z-dir
            for (int _y = 0; _y < 128; _y++) {
                uint8_t id;
                int _a, _b;

                if (dir == DIR_NORTH || dir == DIR_EAST) { // this is a hideous hack
                    _a = _x;
                    _b = _z;
                } else {
                    _a = BLOCKS_PER_X - _x - 1;
                    _b = BLOCKS_PER_Z - _z - 1;
                }
                id = c->blocks[_b * 2048 + _a * 128 + _y];
                if (id == 0)
                    continue;

                size_t offsetside, offsettop;
                oblique_blockcoord_to_image(chunk, dim, imagesize, blockcoord(_b, _y, _a), dir, &offsetside, &offsettop);
                uint8_t *destside = buf + offsetside * 3;
                uint8_t *desttop = buf + offsettop * 3;

                struct game::block *b = static_cast<struct game::block *>(em[id]);
                if (b == NULL) {
                    printf("Block %02x not found in hash\n", id);
                    continue;
                }

                game::color pixel;
                if (b->onecolor)
                    pixel = b->brightcolor;
                else
                    pixel = game::interpolate_color(b->darkcolor, b->brightcolor, _y);

                game::color top(desttop[0], desttop[1], desttop[2]);
                top.add_above(pixel);
                desttop[0] = top.r;
                desttop[1] = top.g;
                desttop[2] = top.b;

                pixel.add_above(game::color(0, 0, 0, 128));

                game::color side(destside[0], destside[1], destside[2]);
                side.add_above(pixel);
                destside[0] = side.r;
                destside[1] = side.g;
                destside[2] = side.b;
            }
        }
    }
}

obliquerenderer::obliquerenderer(struct chunkmanager *cm, uint8_t dir) : renderer(cm, RENDER_OBLIQUE, dir) {
    switch (dir) {
        case DIR_NORTH:
            break;
        case DIR_EAST:
        case DIR_SOUTH:
        case DIR_WEST:
        default:
            dir = DIR_NORTH;
            break;
    }
}

coord obliquerenderer::image_size(scoord origin, coord dim) {
    // returned size is in pixels
    // for overhead, we don't have to think at all
    // just supply the width / height multiplied by 16
    switch (dir) {
        case DIR_NORTH:
        case DIR_SOUTH:
            return coord(dim.second * BLOCKS_PER_Z, dim.first * BLOCKS_PER_X + BLOCKS_PER_Y);
        case DIR_EAST:
        case DIR_WEST:
            return coord(dim.first * BLOCKS_PER_X, dim.second * BLOCKS_PER_Z + BLOCKS_PER_Y);
        default:
            return coord(0, 0);
    }
}

coord obliquerenderer::image_size() {
    // returned size is in pixels
    // for overhead, we don't have to think at all
    // just supply the width / height multiplied by 16
    size_t nx, nz;
    nx = 1 + cm->max->x - cm->min->x;
    nz = 1 + cm->max->z - cm->min->z;
    switch (dir) {
        case DIR_NORTH:
        case DIR_SOUTH:
            return coord(nz * BLOCKS_PER_Z, nx * BLOCKS_PER_X + BLOCKS_PER_Y);
        case DIR_EAST:
        case DIR_WEST:
            return coord(nx * BLOCKS_PER_X, nz * BLOCKS_PER_Z + BLOCKS_PER_Y);
        default:
            return coord(0, 0);
    }
}

uint8_t *obliquerenderer::render(scoord origin, coord dim) {
    // x, z, w, h are all unmodified chunk coordinates
    int32_t x = origin.first, z = origin.second;
    size_t nx = dim.first, nz = dim.second;

    // pi, pj are pixel widths and heights for the image
    coord imagesize = image_size(origin, dim);
    size_t pi = imagesize.first, pj = imagesize.second;

    printf("bounding box is (%d,%d) to (%d, %d)\n", cm->max->x, cm->max->z, cm->min->x, cm->min->z);
    printf("size of box is (%ld,%ld)\n", nx, nz);
    printf("explored area is %lu (%2.0f%% of the total area)\n", cm->chunks.size(), 100 * (double)cm->chunks.size() / (double)(nx * nz));
    printf("image size is (%lu,%lu)\n", pi, pj);

    // XXX FIXME: this stuff is a memory leak; need to abstract / fix it
    game::entitymap em;
    game::init_blocks(em);

    uint8_t *image = (uint8_t*)calloc(pi * pj * 3, sizeof(uint8_t));
    if (!image)
        return NULL;

    // _x, _z are 0-based chunk coordinates
    for (size_t _z = 0; _z < nz; _z++) {
        for (size_t _x = 0; _x < nx; _x++) {
            size_t _a, _b;
            if (dir == DIR_NORTH || dir == DIR_EAST) {
                _a = _x;
                _b = _z;
            } else {
                _a = nx - _x - 1;
                _b = nz - _z - 1;
            }
            struct chunkcoord c(_a + x, _b + z);
            if (cm->chunk_exists(c)) {
                //printf("%lu, %lu -> %d, %d\n", _x, _z, c.x, c.z);
                struct chunkinfo *ci = cm->get_chunk(c);
                if (!ci) {
                    free(image);
                    return NULL;
                }
                render_oblique(ci->c, image, coord(_a, _b), dim, imagesize, dir, em);
            }
        }
    }

    return image;
}
uint8_t *obliquerenderer::render_all() {
    size_t nx, nz;
    nx = 1 + cm->max->x - cm->min->x;
    nz = 1 + cm->max->z - cm->min->z;
    return render(scoord(cm->min->x, cm->min->z), coord(nx, nz));
}
// }}}
// {{{ top-down renderer
int top_down_blockcoord_to_image(coord chunk, coord dim, coord imagesize, blockcoord block, uint8_t dir, size_t *offset) {
    // x, z are chunk coordinates; 0,0 is the origin
    size_t x = chunk.first, z = chunk.second;
    // w, h are number of chunks in the x and z directions
    size_t nx = dim.first, nz = dim.second;
    // pw, ph are the pixel widths and heights of the image buffer
    size_t pi = imagesize.first, pj = imagesize.second;
    // bx, bz are block x and y
    size_t bx = block.x, bz = block.z;

    size_t i, j, ci, cj;

    switch (dir) {
        case DIR_NORTH:
            ci = nz - z - 1;
            cj = x;
            i = BLOCKS_PER_Z - bz - 1;
            j = bx;
            break;
        case DIR_EAST:
            ci = x;
            cj = z;
            i = bx;
            j = bz;
            break;
        case DIR_SOUTH:
            ci = z;
            cj = nx - x - 1;
            i = bz;
            j = BLOCKS_PER_X - bx - 1;
            break;
        case DIR_WEST:
            ci = nx - x - 1;
            cj = nz - z - 1;
            i = BLOCKS_PER_X - bx - 1;
            j = BLOCKS_PER_Z - bz - 1;
            break;
        default:
            *offset = 0;
            return 1;
    }
    ci *= BLOCKS_PER_Z;
    cj *= BLOCKS_PER_X;

    *offset = (ci + pi * cj) + (i + pi * j);

    return 0;
}

void render_top_down(struct chunk *c, uint8_t *buf, coord chunk, coord dim, coord imagesize, uint8_t dir, game::entitymap &em) {
    //printf("looking at chunk %d, %d (%d, %d) %p %p\n", x, z, w, h, c, buf);
    // x and z are chunk coordinates, starting at 0,0 and going to w-1, h-1
    // w and h are the number of chunks along the x- and z-axis
    //size_t x = chunk.first, z = chunk.second;
    //size_t w = dim.first, h = dim.second;
    //size_t pi = imagesize.first, pj = imagesize.second;

    for (int _x = 0; _x < BLOCKS_PER_X; _x++) { // 16 blocks in the x-dir
        for (int _z = 0; _z < BLOCKS_PER_Z; _z++) { // 16 blocks in the z-dir
            game::color pixel(255, 255, 255);
            size_t offset;
            top_down_blockcoord_to_image(chunk, dim, imagesize, blockcoord(_x, 0, _z), dir, &offset);
            uint8_t *dest = buf + offset * 3;
            for (int _y = 0; _y < 128; _y++) {
                uint8_t id = c->blocks[_x * 2048 + _z * 128 + _y];
                if (id == 0)
                    continue;

                struct game::block *b = static_cast<struct game::block *>(em[id]);
                if (b == NULL) {
                    printf("Block %02x not found in hash\n", id);
                    continue;
                }

                if (b->onecolor)
                    pixel.add_above(b->brightcolor);
                else
                    pixel.add_above(game::interpolate_color(b->darkcolor, b->brightcolor, _y));
            }
            dest[0] = pixel.r;
            dest[1] = pixel.g;
            dest[2] = pixel.b;
        }
    }
}

topdownrenderer::topdownrenderer(struct chunkmanager *cm, uint8_t dir) : renderer(cm, RENDER_TOP_DOWN, dir) {
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

coord topdownrenderer::image_size(scoord origin, coord dim) {
    // returned size is in pixels
    // for overhead, we don't have to think at all
    // just supply the width / height multiplied by 16
    switch (dir) {
        case DIR_NORTH:
        case DIR_SOUTH:
            return coord(dim.second * BLOCKS_PER_Z, dim.first * BLOCKS_PER_X);
        case DIR_EAST:
        case DIR_WEST:
            return coord(dim.first * BLOCKS_PER_X, dim.second * BLOCKS_PER_Z);
        default:
            return coord(0, 0);
    }
}

coord topdownrenderer::image_size() {
    // returned size is in pixels
    // for overhead, we don't have to think at all
    // just supply the width / height multiplied by 16
    size_t nx, nz;
    nx = 1 + cm->max->x - cm->min->x;
    nz = 1 + cm->max->z - cm->min->z;
    switch (dir) {
        case DIR_NORTH:
        case DIR_SOUTH:
            return coord(nz * BLOCKS_PER_Z, nx * BLOCKS_PER_X);
        case DIR_EAST:
        case DIR_WEST:
            return coord(nx * BLOCKS_PER_X, nz * BLOCKS_PER_Z);
        default:
            return coord(0, 0);
    }
}

uint8_t *topdownrenderer::render(scoord origin, coord dim) {
    // x, z, w, h are all unmodified chunk coordinates
    int32_t x = origin.first, z = origin.second;
    size_t nx = dim.first, nz = dim.second;

    // pi, pj are pixel widths and heights for the image
    coord imagesize = image_size(origin, dim);
    size_t pi = imagesize.first, pj = imagesize.second;

    printf("bounding box is (%d,%d) to (%d, %d)\n", cm->max->x, cm->max->z, cm->min->x, cm->min->z);
    printf("size of box is (%ld,%ld)\n", nx, nz);
    printf("explored area is %lu (%2.0f%% of the total area)\n", cm->chunks.size(), 100 * (double)cm->chunks.size() / (double)(nx * nz));
    printf("image size is (%lu,%lu)\n", pi, pj);

    // XXX FIXME: this stuff is a memory leak; need to abstract / fix it
    game::entitymap em;
    game::init_blocks(em);

    uint8_t *image = (uint8_t*)calloc(pi * pj * 3, sizeof(uint8_t));
    if (!image)
        return NULL;

    // _x, _z are 0-based chunk coordinates
    for (size_t _z = 0; _z < nz; _z++) {
        for (size_t _x = 0; _x < nx; _x++) {
            struct chunkcoord c(_x + x, _z + z);
            if (cm->chunk_exists(c)) {
                //printf("%lu, %lu -> %d, %d\n", _x, _z, c.x, c.z);
                struct chunkinfo *ci = cm->get_chunk(c);
                if (!ci) {
                    free(image);
                    return NULL;
                }
                render_top_down(ci->c, image, coord(_x, _z), dim, imagesize, dir, em);
            }
        }
    }

    return image;
}
uint8_t *topdownrenderer::render_all() {
    size_t nx, nz;
    nx = 1 + cm->max->x - cm->min->x;
    nz = 1 + cm->max->z - cm->min->z;
    return render(scoord(cm->min->x, cm->min->z), coord(nx, nz));
}
// }}}

} // end namespace cnbt
