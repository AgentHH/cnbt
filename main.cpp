#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <vector>

#include "tagparser.hpp"
#include "datastream.hpp"

#define ERR(args...) fprintf(stderr, args)

#define BUFSIZE 131072

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

    ret = cnbt::decompress_data(raw, ret, data, &len);
    if (ret) {
        return;
    }
    //printf("Ate a %u byte file\n", (uint32_t)len);
    struct cnbt::tag *t = cnbt::parse_tags(data, len);
    if (t == NULL) {
        ERR("Tag parsing failed\n");
    }

    cnbt::print_tag_tree(t);
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
