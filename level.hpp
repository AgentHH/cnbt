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

#include "datastream.hpp"
#include "tagparser.hpp"
// }}}
namespace cnbt {

#define LEVEL_MAIN_FILE "level.dat"
#define LEVEL_COMPRESSED_BUFFER_SIZE    16384
#define LEVEL_UNCOMPRESSED_BUFFER_SIZE  131072

struct level {
    struct tag *root;
    char *path;

    level(char *path);
    virtual ~level();

    virtual int load(); // only loads level.dat
};
} // end namespace cnbt
