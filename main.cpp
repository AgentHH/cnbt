#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <vector>

#include "tagparser.hpp"
#include "datastream.hpp"
#include "level.hpp"

#define ERR(args...) fprintf(stderr, args)

int main(int argc, char *argv[]) {
    if (argc < 2) {
        ERR("usage: %s path/to/level\n", argv[0]);
        exit(1);
    }
    if (!strcmp(argv[1], "-f")) {
        if (argc < 3) {
            ERR("filename must be provided with -f\n");
            exit(1);
        }
        cnbt::tag *t = cnbt::eat_nbt_file(argv[2]);
        if (!t) {
            ERR("Acquiring file failed\n");
            exit(1);
        }
        cnbt::print_tag_tree(t);
        delete(t);
    } else {
        cnbt::level l(argv[1]);
        int ret = l.load();
        if (ret) {
            printf("level.load() failed\n");
            return 1;
        }
        cnbt::print_tag_tree(l.root);
    }

    return 0;
}
