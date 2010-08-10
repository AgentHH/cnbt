#pragma once
// {{{ #includes
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <unordered_set>

#include "datastream.hpp"
#include "tagparser.hpp"
// }}}
namespace cnbt {
// {{{ #defines
#define LEVEL_MAIN_FILE "level.dat"
// }}}
// {{{ various coordinate structs
struct chunkcoord;
struct blockcoord;

struct chunkcoord {
    int32_t x, y;

    chunkcoord(int32_t x, int32_t y);
    chunkcoord(struct blockcoord &b);

    char *x_base36();
    char *y_base36();
};

struct blockcoord {
    int32_t x, z;
    int8_t y;

    blockcoord(int32_t x, int8_t y, int32_t z);
    blockcoord(struct chunkcoord &c);
};
// }}}
// {{{ main level struct
struct level {
    struct tag *root;
    char *path;
    std::unordered_set<struct chunkcoord> chunks;

    level(char *path);
    virtual ~level();

    virtual int load(); // only loads level.dat
    virtual int load_chunk_list();
};
// }}}
} // end namespace cnbt
