#pragma once
// {{{ #includes
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include "nbtconstants.hpp"
#include "datastream.hpp"
// }}}
namespace cnbt {
// {{{ tag class definitions
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

    virtual int init(struct stream_eater &s, int named) = 0;
};
struct tag_end : tag {
    tag_end() : tag(TAG_END) {}
    int init(struct stream_eater &s, int named);
};
struct tag_byte : tag {
    int8_t value;
    tag_byte() : tag(TAG_BYTE) {}
    int init(struct stream_eater &s, int named);
};
struct tag_short : tag {
    int16_t value;
    tag_short() : tag(TAG_SHORT) {}
    int init(struct stream_eater &s, int named);
};
struct tag_int : tag {
    int32_t value;
    tag_int() : tag(TAG_INT) {}
    int init(struct stream_eater &s, int named);
};
struct tag_long : tag {
    int64_t value;
    tag_long() : tag(TAG_LONG) {}
    int init(struct stream_eater &s, int named);
};
struct tag_float : tag {
    float value;
    tag_float() : tag(TAG_FLOAT) {}
    int init(struct stream_eater &s, int named);
};
struct tag_double : tag {
    double value;
    tag_double() : tag(TAG_DOUBLE) {}
    int init(struct stream_eater &s, int named);
};
struct tag_string : tag {
    uint8_t *value;
    tag_string() : tag(TAG_STRING) {}
    int init(struct stream_eater &s, int named);
    virtual ~tag_string();
};
struct tag_byte_array : tag {
    int8_t *value;
    int32_t num;
    tag_byte_array() : tag(TAG_BYTE_ARRAY) {}
    int init(struct stream_eater &s, int named);
    virtual ~tag_byte_array();
};
struct tag_list : tag {
    tag_list() : tag(TAG_LIST) {}
    int init(struct stream_eater &s, int named);
};
struct tag_compound : tag {
    tag_compound() : tag(TAG_COMPOUND) {}
    int init(struct stream_eater &s, int named);
};
// }}}
// {{{ parser functions
struct tag *parse_tags(uint8_t *data, size_t len);
void print_tag_tree(struct cnbt::tag *t);
// }}}
} // end namespace "cnbt"
