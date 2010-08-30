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

#include <map>
#include <deque>

#include "datastream.hpp"
#include "tagparser.hpp"
#include "coord.hpp"
namespace cnbt {
enum {
    CHUNK_DATA_LEN = 16384,
    CHUNK_SKYLIGHT_LEN = 16384,
    CHUNK_HEIGHTMAP_LEN = 256,
    CHUNK_BLOCKLIGHT_LEN = 16384,
    CHUNK_BLOCKS_LEN = 32768,

    CHUNKS_MAX_LOADED = 4096,
};

typedef std::pair<struct chunkcoord, struct chunkinfo*> chunkmaptype;
typedef std::map<struct chunkcoord, struct chunkinfo*> chunkmap;

int chunkcoord_to_filename(struct chunkcoord c, uint8_t *name, int len);
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

    chunkmanager(const char *path);
    ~chunkmanager();
    bool chunk_exists(int32_t x, int32_t z);
    bool chunk_exists(struct chunkcoord c);
    struct chunkinfo *get_chunk(struct chunkcoord c);
    int add_new_chunk(struct chunkcoord c);
    int load_chunk_raw(struct chunkinfo *c);
    int load_chunk(struct chunkinfo *c);
    int set_load_strategy(void (*strat)(chunkcoord selected, std::deque<chunkcoord> &load)); // for oblique etc levels
};
} // end namespace cnbt
