#pragma once
// {{{ #includes
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <map>
#include <deque>

#include "datastream.hpp"
#include "tagparser.hpp"
//#include "level.hpp"
#include "coord.hpp"
// }}}
namespace cnbt {
// {{{ #defines and typedefs
#define CHUNK_DATA_LEN 16384
#define CHUNK_SKYLIGHT_LEN 16384
#define CHUNK_HEIGHTMAP_LEN 256
#define CHUNK_BLOCKLIGHT_LEN 16384
#define CHUNK_BLOCKS_LEN 32768

#define CHUNKS_MAX_LOADED 4096

typedef std::pair<struct chunkcoord, struct chunkinfo*> chunkmaptype;
typedef std::map<struct chunkcoord, struct chunkinfo*> chunkmap;
// }}}
int chunkcoord_to_filename(struct chunkcoord c, uint8_t *name, size_t len);
// {{{ chunk struct
struct chunk {
    uint8_t data[CHUNK_DATA_LEN];
    uint8_t skylight[CHUNK_SKYLIGHT_LEN];
    uint8_t heightmap[CHUNK_HEIGHTMAP_LEN];
    uint8_t blocklight[CHUNK_BLOCKLIGHT_LEN];
    uint8_t blocks[CHUNK_BLOCKS_LEN];

    int32_t x, z;
    int64_t timestamp;

    bool terrainpopulated;

    std::vector<struct tag *> tags;

    int init(struct tag *t);
};
// }}}
// {{{ chunkinfo struct
struct chunkinfo {
    struct chunk *c;
    struct chunkcoord coord;

    chunkinfo(struct chunkcoord coord) : c(NULL), coord(coord) {}
    ~chunkinfo();
};
// }}}
struct chunkmanager {
    chunkmap chunks;
    struct chunkcoord *max, *min;
    chunkmap::iterator mapiterator;
    std::deque<struct chunkinfo*> loadedchunks;
    const char *path;

    chunkmanager();
    ~chunkmanager();
    int add_new_chunk(struct chunkcoord c);
    int load_chunk_raw(struct chunkinfo *c);
    int load_chunk(struct chunkinfo *c);
    //int set_load_strategy(); // for oblique etc levels
    struct chunkinfo *start();
    struct chunkinfo *next();
};
} // end namespace cnbt
