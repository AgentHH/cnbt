#pragma once
// {{{ #includes
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "chunk.hpp"
// }}}
namespace cnbt {
// filename is a temporary hack to see this thing working
void render_top_down(struct chunk *c, uint8_t *buf, int32_t x, int32_t z, int32_t w, int32_t h);
} // end namespace cnbt
