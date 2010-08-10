#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include "datastream.hpp"

namespace cnbt {

struct tag {
    std::vector<struct tag *> children;
    uint8_t *name;

    tag() {
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
    int init(struct stream_eater &s, int named);
};
struct tag_byte : tag {
    int8_t value;
    int init(struct stream_eater &s, int named);
};
struct tag_short : tag {
    int16_t value;
    int init(struct stream_eater &s, int named);
};
struct tag_int : tag {
    int32_t value;
    int init(struct stream_eater &s, int named);
};
struct tag_long : tag {
    int64_t value;
    int init(struct stream_eater &s, int named);
};
struct tag_float : tag {
    float value;
    int init(struct stream_eater &s, int named);
};
struct tag_double : tag {
    double value;
    int init(struct stream_eater &s, int named);
};
struct tag_string : tag {
    uint8_t *value;
    int init(struct stream_eater &s, int named);
    virtual ~tag_string();
};
struct tag_byte_array : tag {
    int8_t *value;
    int32_t num;
    int init(struct stream_eater &s, int named);
    virtual ~tag_byte_array();
};
struct tag_list : tag {
    int init(struct stream_eater &s, int named);
};
struct tag_compound : tag {
    int init(struct stream_eater &s, int named);
};

int parse_tags(uint8_t *data, size_t len);

} // end namespace "cnbt"
