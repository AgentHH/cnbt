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
    for (std::vector<struct tag *>::iterator i = u->children.begin(); i != u->children.end(); ++i) {
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

chunkmanager::chunkmanager(const char *path) {
    this->path = strdup(path);
    max = NULL;
    min = NULL;
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

int chunkmanager::add_new_chunk(struct chunkcoord c) {
    struct chunkinfo *ci = new struct chunkinfo(c);
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
        delete temp->c;
        temp->c = NULL;
        loadedchunks.pop_front();
    }
    loadedchunks.push_back(ci);

    return 0;
}

bool chunkmanager::chunk_exists(int32_t x, int32_t z) {
    struct chunkcoord c(x, z);
    return chunk_exists(c);
}

bool chunkmanager::chunk_exists(struct chunkcoord c) {
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
} // end namespace cnbt
