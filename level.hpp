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
// {{{ #includes
#include "platform.hpp"

//#include <errno.h>
#ifdef _WIN32
# include <windows.h>
#else
# include <dirent.h>
# include <sys/types.h>
#endif

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
