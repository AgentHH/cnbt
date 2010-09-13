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

#include "minecraft.hpp"

namespace cnbt {
namespace game {
namespace colorfile {
enum {
    CHARS_PER_LINE = 256,
};

static const char COLORFILE_NAME[] = "cnbtcolors";
static const char COLORFILE_VERSION[] = "v1";
static const char COLORFILE_COMMENT[] = "#";
static const char COLORFILE_WHITESPACE[] = " \t";
static const char COLORFILE_SEPARATOR[] = "-";
enum options {
    COLORFILE_OPT_NONE       = 0x00000000,
    COLORFILE_OPT_SHADESIDES = 0x00000001,
    COLORFILE_OPT_ALTERNATE  = 0x00000002,
};
struct colorfile_option {
    const char *name;
    options value;
};
static const colorfile_option COLORFILE_OPTIONS[] = {
    {"shadesides", COLORFILE_OPT_SHADESIDES},
    {"alternate",  COLORFILE_OPT_ALTERNATE},
    {NULL,         COLORFILE_OPT_NONE}
};

struct tokendata {
    char *s;
    size_t len;
    size_t pos;
    bool first;

    tokendata(char *s);
    ~tokendata();
    char *next_token();
};

enum {
    COLORFILE_BLOCK_NORMAL = 0,
    COLORFILE_BLOCK_ERROR,
    COLORFILE_BLOCK_BACKGROUND,
    COLORFILE_BLOCK_SIDESHADE,
    COLORFILE_BLOCK_ALTERNATESHADE,
};
enum {
    COLORFILE_C_TOP_BRIGHT    = 0x01,
    COLORFILE_C_TOP_DARK      = 0x02,
    COLORFILE_C_SIDE_BRIGHT   = 0x04,
    COLORFILE_C_SIDE_DARK     = 0x08,

    COLORFILE_C_TOP_BOTH      = 0x03,
    COLORFILE_C_SIDE_BOTH     = 0x0c,
};
struct color {
    uint8_t top_bright[4];
    uint8_t top_dark[4];
    uint8_t side_bright[4];
    uint8_t side_dark[4];

    uint8_t blockid;
    uint8_t blocktype;
};

char *eat_whitespace(char *c);
bool is_comment(char *line);
bool is_end_of_line(char *line);
tokendata *read_line(FILE *fp, size_t *line);
int eat_preamble(FILE *fp, size_t *line);
int32_t eat_options(FILE *fp, size_t *line);
int eat_separator(FILE *fp, size_t *line);
int eat_colorline(FILE *fp, size_t *line, color *cs);

} // end namespace colorfile
struct blockcolors *init_block_colors_from_file(char *name);
} // end namespace game
} // end namespace cnbt
