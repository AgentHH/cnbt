#include "datastream.hpp"

#define ERR(args...) fprintf(stderr, args)

namespace cnbt {
// {{{ evil hacks to support network floating point values
#define htonf(A...)  ntohf(A)
#define htond(A...)  ntohd(A)
#define htonll(A...) ntohll(A)

float ntohf(float f) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    union {
        float x;
        uint32_t y;
    };

    x = f;
    y = __bswap_32(y);
    return x;
#else
    return f;
#endif
}
double ntohd(double d) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    union {
        double x;
        uint64_t y;
    };

    x = d;
    y = __bswap_64(y);
    return x;
#else
    return d;
#endif
}
uint64_t ntohll(uint64_t i) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return __bswap_64(i);
#else
    return i;
#endif
}
// }}}
// {{{ stream eater to simplify extracting data
stream_eater::stream_eater(uint8_t *buf, size_t len) : buf(buf), len(len) {
    pos = 0;
}

size_t stream_eater::remain() {
    return len - pos;
}
size_t stream_eater::eaten() {
    return pos;
}

uint8_t stream_eater::eat_bool() {
    assert(remain() >= sizeof(uint8_t));
    uint8_t ret = buf[pos] ? 1 : 0;
    pos += sizeof(uint8_t);
    return ret;
}
enum tagtype stream_eater::peek_tag() {
    if (remain() < sizeof(uint8_t))
        return TAG_INVALID;
    enum tagtype ret = (enum tagtype)buf[pos];
    return ret;
}
int stream_eater::eat_tag(enum tagtype t) {
    if (remain() < sizeof(uint8_t))
        return 1;
    enum tagtype ret = (enum tagtype)buf[pos];
    pos += sizeof(uint8_t);
    if (t == TAG_UNKNOWN)
        return ret;
    if (ret != t) {
        return 2;
    }
    return 0;
}
uint8_t stream_eater::eat_byte() {
    assert(remain() >= sizeof(uint8_t));
    uint8_t ret = buf[pos];
    pos += sizeof(uint8_t);
    return ret;
}
uint16_t stream_eater::eat_short() {
    assert(remain() >= sizeof(uint16_t));
    uint16_t ret = ntohs(*(uint16_t*)&buf[pos]);
    pos += sizeof(uint16_t);
    return ret;
}
uint32_t stream_eater::eat_int() {
    assert(remain() >= sizeof(uint32_t));
    uint32_t ret = ntohl(*(uint32_t*)&buf[pos]);
    pos += sizeof(uint32_t);
    return ret;
}
uint64_t stream_eater::eat_long() {
    assert(remain() >= sizeof(uint64_t));
    uint64_t ret = ntohll(*(uint64_t*)&buf[pos]);
    pos += sizeof(uint64_t);
    return ret;
}
float stream_eater::eat_float() {
    assert(remain() >= sizeof(float));
    float ret = ntohf(*(float*)&buf[pos]);
    pos += sizeof(float);
    return ret;
}
double stream_eater::eat_double() {
    assert(remain() >= sizeof(double));
    double ret = ntohd(*(double*)&buf[pos]);
    pos += sizeof(double);
    return ret;
}
uint8_t *stream_eater::eat_byte_array(uint32_t l) {
    if (remain() < l * sizeof(uint8_t))
        return NULL;
    uint8_t *ret = &buf[pos];
    pos += l * sizeof(uint8_t);
    return ret;
}
uint16_t *stream_eater::eat_short_array(uint32_t l) {
    if (remain() < l * sizeof(uint16_t))
        return NULL;
    uint16_t *ret = (uint16_t*)&buf[pos];
    pos += l * sizeof(uint16_t);
    return ret;
}
uint8_t *stream_eater::eat_string() {
    // warning: this function allocates memory
    if (remain() < sizeof(uint16_t))
        return NULL;
    uint16_t s = eat_short();
    if (remain() < s) {
        return NULL;
    }
    uint8_t *ret = (uint8_t*)strndup((const char*)&buf[pos], s);
    pos += s * sizeof(uint8_t);
    return ret;
}
// }}}
// {{{ stream writer to spit out new data
stream_writer::stream_writer(uint8_t **extbuf) : extbuf(extbuf) {
    pos = 0;
    len = STREAM_WRITER_BUFSIZE;
    buf = (uint8_t*)malloc(sizeof(uint8_t) * len);
}
stream_writer::~stream_writer() {
    if (buf)
        free(buf);
}
size_t stream_writer::written() {
    return pos;
}
size_t stream_writer::remain() {
    return len - pos;
}
int stream_writer::reallocate() {
    len += STREAM_WRITER_BUFSIZE;
    buf = (uint8_t*)realloc(buf, len);
    if (!buf) {
        ERR("Unable to reallocate output buffer in stream_writer\n");
        return 1;
    }
    return 0;
}

int stream_writer::write_bool(uint8_t b) {
    while (remain() < sizeof(uint8_t)) {
        if (reallocate() != 0) {
            return 1;
        }
    }
    buf[pos] = b;
    pos += sizeof(uint8_t);

    return 0;
}
int stream_writer::write_tag(enum tagtype t) {
    while (remain() < sizeof(uint8_t)) {
        if (reallocate() != 0) {
            return 1;
        }
    }
    buf[pos] = (uint8_t)t;
    pos += sizeof(uint8_t);

    return 0;
}
int stream_writer::write_byte(uint8_t b) {
    while (remain() < sizeof(uint8_t)) {
        if (reallocate() != 0) {
            return 1;
        }
    }
    buf[pos] = b;
    pos += sizeof(uint8_t);

    return 0;
}
int stream_writer::write_short(uint16_t s) {
    while (remain() < sizeof(uint16_t)) {
        if (reallocate() != 0) {
            return 1;
        }
    }
    *(uint16_t*)&buf[pos] = htons(s);
    pos += sizeof(uint16_t);

    return 0;
}
int stream_writer::write_int(uint32_t i) {
    while (remain() < sizeof(uint32_t)) {
        if (reallocate() != 0) {
            return 1;
        }
    }
    *(uint32_t*)&buf[pos] = htonl(i);
    pos += sizeof(uint32_t);

    return 0;
}
int stream_writer::write_long(uint64_t l) {
    while (remain() < sizeof(uint64_t)) {
        if (reallocate() != 0) {
            return 1;
        }
    }
    *(uint64_t*)&buf[pos] = htonll(l);
    pos += sizeof(uint64_t);

    return 0;
}
int stream_writer::write_float(float f) {
    while (remain() < sizeof(float)) {
        if (reallocate() != 0) {
            return 1;
        }
    }
    *(float*)&buf[pos] = htonf(f);
    pos += sizeof(float);

    return 0;
}
int stream_writer::write_double(double d) {
    while (remain() < sizeof(double)) {
        if (reallocate() != 0) {
            return 1;
        }
    }
    *(double*)&buf[pos] = htond(d);
    pos += sizeof(double);

    return 0;
}
int stream_writer::write_byte_array(uint8_t *d, uint32_t l) {
    while (remain() < l * sizeof(uint8_t)) {
        if (reallocate() != 0) {
            return 1;
        }
    }
    memcpy(&buf[pos], d, l * sizeof(uint8_t));
    pos += l * sizeof(uint8_t);

    return 0;
}
int stream_writer::write_short_array(uint16_t *d, uint32_t l) {
    while (remain() < l * sizeof(uint16_t)) {
        if (reallocate() != 0) {
            return 1;
        }
    }
    memcpy(&buf[pos], d, l * sizeof(uint16_t));
    pos += l * sizeof(uint16_t);

    return 0;
}
int stream_writer::write_string(uint8_t *s) {
    if (s == NULL) {
        s = (uint8_t *)"";
    }
    size_t l = strlen((const char *)s) * sizeof(uint8_t);
    if (l >= (1 << (sizeof(uint16_t) * 8))) {
        return 2;
    }
    while (remain() < sizeof(uint16_t) + l) {
        if (reallocate() != 0) {
            return 1;
        }
    }
    *(uint16_t*)&buf[pos] = htons((uint16_t)l);
    pos += sizeof(uint16_t);
    memcpy(&buf[pos], s, l);
    pos += l;

    return 0;
}
// }}}
// {{{ zlib helper function
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

    ret = inflateEnd(&stream);
    if (ret != Z_OK) {
        ERR("inflateEnd failed\n");
    }

    return 0;

_decompress_data_cleanup:
    inflateEnd(&stream);
    return 1;
}
// }}}
} // end namespace cnbt
