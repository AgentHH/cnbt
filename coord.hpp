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
    int32_t x, z;

    chunkcoord() : x(0), z(0) {}
    chunkcoord(int32_t x, int32_t z) : x(x), z(z) {}
    bool operator<(const chunkcoord &other) const {
        if (x < other.x)
            return true;
        if (x == other.x && z < other.z)
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
