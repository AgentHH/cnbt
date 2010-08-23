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
#include "minecraft.hpp"
#include "render.hpp"

#define ERR(args...) fprintf(stderr, args)

int main(int argc, char *argv[]) {
    if (argc < 2) {
        ERR("usage: %s path/to/level\n", argv[0]);
        exit(1);
    }
    if (!strcmp(argv[1], "-f")) { // for testing NBT read / write
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
    } else if (!strcmp(argv[1], "-c")) { // for testing base36 string generation
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

        //cnbt::renderer r(&l.manager);
        //uint8_t *image = r.render_all();

        cnbt::chunkmanager *cm = &l.manager;
        printf("bounding box is (%d,%d) to (%d,%d)\n", cm->max->x, cm->max->z, cm->min->x, cm->min->z);
        size_t h = 1 + cm->max->x - cm->min->x; // x is north-south
        size_t w = 1 + cm->max->z - cm->min->z; // z is east-west
        printf("size of box is (%ld,%ld)\n", w, h);
        printf("explored area is %lu (%2.0f%% of the total area)\n", cm->chunks.size(), 100 * (double)cm->chunks.size() / (double)(w * h));

        uint8_t *image = (uint8_t*)calloc(w * h * 256 * 3, sizeof(uint8_t));
        if (!image) {
            ERR("Failed to allocate image\n");
            exit(1);
        }

        cnbt::game::entitymap em;
        cnbt::game::init_blocks(em);

        cnbt::chunkinfo *ci = cm->start();
        do {
            cnbt::render_top_down(ci->c, image, (ci->coord.x - cm->min->x), (ci->coord.z - cm->min->z), w, h, em);
        } while ((ci = cm->next()));

        ret = cnbt::write_png_to_file(image, w * 16, h * 16, "out/map.png");

        free(image);
        for (cnbt::game::entitymap::iterator i = em.begin(); i != em.end(); ++i) {
            delete((*i).second);
        }
    }

    return 0;
}
