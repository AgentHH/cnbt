#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include "tagparser.hpp"
#include "datastream.hpp"

#define ERR(args...) fprintf(stderr, args)

namespace cnbt {
// {{{ NBT parser
// {{{ various classes representing each tag, as well as a recursive tag parser
// class definitions are in header
// {{{ end tag
int tag_end::init(struct stream_eater &s, int named) {
    if (s.eat_tag(TAG_END))
        return 1;
    return 0;
}
// }}}
// {{{ byte tag
int tag_byte::init(struct stream_eater &s, int named) {
    if (named) {
        if (s.eat_tag(TAG_BYTE))
            return 1;
        name = s.eat_string();
        if (name == NULL)
            return 1;
    }
    if (s.remain() < sizeof(uint8_t))
        return 1;
    value = (int8_t)s.eat_byte();

    return 0;
}
// }}}
// {{{ short tag
int tag_short::init(struct stream_eater &s, int named) {
    if (named) {
        if (s.eat_tag(TAG_SHORT))
            return 1;
        name = s.eat_string();
        if (name == NULL)
            return 1;
    }
    if (s.remain() < sizeof(uint16_t))
        return 1;
    value = (int16_t)s.eat_short();

    return 0;
}
// }}}
// {{{ int tag
int tag_int::init(struct stream_eater &s, int named) {
    if (named) {
        if (s.eat_tag(TAG_INT))
            return 1;
        name = s.eat_string();
        if (name == NULL)
            return 1;
    }
    if (s.remain() < sizeof(uint32_t))
        return 1;
    value = (int32_t)s.eat_int();

    return 0;
}
// }}}
// {{{ long tag
int tag_long::init(struct stream_eater &s, int named) {
    if (named) {
        if (s.eat_tag(TAG_LONG))
            return 1;
        name = s.eat_string();
        if (name == NULL)
            return 1;
    }
    if (s.remain() < sizeof(uint64_t))
        return 1;
    value = (int64_t)s.eat_long();

    return 0;
}
// }}}
// {{{ float tag
int tag_float::init(struct stream_eater &s, int named) {
    if (named) {
        if (s.eat_tag(TAG_FLOAT))
            return 1;
        name = s.eat_string();
        if (name == NULL)
            return 1;
    }
    if (s.remain() < sizeof(float))
        return 1;
    value = s.eat_float();

    return 0;
}
// }}}
// {{{ double tag
int tag_double::init(struct stream_eater &s, int named) {
    if (named) {
        if (s.eat_tag(TAG_DOUBLE))
            return 1;
        name = s.eat_string();
        if (name == NULL)
            return 1;
    }
    if (s.remain() < sizeof(double))
        return 1;
    value = s.eat_double();

    return 0;
}
// }}}
// {{{ string tag
int tag_string::init(struct stream_eater &s, int named) {
    if (named) {
        if (s.eat_tag(TAG_STRING))
            return 1;
        name = s.eat_string();
        if (name == NULL)
            return 1;
    }
    value = s.eat_string();

    return 0;
}

tag_string::~tag_string() {
    if (value)
        free(value);
}
// }}}
// {{{ byte array tag
int tag_byte_array::init(struct stream_eater &s, int named) {
    if (named) {
        if (s.eat_tag(TAG_BYTE_ARRAY))
            return 1;
        name = s.eat_string();
        if (name == NULL)
            return 1;
    }
    if (s.remain() < sizeof(uint32_t))
        return 1;
    num = (int32_t)s.eat_int();
    int8_t *temp = (int8_t *)s.eat_byte_array(num);
    if (temp == NULL)
        return 1;
    value = (int8_t *)calloc(num, sizeof(int8_t));
    memcpy(value, temp, num * sizeof(int8_t));

    return 0;
}

tag_byte_array::~tag_byte_array() {
    if (value)
        free(value);
}
// }}}
// {{{ list tag
int tag_list::init(struct stream_eater &s, int named) {
        struct tag *t;

        if (named) {
            if (s.eat_tag(TAG_LIST))
                return 1;
            name = s.eat_string();
            if (name == NULL)
                return 1;
        }

        enum tagtype type = (enum tagtype)s.eat_tag(TAG_UNKNOWN);
        if (type == TAG_INVALID || s.remain() < sizeof(int32_t))
            return 1;
        int32_t num = (int32_t)s.eat_int();

        for (ssize_t i = 0; i < num; i++) {
            switch (type) {
                case TAG_END: // I'm not sure this is valid, but leaving it here anyways
                    t = new tag_end();
                    ERR("Unsure about TAG_ENDs in lists\n");
                    break;
                case TAG_BYTE:
                    t = new tag_byte();
                    break;
                case TAG_SHORT:
                    t = new tag_short();
                    break;
                case TAG_INT:
                    t = new tag_int();
                    break;
                case TAG_LONG:
                    t = new tag_long();
                    break;
                case TAG_FLOAT:
                    t = new tag_float();
                    break;
                case TAG_DOUBLE:
                    t = new tag_double();
                    break;
                case TAG_BYTE_ARRAY:
                    t = new tag_byte_array();
                    break;
                case TAG_STRING:
                    t = new tag_string();
                    break;
                case TAG_LIST:
                    t = new tag_list();
                    break;
                case TAG_COMPOUND:
                    t = new tag_compound();
                    break;
                default:
                    ERR("Invalid tag %d encountered in list\n", type);
                    return 1;
            }
            if (!t) {
                ERR("Uh-oh, t was null in list\n");
                return 1;
            }
            int ret = t->init(s, 0); // since we're in a list, it's not named
            if (ret) // something broke
                return 1;
            children.push_back(t);
        }

        return 0;
}
// }}}
// {{{ compound tag
int tag_compound::init(struct stream_eater &s, int named) {
        struct tag *t;

        if (named) {
            if (s.eat_tag(TAG_COMPOUND))
                return 1;
            name = s.eat_string();
            if (name == NULL)
                return 1;
        }

        do {
            switch (s.peek_tag()) {
                case TAG_END:
                    t = new tag_end();
                    t->init(s, 1);
                    delete t;
                    return 0; // TAG_END means we're done here
                case TAG_BYTE:
                    t = new tag_byte();
                    break;
                case TAG_SHORT:
                    t = new tag_short();
                    break;
                case TAG_INT:
                    t = new tag_int();
                    break;
                case TAG_LONG:
                    t = new tag_long();
                    break;
                case TAG_FLOAT:
                    t = new tag_float();
                    break;
                case TAG_DOUBLE:
                    t = new tag_double();
                    break;
                case TAG_BYTE_ARRAY:
                    t = new tag_byte_array();
                    break;
                case TAG_STRING:
                    t = new tag_string();
                    break;
                case TAG_LIST:
                    t = new tag_list();
                    break;
                case TAG_COMPOUND:
                    t = new tag_compound();
                    break;
                default:
                    ERR("Invalid tag %d encountered in compound\n", s.peek_tag());
                    return 1;
            }
            if (!t) {
                ERR("Uh-oh, t was null in compound\n");
                return 1;
            }
            int ret = t->init(s, 1);
            if (ret) // something broke
                return 1;
            children.push_back(t);
        } while (s.remain());

        ERR("s.remain() came up with 0 bytes before end of compound\n");
        return 1;
}
// }}}
// }}}
int parse_tags(uint8_t *data, size_t len) {
    // all NBT files start with a TAG_Compound
    struct stream_eater s(data, len);
    struct tag *top = new tag_compound();
    int ret = top->init(s, 1);
    if (ret) {
        ERR("Unable to successfully parse NBT data\n");
    }
    delete top;
    return ret;
}
// }}}
} // end namespace "cnbt"
