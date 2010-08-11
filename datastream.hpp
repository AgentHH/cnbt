#pragma once
// {{{ #includes
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <byteswap.h>
#include <assert.h>
#include <zlib.h>
#include <arpa/inet.h>

#include "nbtconstants.hpp"
// }}}
namespace cnbt {
#define STREAM_WRITER_BUFSIZE 16384
// {{{ evil hacks to support network floating point values
float ntohf(float f);
double ntohd(double d);
uint64_t ntohll(uint64_t i);
float htonf(float f);
double htond(double d);
uint64_t htonll(uint64_t i);
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
