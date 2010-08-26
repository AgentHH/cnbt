#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <vector>

#include "tagparser.hpp"
#include "datastream.hpp"
#include "level.hpp"
#include "chunk.hpp"
#include "coord.hpp"
#include "minecraft.hpp"
#include "render.hpp"
#include "util.hpp"

#define ERR(args...) fprintf(stderr, args)

int main(int argc, char *argv[]) {
    apr_app_initialize(&argc, (const char * const**)&argv, NULL);
    atexit(apr_terminate);

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
        cnbt::util::pool p;
        int32_t x = strtol(argv[2], NULL, 0);
        int32_t z = strtol(argv[3], NULL, 0);
        uint8_t *buf;
        cnbt::chunkcoord_to_filename(cnbt::chunkcoord(x, z), &buf, p);
        printf("Filename is \"%s\"\n", buf);
    } else {
        cnbt::level l(argv[1]);
        int ret = l.load();
        if (ret) {
            printf("level.load() failed\n");
            return 1;
        }

        cnbt::renderer *r = cnbt::get_renderer(&l.manager, cnbt::RENDER_OBLIQUE, DIR_SOUTH);
        uint8_t *image = r->render_all();
        cnbt::coord size = r->image_size();
        cnbt::write_png_to_file(image, size.first, size.second, "out/map.png");

        free(image);
        delete r;

#if 0
        for (cnbt::game::entitymap::iterator i = em.begin(); i != em.end(); ++i) {
            delete((*i).second);
        }
#endif
    }

    return 0;
}
