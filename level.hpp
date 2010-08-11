#pragma once
// {{{ #includes
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <map>

#include "datastream.hpp"
#include "tagparser.hpp"
#include "chunk.hpp"
#include "coord.hpp"
// }}}
namespace cnbt {
// {{{ #defines and typedefs
#define LEVEL_MAIN_FILE "level.dat"
typedef std::map<struct chunkcoord, struct chunkinfo*> chunkmap;
// }}}
// {{{ main level struct
struct level {
    struct tag *root;
    char *path;
    chunkmap chunks;

    level(char *path);
    virtual ~level();

    virtual int load(); // only loads level.dat
    virtual int load_chunk_list();
};
// }}}
} // end namespace cnbt
