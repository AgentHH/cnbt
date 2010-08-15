#pragma once
// {{{ #includes
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

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
    struct chunkmanager manager;

    level(char *path);
    ~level();

    int load(); // loads level.dat and chunk list
};
// }}}
} // end namespace cnbt
