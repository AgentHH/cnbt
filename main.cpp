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
#include "chunk.hpp"
#include "coord.hpp"
#include "render.hpp"

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

        if (argc < 4) {
            delete(t);
            return 0;
        }

        uint8_t *buf;
        cnbt::stream_writer w(&buf);
        t->write(w, 1);

        gzFile fp = gzopen(argv[3], "wb");
        gzwrite(fp, buf, sizeof(uint8_t) * w.written());
        gzclose(fp);

        delete(t);
    } else if (!strcmp(argv[1], "-c")) {
        if (argc < 4) {
            ERR("need x and z\n");
            exit(1);
        }
        int32_t x = strtol(argv[2], NULL, 0);
        int32_t z = strtol(argv[3], NULL, 0);
        uint8_t buf[32];
        cnbt::chunkcoord_to_filename(cnbt::chunkcoord(x, z), buf, 32);
        printf("Filename is \"%s\"\n", buf);
    } else {
        cnbt::level l(argv[1]);
        int ret = l.load();
        if (ret) {
            printf("level.load() failed\n");
            return 1;
        }
        //cnbt::print_tag_tree(l.root);
        l.load_chunk_list();

        cnbt::chunkmanager *cm = &l.manager;
        printf("bounding box is (%d,%d) to (%d,%d)\n", cm->max->x, cm->max->z, cm->min->x, cm->min->z);
        size_t h = 1 + cm->max->x - cm->min->x, w = 1 + cm->max->z - cm->min->z;
        printf("size of box is (%d,%d)\n", w, h);
        printf("explored area is %lu (%2.0f%% of the total area)\n", cm->chunks.size(), 100 * (double)cm->chunks.size() / (double)(w * h));

        uint8_t *image = (uint8_t*)calloc(w * h * 256 * 3, sizeof(uint8_t));
        if (!image) {
            ERR("Failed to allocate image\n");
            exit(1);
        }

        // XXX this should be handled by the chunk manager,
        //     but I really want to see a whole, rendered map
        for (cnbt::chunkmap::iterator i = cm->chunks.begin(); i != cm->chunks.end(); ++i) {
            char *path = argv[1], buf[32];
            //printf("Chunk (%d,%d)\n", (*i).first.x, (*i).first.z);
            int namelen = cnbt::chunkcoord_to_filename((*i).first, (uint8_t*)buf, 32);
            size_t chunkpathlen = strlen(path) + 1 + namelen;
            char chunkpath[chunkpathlen];
            strncpy(chunkpath, path, chunkpathlen);
            strncat(chunkpath, "/", chunkpathlen - strlen(path) - 1);
            strncat(chunkpath, buf, chunkpathlen - strlen(path) - 2);

            cnbt::tag *t = cnbt::eat_nbt_file(chunkpath);
            if (!t) {
                printf("tried to acquire chunk %d, %d\n", (*i).first.x, (*i).first.z);
                ERR("Eating chunk file failed\n");
                exit(1);
            }

            cnbt::chunk c;
            ret = c.init(t);
            if (ret) {
                ERR("Chunk init failed\n");
                exit(1);
            }
            delete(t);

            //printf("%d %d\n", c.x, c.z);

            cnbt::render_top_down(&c, image, (c.x - cm->min->x), (c.z - cm->min->z), w, h);
        }
        FILE *fp = fopen("out/map.pgm", "wb");
        char header[32];
        ret = snprintf(header, 32, "P6 %lu %lu 255\n", w * 16, h * 16);
        fwrite(header, sizeof(char), ret, fp);
        fwrite(image, sizeof(uint8_t), w * h * 256 * 3, fp);
        fclose(fp);
        free(image);
    }

    return 0;
}
