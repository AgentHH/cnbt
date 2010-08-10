#pragma once
// {{{ #includes
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <errno.h>

#include "nbtconstants.hpp"
#include "datastream.hpp"
// }}}
namespace cnbt {
// {{{ constants for NBT file buffers
// note, this is a hack. XXX
// I should carefully pick the size, and reallocate
// as necessary, but I'm lazy and this silly hack
// works for all of the Minecraft levels I've seen
#define NBT_BUFFER_SIZE  131072
// }}}
// {{{ tag class defreadions
// XXX: writing doesn't check for error conditions, so it'll explode if
// something bad happens to it
struct tag {
    std::vector<struct tag *> children;
    uint8_t *name;
    enum tagtype type;

    tag(enum tagtype type) : type(type) {
        name = NULL;
    }
    virtual ~tag() {
        for (std::vector<struct tag *>:: iterator i = children.begin(); i != children.end(); ++i) {
            delete *i;
        }
        if (name)
            free(name);
    }

    virtual int read(struct stream_eater &s, int named) = 0;
    virtual int write(struct stream_writer &s, int named) = 0;
};
struct tag_end : tag {
    tag_end() : tag(TAG_END) {}
    int read(struct stream_eater &s, int named);
    int write(struct stream_writer &s, int named);
};
struct tag_byte : tag {
    int8_t value;
    tag_byte() : tag(TAG_BYTE) {}
    int read(struct stream_eater &s, int named);
    int write(struct stream_writer &s, int named);
};
struct tag_short : tag {
    int16_t value;
    tag_short() : tag(TAG_SHORT) {}
    int read(struct stream_eater &s, int named);
    int write(struct stream_writer &s, int named);
};
struct tag_int : tag {
    int32_t value;
    tag_int() : tag(TAG_INT) {}
    int read(struct stream_eater &s, int named);
    int write(struct stream_writer &s, int named);
};
struct tag_long : tag {
    int64_t value;
    tag_long() : tag(TAG_LONG) {}
    int read(struct stream_eater &s, int named);
    int write(struct stream_writer &s, int named);
};
struct tag_float : tag {
    float value;
    tag_float() : tag(TAG_FLOAT) {}
    int read(struct stream_eater &s, int named);
    int write(struct stream_writer &s, int named);
};
struct tag_double : tag {
    double value;
    tag_double() : tag(TAG_DOUBLE) {}
    int read(struct stream_eater &s, int named);
    int write(struct stream_writer &s, int named);
};
struct tag_string : tag {
    uint8_t *value;
    tag_string() : tag(TAG_STRING) {}
    int read(struct stream_eater &s, int named);
    virtual ~tag_string();
    int write(struct stream_writer &s, int named);
};
struct tag_byte_array : tag {
    int8_t *value;
    int32_t num;
    tag_byte_array() : tag(TAG_BYTE_ARRAY) {}
    int read(struct stream_eater &s, int named);
    virtual ~tag_byte_array();
    int write(struct stream_writer &s, int named);
};
struct tag_list : tag {
    enum tagtype listtype;
    tag_list() : tag(TAG_LIST) {}
    int read(struct stream_eater &s, int named);
    int write(struct stream_writer &s, int named);
};
struct tag_compound : tag {
    tag_compound() : tag(TAG_COMPOUND) {}
    int read(struct stream_eater &s, int named);
    int write(struct stream_writer &s, int named);
};
// }}}
// {{{ parser functions
struct tag *parse_tags(uint8_t *data, size_t len);
void print_tag_tree(struct cnbt::tag *t);
struct tag *eat_nbt_file(char *filename);
// }}}
} // end namespace "cnbt"
