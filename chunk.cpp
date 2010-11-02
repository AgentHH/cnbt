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
#include "chunk.hpp"

namespace cnbt {
int chunkcoord_to_filename(struct chunkcoord c, uint8_t *name, int len) {
    struct stream_writer w(name, len);

    if (w.write_base36_int((uint32_t)c.x % 64)  ||
        w.write_byte_array((uint8_t*)"/", 1)    ||
        w.write_base36_int((uint32_t)c.z % 64)  ||
        w.write_byte_array((uint8_t*)"/c.", 3)  ||
        w.write_base36_int(c.x)                 ||
        w.write_byte_array((uint8_t*)".", 1)    ||
        w.write_base36_int(c.z)                 ||
        w.write_byte_array((uint8_t*)".dat", 5)) {
        return 1;
    }

    return 0;
}

int chunk::init(struct tag *t) {
    struct tag *u;
    // assuming current tag structure of a compound named "Level" in a compound
    if (t == NULL || t->type != TAG_COMPOUND)
        return 1;
    if (strcmp((char*)t->name, ""))
        return 1;
    if (t->children.size() != 1)
        return 1;
    u = t->children[0];
    if (u->type != TAG_COMPOUND)
        return 1;
    if (strcmp((char*)u->name, "Level"))
        return 1;
    // ok, u->children has everything we want now
    for (std::deque<struct tag *>::iterator i = u->children.begin(); i != u->children.end(); ++i) {
        char *name = (char*)(*i)->name;
        enum tagtype type = (*i)->type;
        if (name == NULL) {
            tags.push_back(*i);
        } else if (type == TAG_BYTE_ARRAY) {
            size_t elen = 0;
            void *dest = NULL;
            struct tag_byte_array *temp = dynamic_cast<struct tag_byte_array*>(*i);
            if (!strcmp(name, "Data")) {
                elen = CHUNK_DATA_LEN;
                dest = (void*)data;
            } else if (!strcmp(name, "SkyLight")) {
                elen = CHUNK_SKYLIGHT_LEN;
                dest = (void*)skylight;
            } else if (!strcmp(name, "HeightMap")) {
                elen = CHUNK_HEIGHTMAP_LEN;
                dest = (void*)heightmap;
            } else if (!strcmp(name, "BlockLight")) {
                elen = CHUNK_BLOCKLIGHT_LEN;
                dest = (void*)blocklight;
            } else if (!strcmp(name, "Blocks")) {
                elen = CHUNK_BLOCKS_LEN;
                dest = (void*)blocks;
            }
            if (dest) {
                if ((size_t)temp->num != elen) {
                    printf("Found right tag with wrong size\n");
                    return 1;
                }
                memcpy(dest, temp->value, elen);
                continue;
            }
        } else if (type == TAG_INT) {
            struct tag_int *temp = dynamic_cast<struct tag_int*>(*i);
            if (!strcmp(name, "xPos")) {
                x = temp->value;
                continue;
            } else if (!strcmp(name, "zPos")) {
                z = temp->value;
                continue;
            }
        } else if (type == TAG_BYTE) {
            struct tag_byte *temp = dynamic_cast<struct tag_byte*>(*i);
            if (!strcmp(name, "TerrainPopulated")) {
                terrainpopulated = temp->value;
                continue;
            }
        } else if (type == TAG_LONG) {
            struct tag_long *temp = dynamic_cast<struct tag_long*>(*i);
            if (!strcmp(name, "LastUpdate")) {
                timestamp = temp->value;
                continue;
            }
        }
        // catch-all, so we keep anything we don't care about
        tags.push_back(*i);
    }

    return 0;
}

chunkinfo::~chunkinfo() {
    if (c)
        delete c;
}

chunkmanager::chunkmanager(const char *path, int32_t dim) {
    if (path) {
        this->path = strdup(path);
    } else {
        this->path = "";
    }
    max = NULL;
    min = NULL;
    this->dim = dim;
}
chunkmanager::~chunkmanager() {
    free((void*)path);
    if (max)
        delete max;
    if (min)
        delete min;

    for (chunkmap::iterator i = chunks.begin(); i != chunks.end(); ++i) {
        struct chunkinfo *ci = (*i).second;
        if (ci)
            delete ci;
    }
}

int chunkmanager::delete_chunk(struct chunkcoord c) {
    chunkmap::iterator i = chunks.find(c);
    if (i == chunks.end())
        return 0;

    struct chunkinfo *ci = (*i).second;
    if (ci)
        delete ci;

    chunks.erase(i);

    return 1;
}

int chunkmanager::add_new_chunk(struct chunkcoord c) {
    struct chunkinfo *ci = new struct chunkinfo(c);
    if (c.d != dim) {
        return 2;
    }
    if (chunks.count(c)) {
        return 1;
    }
    chunks[c] = ci;

    if (!max && !min) {
        max = new struct chunkcoord(c);
        min = new struct chunkcoord(c);
    } else if (max && min) {
        if (max->x < c.x)
            max->x = c.x;
        if (max->z < c.z)
            max->z = c.z;
        if (min->x > c.x)
            min->x = c.x;
        if (min->z > c.z)
            min->z = c.z;
    } else {
        ERR("One of max and min were set; this should not happen\n");
        return 1;
    }

    return 0;
}

int chunkmanager::load_chunk_raw(struct chunkinfo *ci) {
    uint8_t buf[32];
    char *chunkpath;
    if (!ci) {
        ERR("chunkinfo passed to load_chunk_raw is NULL\n");
        return 1;
    }
    if (ci->coord.d != dim) {
        ERR("chunkinfo passed to load_chunk_raw had wrong dimension %d\n", ci->coord.d);
        return 1;
    }
    chunkcoord_to_filename(ci->coord, buf, 32);
    filepath_merge(&chunkpath, path, (char*)buf);
    struct tag *t = eat_nbt_file(chunkpath);
    free(chunkpath);
    if (!t) {
        ERR("Eating file for chunk %d, %d failed\n", ci->coord.x, ci->coord.z);
        return 1;
    }

    struct chunk *c = new struct chunk();
    int ret = c->init(t);
    delete t;
    if (ret) {
        delete c;
        ERR("Chunk parsing failed\n");
        return 1;
    }

    ci->c = c;
    return 0;
}

int chunkmanager::load_chunk(struct chunkinfo *ci) {
    if (ci->c)
        return 0;

    int ret = load_chunk_raw(ci);
    if (ret) {
        return 1;
    }
    while (loadedchunks.size() > CHUNKS_MAX_LOADED) {
        struct chunkinfo *temp = loadedchunks.front();
        if (temp) {
            delete temp->c;
            temp->c = NULL;
        }
        loadedchunks.pop_front();
    }
    loadedchunks.push_back(ci);

    return 0;
}

bool chunkmanager::chunk_exists(int32_t x, int32_t z) {
    struct chunkcoord c(x, z, dim);
    return chunk_exists(c);
}

bool chunkmanager::chunk_exists(struct chunkcoord c) {
    if (c.d != dim) {
        return false;
    }
    return chunks.find(c) != chunks.end();
}

struct chunkinfo *chunkmanager::get_chunk(struct chunkcoord c) {
    chunkmap::iterator i = chunks.find(c);
    if (i != chunks.end()) {
        struct chunkinfo *ci = (*i).second;
        int ret = load_chunk(ci);
        if (ret) {
            return NULL;
        }
        return ci;
    }
    return NULL;
}

typedef std::map<struct chunkcoord, size_t> chunkctog;
typedef std::map<size_t, chunklist*> chunkgtol;

cnbt::chunkcoord find_centroid(chunklist *cl) {
    // XXX FIXME: this has the potential to fail on gigantic maps
    ptrdiff_t sx = 0, sz = 0, n = 0;
    for (chunklist::iterator ci = cl->begin(); ci != cl->end(); ++ci) {
        n++;
        sx += (*ci).x;
        sz += (*ci).z;
    }
    return chunkcoord(sx / n, sz / n);
}

void merge_groups(chunkctog &chunktogroup,
                  chunkgtol &grouptolist,
                  chunkcoord co, chunkcoord cn) {
    chunkctog::iterator cni = chunktogroup.find(cn);
    size_t ng = 0;
    if (cni != chunktogroup.end()) {
        ng = (*cni).second;
    } else {
        return; // possible neighbor does not exist, do nothing
    }
    chunkctog::iterator coi = chunktogroup.find(co);
    size_t og = 0;
    if (coi != chunktogroup.end()) {
        og = (*coi).second;
    }
    if (ng == og) { // already part of same group
        return;
    } else if (og == 0) { // not in a group, and there is a group to join
        chunktogroup[co] = ng;
        grouptolist[ng]->push_back(co);
    } else { // group conflict
        size_t a, b;
        if (ng < og) {
            a = ng;
            b = og;
        } else {
            a = og;
            b = ng;
        }
        chunklist *al = grouptolist[a];
        chunklist *bl = grouptolist[b];
        for (chunklist::iterator bi = bl->begin(); bi != bl->end(); ++bi) {
            chunktogroup[*bi] = a;
            al->push_back(*bi);
        }
        delete bl;
        grouptolist.erase(b);
    }
}

int chunkmanager::prune_chunks(chunklist *cl) {
    if (!cl)
        return 0;

    int deleted = 0;
    for (chunklist::iterator i = cl->begin(); i != cl->end(); ++i) {
        deleted += delete_chunk(*i);
    }

    return deleted;
}

int chunkmanager::prune_chunks_on_disk(chunklist *cl) {
    if (!cl)
        return 0;

    int deleted = 0;
    for (chunklist::iterator i = cl->begin(); i != cl->end(); ++i) {
        char buf[32];
        chunkcoord_to_filename(*i, (uint8_t*)buf, 32);
        char *chunkpath;
        filepath_merge(&chunkpath, path, (char*)buf);
        int ret = unlink(chunkpath);
        if (ret) {
            ERR("Unable to remove \"%s\"\n", chunkpath);
        } else {
            deleted += 1;
        }
        free(chunkpath);
    }

    return deleted;
}

std::deque<chunklist*> *chunkmanager::find_chunk_groups() {
    if (chunks.empty())
        return NULL;

    // XXX FIXME: if the chunks are read in just the right way and there are
    //              enough of them, we'll run out of ints
    size_t groupmax = 1; // 0 is special
    chunkctog chunktogroup;
    chunkgtol grouptolist;
    for (chunkmap::iterator i = chunks.begin(); i != chunks.end(); ++i) {
        struct chunkcoord c = (*i).first;
        merge_groups(chunktogroup, grouptolist, c, chunkcoord(c.x, c.z + 1));
        merge_groups(chunktogroup, grouptolist, c, chunkcoord(c.x + 1, c.z));
        merge_groups(chunktogroup, grouptolist, c, chunkcoord(c.x, c.z - 1));
        merge_groups(chunktogroup, grouptolist, c, chunkcoord(c.x - 1, c.z));

        chunkctog::iterator ci = chunktogroup.find(c);
        size_t g = 0;
        if (ci != chunktogroup.end()) {
            g = (*ci).second;
            //printf("chunk (%d,%d) has been merged into group %lu\n",
            //        c.x, c.z, g);
        } else {
            //printf("chunk (%d,%d) has no neighbors, adding as group %lu\n",
            //        c.x, c.z, groupmax);
            chunktogroup[c] = groupmax;
            grouptolist[groupmax] = new chunklist();
            grouptolist[groupmax]->push_back(c);
            groupmax++;
        }
    }

    std::deque<chunklist*> *ret = new std::deque<chunklist*>();
    for (chunkgtol::iterator gi = grouptolist.begin(); gi != grouptolist.end(); ++gi) {
        //size_t g = (*gi).first;
        //chunklist *l = (*gi).second;
        ret->push_back((*gi).second);
        //printf("Group %lu has %lu chunks in it\n", g, l->size());
    }
    return ret;
}

dimensionmanager::dimensionmanager(const char *path) {
    if (path) {
        this->path = strdup(path);
    } else {
        this->path = "";
    }
}

dimensionmanager::~dimensionmanager() {
    free((void*)path);
    for (chunkmanagermap::iterator i = chunkmanagers.begin(); i != chunkmanagers.end(); ++i) {
        struct chunkmanager *cm = (*i).second;
        if (cm)
            delete cm;
    }
}

int dimensionmanager::add_new_chunk(struct chunkcoord c) {
    int dim = c.d;
    struct chunkmanager *cm = get_chunk_manager(dim);
    if (!cm) {
        cm = add_new_chunk_manager(dim);
    }

    return cm->add_new_chunk(c);
}

struct chunkmanager *dimensionmanager::add_new_chunk_manager(int32_t dim) {
    struct chunkmanager *cm = NULL;

    if (chunkmanagers[dim])
        return chunkmanagers[dim];

    if (dim == 0) {
        cm = new chunkmanager(path, dim); // special case for main world
    } else if (dim > 0) {
        char *newpath;
        char dimdir[16];
        snprintf(dimdir, 16, "%s%d", LEVEL_DIMENSION_TAG, dim);
        filepath_merge(&newpath, path, dimdir);
        cm = new chunkmanager(newpath, dim);
        free(newpath);
    } else {
        printf("Attempting to add a dimension less than zero (%d)\n", dim);
        return NULL;
    }

    chunkmanagers[dim] = cm;

    return cm;
}

struct chunkmanager *dimensionmanager::get_chunk_manager(int32_t dim) {
    return chunkmanagers[dim];
}

} // end namespace cnbt
