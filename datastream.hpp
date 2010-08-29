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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <zlib.h>
#include <algorithm>
#ifdef __linux
#  include <byteswap.h>
#endif

#include "nbtconstants.hpp"
// }}}
namespace cnbt {
#define STREAM_WRITER_BUFSIZE 4096
// {{{ evil hacks to support network floating point values
float ntohf(float f);
double ntohd(double d);
uint16_t ntohs(uint16_t i);
uint32_t ntohl(uint32_t i);
uint64_t ntohll(uint64_t i);
#define htonf(expr) ntohf(expr)
#define htond(expr) ntohd(expr)
#define htons(expr) ntohs(expr)
#define htonl(expr) ntohl(expr)
#define htonll(expr) ntohll(expr)
//float htonf(float f);
//double htond(double d);
//uint64_t htonll(uint64_t i);
// }}}
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
