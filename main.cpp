#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <zlib.h>
#include <vector>

#include "tagparser.hpp"

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

    ret = inflateEnd(&stream);
    if (ret != Z_OK) {
        ERR("inflateEnd failed\n");
    }

    return 0;

_decompress_data_cleanup:
    inflateEnd(&stream);
    return 1;
}

void examine_file(FILE *fp) {
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
    if (cnbt::parse_tags(data, len)) {
        ERR("Tag parsing failed\n");
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
