#pragma once
// {{{ #includes
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <sys/types.h>
//#include <dirent.h>
#include <assert.h>
#include <errno.h>
#include <apr-1.0/apr_file_io.h>
#include <apr-1.0/apr_strings.h>

#include "datastream.hpp"
#include "tagparser.hpp"
#include "chunk.hpp"
#include "coord.hpp"
// }}}
namespace cnbt {
// {{{ #defines and typedefs
#define LEVEL_MAIN_FILE "level.dat"
// }}}
int find_chunk_files(struct chunkmanager *cm, const char *path);
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

