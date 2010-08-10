#include "tagparser.hpp"

#define ERR(args...) fprintf(stderr, args)
#define tag_printf(A...) for(int _i=0;_i<depth;_i++){printf("    ");}printf(A)

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
struct tag *parse_tags(uint8_t *data, size_t len) {
    // all NBT files start with a TAG_Compound
    struct stream_eater s(data, len);
    struct tag *top = new tag_compound();
    int ret = top->init(s, 1);
    if (ret) {
        ERR("Unable to successfully parse NBT data\n");
        delete top;
        return NULL;
    }
    return top;
}
// }}}
// {{{ NBT tree printout
void print_tag_tree(struct cnbt::tag *t, int depth) {
    switch (t->type) {
        case cnbt::TAG_END:
            break;
        case cnbt::TAG_BYTE: {
            struct cnbt::tag_byte *x = dynamic_cast<struct cnbt::tag_byte*>(t);
            if (x->name) {
                tag_printf("byte \"%s\" %d\n", x->name, x->value);
            } else {
                tag_printf("byte %d\n", x->value);
            }
            break;
        }
        case cnbt::TAG_SHORT: {
            struct cnbt::tag_short *x = dynamic_cast<struct cnbt::tag_short*>(t);
            if (x->name) {
                tag_printf("short \"%s\" %d\n", x->name, x->value);
            } else {
                tag_printf("short %d\n", x->value);
            }
            break;
        }
        case cnbt::TAG_INT: {
            struct cnbt::tag_int *x = dynamic_cast<struct cnbt::tag_int*>(t);
            if (x->name) {
                tag_printf("int \"%s\" %d\n", x->name, x->value);
            } else {
                tag_printf("int %d\n", x->value);
            }
            break;
        }
        case cnbt::TAG_LONG: {
            struct cnbt::tag_long *x = dynamic_cast<struct cnbt::tag_long*>(t);
            if (x->name) {
                tag_printf("long \"%s\" %ld\n", x->name, x->value);
            } else {
                tag_printf("long %ld\n", x->value);
            }
            break;
        }
        case cnbt::TAG_FLOAT: {
            struct cnbt::tag_float *x = dynamic_cast<struct cnbt::tag_float*>(t);
            if (x->name) {
                tag_printf("float \"%s\" %f\n", x->name, x->value);
            } else {
                tag_printf("float %f\n", x->value);
            }
            break;
        }
        case cnbt::TAG_DOUBLE: {
            struct cnbt::tag_double *x = dynamic_cast<struct cnbt::tag_double*>(t);
            if (x->name) {
                tag_printf("double \"%s\" %g\n", x->name, x->value);
            } else {
                tag_printf("double %g\n", x->value);
            }
            break;
        }
        case cnbt::TAG_STRING: {
            struct cnbt::tag_string *x = dynamic_cast<struct cnbt::tag_string*>(t);
            if (x->name) {
                tag_printf("string \"%s\" \"%s\"\n", x->name, x->value);
            } else {
                tag_printf("string \"%s\"\n", x->value);
            }
            break;
        }
        case cnbt::TAG_BYTE_ARRAY: {
            struct cnbt::tag_byte_array *x = dynamic_cast<struct cnbt::tag_byte_array*>(t);
            if (x->name) {
                tag_printf("byte array \"%s\" with %d elements\n", x->name, x->num);
            } else {
                tag_printf("byte array with %d elements\n", x->num);
            }
            break;
        }
        case cnbt::TAG_LIST: {
            struct cnbt::tag_list *x = dynamic_cast<struct cnbt::tag_list*>(t);
            if (x->name) {
                tag_printf("list \"%s\"\n", x->name);
            } else {
                tag_printf("list\n");
            }
            for (std::vector<struct cnbt::tag*>::iterator i = x->children.begin(); i != x->children.end(); ++i) {
                print_tag_tree(*i, depth + 1);
            }
            break;
        }
        case cnbt::TAG_COMPOUND: {
            struct cnbt::tag_compound *x = dynamic_cast<struct cnbt::tag_compound*>(t);
            if (x->name) {
                tag_printf("compound \"%s\"\n", x->name);
            } else {
                tag_printf("compound\n");
            }
            for (std::vector<struct cnbt::tag*>::iterator i = x->children.begin(); i != x->children.end(); ++i) {
                print_tag_tree(*i, depth + 1);
            }
            break;
        }
        default:
            tag_printf("unrecognized tag\n");
            break;
    }
}

void print_tag_tree(struct cnbt::tag *t) {
    print_tag_tree(t, 0);
}
// }}}
// {{{ helper function to eat an NBT file and return the tree
struct tag *eat_nbt_file(char *filename) {
    // these are hacks, see tagparser.hpp
    uint8_t raw[NBT_COMPRESSED_BUFFER_SIZE], data[NBT_UNCOMPRESSED_BUFFER_SIZE];
    size_t len;
    int ret;

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        ERR("Unable to open file \"%s\": %s\n", filename, strerror(errno));
        return NULL;
    }
    ret = fread(raw, sizeof(uint8_t), NBT_COMPRESSED_BUFFER_SIZE, fp);
    fclose(fp);
    if (!ret) {
        ERR("No data returned while reading %s\n", filename);
        return NULL;
    } else if (ret == NBT_COMPRESSED_BUFFER_SIZE) {
        ERR("File was longer than %d\n", NBT_COMPRESSED_BUFFER_SIZE);
        return NULL;
    }

    len = NBT_UNCOMPRESSED_BUFFER_SIZE;
    ret = decompress_data(raw, ret, data, &len);
    if (ret) {
        return NULL;
    }

    struct tag *t = parse_tags(data, len);
    if (ret) {
        ERR("Error parsing tags from file\n");
        return NULL;
    }

    return t;
}
// }}}
} // end namespace "cnbt"
