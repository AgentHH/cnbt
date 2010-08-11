#pragma once
// {{{ #includes
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
// }}}
namespace cnbt {
struct chunkcoord {
    int32_t x, y;

    chunkcoord(int32_t x, int32_t y) : x(x), y(y) {}
    bool operator<(const chunkcoord &other) const {
        if (x < other.x || y < other.y)
            return true;
        return false;
    }
};

struct blockcoord {
    int32_t x;
    int8_t y;
    int32_t z;

    blockcoord(int32_t x, int8_t y, int32_t z) : x(x), y(y), z(z) {}
};

struct blockcoord chunkcoord_to_blockcoord(struct chunkcoord c);
struct chunkcoord blockcoord_to_chunkcoord(struct blockcoord b);

} // end namespace cnbt
