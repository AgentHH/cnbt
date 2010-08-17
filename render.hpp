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
void render_top_down(struct chunk *c, uint8_t *buf, int32_t x, int32_t z, int32_t w, int32_t h, game::entitymap &em);
} // end namespace cnbt
