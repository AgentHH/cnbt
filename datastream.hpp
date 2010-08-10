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
// {{{ evil hacks to support network floating point values
float ntohf(float f);
double ntohd(double d);
uint64_t ntohll(uint64_t i);
// }}}
// {{{ stream eater to simplify extracting data
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
};
// }}}
// {{{ zlib helper function
int decompress_data(uint8_t *in, size_t inlen, uint8_t *out, size_t *outlen);
// }}}
} // end namespace cnbt
