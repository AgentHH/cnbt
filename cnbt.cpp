#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <zlib.h>
#include <vector>
#include "datastream.hpp"

#define ERR(args...) fprintf(stderr, args)

#define BUFSIZE 131072

int decompress_data(uint8_t *in, size_t inlen, uint8_t *out, size_t *outlen) {
    int ret;
    z_stream stream;

    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = inlen;
    stream.next_in = in;
    ret = inflateInit2(&stream, 16 + MAX_WBITS);
    if (ret != Z_OK) {
        switch (ret) {
            case Z_VERSION_ERROR:
                ERR("Z_VERSION_ERROR in inflateInit\n");
                goto _decompress_data_cleanup;
            case Z_STREAM_ERROR:
                ERR("Z_STREAM_ERROR in inflateInit\n");
                goto _decompress_data_cleanup;
        }
        ERR("Error in inflateInit (%d)\n", ret);
        goto _decompress_data_cleanup;
    }

    stream.avail_out = *outlen;
    stream.next_out = out;

    ret = inflate(&stream, Z_FINISH);
    switch (ret) {
        case Z_STREAM_ERROR:
            ERR("Z_STREAM_ERROR in inflate\n");
            goto _decompress_data_cleanup;
        case Z_NEED_DICT:
            ERR("Z_NEED_DICT in inflate\n");
            goto _decompress_data_cleanup;
        case Z_DATA_ERROR:
            ERR("Z_DATA_ERROR in inflate\n");
            goto _decompress_data_cleanup;
        case Z_MEM_ERROR:
            ERR("Z_MEM_ERROR in inflate\n");
            goto _decompress_data_cleanup;
    }

    if (ret != Z_STREAM_END) {
        ERR("Z_STREAM_END not encountered in inflate (%d)\n", ret);
        goto _decompress_data_cleanup;
    }

    *outlen = *outlen - stream.avail_out;

    return 0;

_decompress_data_cleanup:
    inflateEnd(&stream);
    return 1;
}

struct tag {
    std::vector<struct tag *> children;
    uint8_t *name;

    tag() {
        name = NULL;
    }

    ~tag() {
        if (name) {
            free(name);
        }
    }

    virtual int init(struct stream_eater &s, int named) = 0;
};

struct tag_end : tag {
    int init(struct stream_eater &s, int named) {
        if (s.eat_tag(TAG_END))
            return 1;
        return 0;
    }
};

struct tag_byte : tag {
    int8_t value;

    int init(struct stream_eater &s, int named) {
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
};

struct tag_short : tag {
    int16_t value;

    int init(struct stream_eater &s, int named) {
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
};

struct tag_int : tag {
    int32_t value;

    int init(struct stream_eater &s, int named) {
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
};

struct tag_long : tag {
    int64_t value;

    int init(struct stream_eater &s, int named) {
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
};

struct tag_float : tag {
    float value;

    int init(struct stream_eater &s, int named) {
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
};

struct tag_double : tag {
    double value;

    int init(struct stream_eater &s, int named) {
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
};

struct tag_string : tag {
    uint8_t *value;

    int init(struct stream_eater &s, int named) {
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
};

struct tag_byte_array : tag {
    int8_t *value;
    int32_t num;

    int init(struct stream_eater &s, int named) {
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
};

struct tag_list : tag {
    int init(struct stream_eater &s, int named);
};

struct tag_compound : tag {
    int init(struct stream_eater &s, int named);
};

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
                    printf("Unsure about TAG_ENDs in lists\n");
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
                    printf("Invalid tag %d encountered in list\n", type);
                    return 1;
            }
            if (!t) {
                printf("Uh-oh, t was null.\n");
                return 1;
            }
            int ret = t->init(s, 0); // since we're in a list, it's not named
            if (ret) // something broke
                return 1;
            children.push_back(t);
        }

        return 0;
}

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
                    printf("Invalid tag %d encountered in compound\n", s.peek_tag());
                    return 1;
            }
            if (!t) {
                printf("Uh-oh, t was null.\n");
                return 1;
            }
            int ret = t->init(s, 1);
            if (ret) // something broke
                return 1;
            children.push_back(t);
        } while (s.remain());

        printf("s.remain() came up with 0 bytes before end of compound\n");
        return 1;
}

int parse_tags(uint8_t *data, size_t len) {
    struct stream_eater s(data, len);
    struct tag *top = new tag_compound();
    int ret = top->init(s, 1);
    return ret;
}

void examine_file(FILE *fp) {
    {
        uint8_t raw[BUFSIZE], data[BUFSIZE];
        size_t len = BUFSIZE;
        int ret;
        ret = fread(raw, sizeof(uint8_t), BUFSIZE, fp);
        if (!ret) {
            ERR("No data returned when reading\n");
            return;
        } else if (ret == BUFSIZE) {
            ERR("File was longer than BUFSIZE (%u)\n", BUFSIZE);
            return;
        }

        ret = decompress_data(raw, ret, data, &len);
        if (ret) {
            return;
        }
        //printf("Ate a %u byte file\n", (uint32_t)len);
        if (parse_tags(data, len)) {
            ERR("Tag parsing failed\n");
        }
    }
}

int main(int argc, char *argv[]) {
    FILE *fp;

    if (argc < 2) {
        ERR("usage: %s filename\n", argv[0]);
        exit(1);
    }

    if (!(fp = fopen(argv[1], "r"))) {
        ERR("Unable to open file %s\n", argv[1]);
        exit(1);
    }

    examine_file(fp);

    fclose(fp);

    return 0;
}
