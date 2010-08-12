#pragma once
// {{{ #includes
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include <map>

#include "datastream.hpp"
#include "tagparser.hpp"
#include "coord.hpp"
// }}}
namespace cnbt {
// {{{ #defines and typedefs
#define CHUNK_DATA_LEN 16384
#define CHUNK_SKYLIGHT_LEN 16384
#define CHUNK_HEIGHTMAP_LEN 256
#define CHUNK_BLOCKLIGHT_LEN 16384
#define CHUNK_BLOCKS_LEN 32768

typedef std::pair<struct chunkcoord, struct chunkinfo*> chunkmaptype;
typedef std::map<struct chunkcoord, struct chunkinfo*> chunkmap;
// }}}
int chunkcoord_to_filename(struct chunkcoord c, uint8_t *name, size_t len);
int find_chunk_files(struct chunkmanager *cm, const char *path);
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

    chunkinfo(struct chunkcoord coord) : coord(coord) {}
};
// }}}
struct chunkmanager {
    chunkmap chunks;
    struct chunkcoord *max, *min;

    chunkmanager();
    int add_new_chunk(struct chunkcoord c);
};
} // end namespace cnbt
