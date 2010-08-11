#pragma once
// {{{ #includes
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ftw.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <unordered_set>

#include "datastream.hpp"
#include "tagparser.hpp"
// }}}
namespace cnbt {
// {{{ #defines
#define LEVEL_MAIN_FILE "level.dat"

#define CHUNK_DATA_LEN 16384
#define CHUNK_SKYLIGHT_LEN 16384
#define CHUNK_HEIGHTMAP_LEN 256
#define CHUNK_BLOCKLIGHT_LEN 16384
#define CHUNK_BLOCKS_LEN 32768
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
struct chunk {
    uint8_t data[CHUNK_DATA_LEN];
    uint8_t skylight[CHUNK_SKYLIGHT_LEN];
    uint8_t heightmap[CHUNK_HEIGHTMAP_LEN];
    uint8_t blocklight[CHUNK_BLOCKLIGHT_LEN];
    uint8_t blocks[CHUNK_BLOCKS_LEN];

    int32_t x, y;
    int64_t timestamp;

    bool terrainpopulated;

    std::vector<struct tag *> tags;

    int init(struct tag *t);
};
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
