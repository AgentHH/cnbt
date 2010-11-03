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
#include "colorfile.hpp"
#include "render.hpp"

int parse_options(int argc, char **argv, char **world, char **out, char **colorfile, cnbt::rendertype *rt, uint8_t *dir, bool *alc, cnbt::scoord *origin, cnbt::coord *imagedim, int32_t *dim) {
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
    if (colorfile) {
        *colorfile = NULL;
    }
    if (alc) {
        *alc = false;
    }
    if (dim) {
        *dim = 0;
    }

    for (i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-d")) {
            i++;
            if (i >= argc) {
                ERR("No direction specified\n");
                return 1;
            }
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
            if (i >= argc) {
                ERR("No rendertype specified\n");
                return 1;
            }
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
        } else if (!strcmp(argv[i], "-c")) {
            i++;
            if (i >= argc) {
                ERR("No colorfile specified\n");
                return 1;
            }
            if (colorfile) {
                *colorfile = argv[i];
                printf("got color file %s\n", *colorfile);
            }
        } else if (!strcmp(argv[i], "-m")) {
            i++;
            if (dim) {
                char *end;
                int32_t value = strtol(argv[i], &end, 0);
                if (end == argv[i]) {
                    printf("Invalid dimension\n");
                    return 1;
                }
                *dim = value;
            }
        } else if (!strcmp(argv[i], "-b")) {
            if (i + 4 >= argc) {
                ERR("Not enough coordinates for bounding box specified\n");
                return 1;
            }
            i++;
            if (origin) {
                char *end;
                long value = strtol(argv[i], &end, 0);
                if (end == argv[i]) {
                    printf("Invalid origin x coordinate\n");
                    return 1;
                }
                origin->first = value;
            }
            i++;
            if (origin) {
                char *end;
                long value = strtol(argv[i], &end, 0);
                if (end == argv[i]) {
                    printf("Invalid origin z coordinate\n");
                    return 1;
                }
                origin->second = value;
            }
            i++;
            if (imagedim) {
                char *end;
                unsigned long value = strtoul(argv[i], &end, 0);
                if (end == argv[i]) {
                    printf("Invalid image dimension x coordinate\n");
                    return 1;
                }
                imagedim->first = value;
            }
            i++;
            if (imagedim) {
                char *end;
                unsigned long value = strtoul(argv[i], &end, 0);
                if (end == argv[i]) {
                    printf("Invalid image dimension z coordinate\n");
                    return 1;
                }
                imagedim->second = value;
            }
        } else if (out && *out) {
            ERR("Too many files specified on command-line\n");
            return 1;
        } else if (world && *world) {
            if (out) {
                *out = argv[i];
                printf("got output file %s\n", *out);
            }
        } else {
            if (world) {
                *world = argv[i];
                printf("got world directory %s\n", *world);
            }
        }
    }

    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        ERR(
"usage: %s [-d dir] [-r rendertype] [-a] [-c colorfile] [-b originx originz areax areaz]\n"
"              [-m dimension] path/to/level output.png\n"
"By default, an angled view towards the NE is rendered\n"
"\n"
"-d: Valid dirs are: n ne e se s sw w nw\n"
"-r: Valid rendertypes are: topdown oblique angled\n"
"-a: Alternate dark and light colors for horizontal planes\n"
"        Has no effect when a color file is specified\n"
"-c: Render the map with a custom color file\n"
"-m: Render a specific dimension (default is 0, hell is 1)\n"
"-b: Renders only the bounding box specified. Origin is the corner\n"
"        to render from, area is the number of chunks in the x and z\n"
"        direction. Area is an unsigned quantity, and the corner is\n"
"        the \"bottom left\" corner\n"
"       %s -p\n"
"Prunes any unconnected chunks from a world to save space.\n"
"       %s -f file\n"
"Prints out the raw NBT file structure of the given file.\n"
            , argv[0], argv[0], argv[0]);
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
    } else if (!strcmp(argv[1], "-o")) { // for testing base36 string generation
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

        cnbt::dimensionmanager *dm = &l.manager;
        cnbt::chunkmanager *cm = dm->get_chunk_manager(0);
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

            printf("THIS OPERATION CANNOT BE UNDONE\n");
            printf("ALWAYS MAKE A BACKUP BEFORE PRUNING\n");
            printf("IF YOU HAVE USED PORTALS, YOU MAY HAVE EXPLORED AREAS\n");
            printf("  THAT ARE UNCONNECTED\n");
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
        char *name, *out, *colorfile;
        cnbt::rendertype rt;
        cnbt::scoord origin(0, 0);
        cnbt::coord imagedim(0, 0);
        uint8_t dir;
        bool alc;
        int32_t dim;
        int ret;

        ret = parse_options(argc - 1, argv + 1, &name, &out, &colorfile, &rt, &dir, &alc, &origin, &imagedim, &dim);
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

        cnbt::game::blockcolors *bc;
        if (colorfile) {
            bc = cnbt::game::init_block_colors_from_file(colorfile);
            if (!bc) {
                return 1;
            }
        } else {
            bc = cnbt::game::init_block_colors(alc);
        }

        cnbt::dimensionmanager *dm = &l.manager;
        cnbt::chunkmanager *cm = dm->get_chunk_manager(dim);
        cnbt::renderer *r = cnbt::get_renderer(cm, rt, dir, bc);
        uint8_t *image;
        cnbt::coord size;
        printf("total bounding box is (%d, %d) to (%d, %d)\n", cm->max->x, cm->max->z, cm->min->x, cm->min->z);
        if (imagedim.first > 0 && imagedim.second > 0) {
            printf("rendered bounding box is (%d, %d) to (%ld, %ld)\n", origin.first, origin.second, origin.first - imagedim.first + 1, origin.second - imagedim.second + 1);
            image = r->render(origin, imagedim);
            size = r->image_size(origin, imagedim);
        } else {
            image = r->render_all();
            size = r->image_size();
        }
        free(bc);

        if (!image) { // all printing should happen inside render_all
            return 1;
        }
        cnbt::write_png_to_file(image, size.first, size.second, out);

        free(image);
        delete r;

        time_t end = time(NULL);

        printf("map render took %lu wallclock seconds\n", end - start);
    }

    return 0;
}
