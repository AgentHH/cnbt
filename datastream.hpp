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

#include <zlib.h>
#include <algorithm>
#ifdef __linux
#  include <byteswap.h>
#endif
namespace cnbt {
enum {
    STREAM_WRITER_BUFSIZE = 4096,
};

enum tagtype { // this is a slightly bad place, but better than a separate header
    TAG_END = 0,
    TAG_BYTE,
    TAG_SHORT,
    TAG_INT,
    TAG_LONG,
    TAG_FLOAT,
    TAG_DOUBLE,
    TAG_BYTE_ARRAY,
    TAG_STRING,
    TAG_LIST,
    TAG_COMPOUND,

    TAG_INVALID = 100,
    TAG_UNKNOWN,
};
// {{{ stream classes to simplify extracting and writing data
struct stream_eater {
    uint8_t *buf;
    size_t len, pos;

    stream_eater(uint8_t *buf, size_t len);
    size_t remain();
    size_t eaten();

    uint8_t eat_bool();
    enum tagtype peek_tag();
    int eat_tag(enum tagtype t);
    uint8_t eat_byte();
    uint16_t eat_short();
    uint32_t eat_int();
    uint64_t eat_long();
    float eat_float();
    double eat_double();
    uint8_t *eat_byte_array(uint32_t l);
    uint16_t *eat_short_array(uint32_t l);
    uint8_t *eat_string();
    int32_t eat_base36_int();
};
struct stream_writer {
    uint8_t *buf;
    uint8_t **extbuf;
    size_t len, pos;

    stream_writer(uint8_t **buf);
    stream_writer(uint8_t *buf, size_t len);
    ~stream_writer();
    size_t written();
    size_t remain();
    int reallocate();

    int write_bool(uint8_t b);
    int write_tag(enum tagtype t);
    int write_byte(uint8_t b);
    int write_short(uint16_t s);
    int write_int(uint32_t i);
    int write_long(uint64_t l);
    int write_float(float f);
    int write_double(double d);
    int write_byte_array(uint8_t *d, uint32_t l);
    int write_short_array(uint16_t *d, uint32_t l);
    int write_string(uint8_t *s);
    int write_base36_int(int32_t d);
};
// }}}
} // end namespace cnbt
