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
// }}}
// {{{ main level struct
struct level {
    struct tag *root;
    char *path;
    std::map<struct chunkcoord, struct chunkinfo*> chunks;

    level(char *path);
    virtual ~level();

    virtual int load(); // only loads level.dat
    virtual int load_chunk_list();
};
// }}}
} // end namespace cnbt
