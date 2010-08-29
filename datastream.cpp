/*
    Copyright 2010 Hans Nielsen (AgentHH)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "datastream.hpp"

#define ERR(args...) fprintf(stderr, args)

namespace cnbt {
// {{{ evil hacks to support network floating point values
float ntohf(float f) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    union {
        float x;
        uint32_t y;
    };

    x = f;
# ifdef _MSC_VER
    y = __byteswap_uint32(y);
# elif defined(__linux)
    y = __bswap_32(y);
# endif
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
# ifdef _MSC_VER
    y = __byteswap_uint64(y);
# elif defined(__linux)
    y = __bswap_64(y);
# endif
    return x;
#else
    return d;
#endif
}
uint16_t ntohs(uint16_t i) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
# ifdef _MSC_VER
    return __byteswap_uint16(i);
# elif defined(__linux)
    return __bswap_16(i);
# endif
#else
    return i;
#endif
}
uint32_t ntohl(uint32_t i) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
# ifdef _MSC_VER
    return __byteswap_uint32(i);
# elif defined(__linux)
    return __bswap_32(i);
# endif
#else
    return i;
#endif
}
uint64_t ntohll(uint64_t i) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
# ifdef _MSC_VER
    return __byteswap_uint64(i);
# elif defined(__linux)
    return __bswap_64(i);
# endif
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
    uint8_t *ret = (uint8_t*)malloc(sizeof(uint8_t) * (s + 1));
    memcpy(ret, &buf[pos], s);
    ret[s] = '\0';
    pos += s * sizeof(uint8_t);
    return ret;
}
int32_t stream_eater::eat_base36_int() {
    // this function is not space-friendly, but that's ok
    bool negative = false;
    uint8_t *s = &buf[pos];

    if (remain() < 1)
        return 0;

    if (s[0] == '-') {
        negative = true;
        pos++;
        s++;
    }

    size_t i, len = remain(), ret = 0;
    for (i = 0; i < len; i++) {
        int temp = 0;
        if (s[i] >= '0' && s[i] <= '9') {
            temp = s[i] - '0';
        } else if (s[i] >= 'a' && s[i] <= 'z') {
            temp = 10 + s[i] - 'a';
        } else if (s[i] >= 'A' && s[i] <= 'Z') {
            temp = 10 + s[i] - 'A';
        } else {// invalid character
            break;
        }
        ret *= 36;
        ret += temp;

        if (i == 20) // sanity check
            printf("Should i be larger than 20 in eat_base36_int()?\n");
    }

    pos += i;
    return (int32_t)(ret * (negative ? -1 : 1));
}
// }}}
// {{{ stream writer to spit out new data
// note: the buffer allocated by the stream writer will be
// automatically freed upon deletion
stream_writer::stream_writer(uint8_t **extbuf) : extbuf(extbuf) {
    pos = 0;
    len = STREAM_WRITER_BUFSIZE;
    buf = (uint8_t*)malloc(sizeof(uint8_t) * len);
    *extbuf = buf;
}
stream_writer::stream_writer(uint8_t *buf, size_t len) : buf(buf), len(len) {
    pos = 0;
    extbuf = NULL;
}
stream_writer::~stream_writer() {
    if (extbuf)
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
    if (!extbuf)
        return 1;
    len += STREAM_WRITER_BUFSIZE;
    buf = (uint8_t*)realloc(buf, len);
    if (!buf) {
        ERR("Unable to reallocate output buffer in stream_writer\n");
        return 1;
    }
    *extbuf = buf;
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
int stream_writer::write_base36_int(int32_t n) {
    // this function is not space-friendly, but that's ok
    if (n < 0) {
        n *= -1;
        while (remain() < 1) {
            if (reallocate() != 0) {
                return 1;
            }
        }
        buf[pos] = '-';
        pos++;
    } else if (n == 0) {
        while (remain() < 1) {
            if (reallocate() != 0) {
                return 1;
            }
        }
        buf[pos] = '0';
        pos++;
        return 0;
    }

    uint8_t tempbuf[11]; // 32-bit numbers are at most 10 digits, and base-36 will be much smaller

    size_t i = 0;
    while (n) {
        int temp = n % 36;
        n /= 36;
        if (temp < 10) {
            tempbuf[i] = '0' + temp;
        } else {
            tempbuf[i] = 'a' + (temp - 10);
        }
        i++;
    }
    while (remain() < i) {
        if (reallocate() != 0) {
            return 1;
        }
    }
    for (size_t j = 0; j < i; j++) {
        buf[pos++] = tempbuf[(i - 1) - j];
    }

    return 0;
}
// }}}
} // end namespace cnbt
