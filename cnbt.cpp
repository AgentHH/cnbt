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
#include "platform.hpp"

#include <time.h>

#include "tagparser.hpp"
#include "datastream.hpp"
#include "level.hpp"
#include "chunk.hpp"
#include "coord.hpp"
#include "minecraft.hpp"
#include "render.hpp"

int parse_options(int argc, char **argv, char **world, char **out, cnbt::rendertype *rt, uint8_t *dir, bool *alc) {
    int i;
    if (rt) {
        *rt = cnbt::RENDER_ANGLED;
    }
    if (dir) {
        *dir = cnbt::DIR_NORTHEAST;
    }
    if (world) {
        *world = NULL;
    }
    if (out) {
        *out = NULL;
    }
    if (alc) {
        *alc = false;
    }

    for (i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-d")) {
            i++;
            if (strlen(argv[i]) == 1) {
                switch (argv[i][0]) {
                    case 'n':
                        if (dir) {
                            *dir = cnbt::DIR_NORTH;
                        }
                        continue;
                    case 's':
                        if (dir) {
                            *dir = cnbt::DIR_SOUTH;
                        }
                        continue;
                    case 'e':
                        if (dir) {
                            *dir = cnbt::DIR_EAST;
                        }
                        continue;
                    case 'w':
                        if (dir) {
                            *dir = cnbt::DIR_WEST;
                        }
                        continue;
                    default:
                        break;
                }
            } else {
                if (!strcmp(argv[i], "ne")) {
                    if (dir) {
                        *dir = cnbt::DIR_NORTHEAST;
                    }
                    continue;
                } else if (!strcmp(argv[i], "se")) {
                    if (dir) {
                        *dir = cnbt::DIR_SOUTHEAST;
                    }
                    continue;
                } else if (!strcmp(argv[i], "sw")) {
                    if (dir) {
                        *dir = cnbt::DIR_SOUTHWEST;
                    }
                    continue;
                } else if (!strcmp(argv[i], "nw")) {
                    if (dir) {
                        *dir = cnbt::DIR_NORTHWEST;
                    }
                    continue;
                }
            }
            ERR("Invalid direction \"%s\" specified\n", argv[i]);
            return 1;
        } else if (!strcmp(argv[i], "-r")) {
            i++;
            if (!strcmp(argv[i], "topdown")) {
                if (rt) {
                    *rt = cnbt::RENDER_TOP_DOWN;
                }
            } else if (!strcmp(argv[i], "oblique")) {
                if (rt) {
                    *rt = cnbt::RENDER_OBLIQUE;
                }
            } else if (!strcmp(argv[i], "angled")) {
                if (rt) {
                    *rt = cnbt::RENDER_ANGLED;
                }
            } else {
                ERR("Invalid rendertype \"%s\" specified\n", argv[i]);
                return 1;
            }
        } else if (!strcmp(argv[i], "-a")) {
            if (alc) {
                *alc = true;
            }
        } else if (out && *out) {
            ERR("Too many files specified on command-line\n");
            return 1;
        } else if (world && *world) {
            if (out) {
                *out = argv[i];
            }
            //printf("got output file %s\n", *out);
        } else {
            if (world) {
                *world = argv[i];
            }
            //printf("got world directory %s\n", *world);
        }
    }

    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        ERR("usage: %s [-d dir] [-r rendertype] [-a] path/to/level output.png\n"
"    valid dirs are: n ne e se s sw w nw\n"
"    valid rendertypes are: topdown oblique angled\n"
"    -a: alternate dark and light colors for horizontal planes\n"
"\n"
"    By default, an angled view towards the NE is rendered\n", argv[0]);
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
    } else if (!strcmp(argv[1], "-p")) { // prune a map
        if (argc < 2) {
            ERR("You must supply a map directory\n");
            return 1;
        }

        char *name = argv[2];
        int ret;

        cnbt::level l((const char*)name);
        ret = l.load();
        if (ret) {
            printf("level.load() failed\n");
            return 1;
        }

        cnbt::chunkmanager *cm = &l.manager;
        std::deque<cnbt::chunklist*> *groups = cm->find_chunk_groups();
        if (!groups) {
            printf("No chunks in level\n");
            return 1;
        }

        if (groups->size() > 1) {
            size_t max = 0, sum = 0;
            std::deque<cnbt::chunklist*>::iterator maxci;
            for (std::deque<cnbt::chunklist*>::iterator i = groups->begin(); i != groups->end(); ++i) {
                size_t temp = (*i)->size();
                if (temp > max) {
                    sum += max;
                    max = temp;
                    maxci = i;
                } else {
                    sum += temp;
                }
                cnbt::chunkcoord center = cnbt::find_centroid(*i);
                printf("Region with %lu chunks is centered at (%d,%d)\n", temp, center.x, center.z);
            }

            delete (*maxci);
            groups->erase(maxci);

            printf("\nThe largest contiguous region has %lu out of %lu chunks total (%g%%)\n", max, cm->chunks.size(), 100 * (double)max / (double)cm->chunks.size());

            printf("Are you sure you want to delete %lu chunks?\n", sum);
            printf("Type \"yes\" to prune, or anything else to abort.\n");

            char buf[32];
            if (buf == fgets(buf, 32, stdin) && !strcmp(buf, "yes\n")) {
                printf("Pruning all non-connected chunks...\n");
                for (std::deque<cnbt::chunklist*>::iterator i = groups->begin(); i != groups->end(); ++i) {
                    cm->prune_chunks_on_disk(*i);
                }
                printf("...done\n");
            } else {
                printf("Pruning aborted\n");
            }
        } else {
            printf("Entire map is a contiguous region; no need to prune\n");
        }

        for (std::deque<cnbt::chunklist*>::iterator i = groups->begin(); i != groups->end(); ++i) {
            delete (*i);
        }
        delete groups;
    } else { // render a map
        char *name, *out;
        cnbt::rendertype rt;
        uint8_t dir;
        bool alc;
        int ret;

        ret = parse_options(argc - 1, argv + 1, &name, &out, &rt, &dir, &alc);
        if (ret)
            return ret;
        if (name == NULL || out == NULL) {
            ERR("You must supply both the map directory and the output file\n");
            return 1;
        }

        time_t start = time(NULL);

        cnbt::level l((const char*)name);
        ret = l.load();
        if (ret) {
            printf("level.load() failed\n");
            return 1;
        }

        cnbt::game::blockcolors *bc = cnbt::game::init_block_colors(alc);

        cnbt::renderer *r = cnbt::get_renderer(&l.manager, rt, dir, bc);
        uint8_t *image = r->render_all();
        free(bc);

        if (!image) { // all printing should happen inside render_all
            return 1;
        }
        cnbt::coord size = r->image_size();
        cnbt::write_png_to_file(image, size.first, size.second, out);

        free(image);
        delete r;

        time_t end = time(NULL);

        printf("map render took %lu wallclock seconds\n", end - start);
    }

    return 0;
}
