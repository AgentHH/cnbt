#pragma once
// {{{ #includes
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <byteswap.h>
#include <assert.h>
#include <arpa/inet.h>
// }}}
namespace cnbt {
// {{{ stuff to help with NBT
enum tagtype {
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
// }}}
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
const uint8_t *lookup_thing(uint16_t id);
// }}}
} // end namespace cnbt
