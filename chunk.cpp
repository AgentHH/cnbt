#include "chunk.hpp"

#define ERR(args...) fprintf(stderr, args)

namespace cnbt {
int chunkcoord_to_filename(struct chunkcoord c, uint8_t *name, size_t len) {
    struct stream_writer w(name, len);

    w.write_base36_int((uint32_t)c.x % 64);
    w.write_byte_array((uint8_t*)"/", 1);
    w.write_base36_int((uint32_t)c.z % 64);
    w.write_byte_array((uint8_t*)"/c.", 3);
    w.write_base36_int(c.x);
    w.write_byte_array((uint8_t*)".", 1);
    w.write_base36_int(c.z);
    w.write_byte_array((uint8_t*)".dat", 5);

    return w.written();
}

int parse_chunk_file_name(const char *name, int32_t *x, int32_t *z) {
    struct stream_eater s((uint8_t *)name, strlen(name));
    const char *temp;
    temp = (const char *)s.eat_byte_array(2);
    if (temp == NULL || strncmp(temp, "c.", 2))
        return 1;

    *x = s.eat_base36_int();

    temp = (const char *)s.eat_byte_array(1);
    if (temp == NULL || strncmp(temp, ".", 1))
        return 1;

    *z = s.eat_base36_int();

    temp = (const char*)s.eat_byte_array(4);
    if (temp == NULL || strncmp(temp, ".dat", 4))
        return 1;

    return 0;
}
int find_chunk_files(struct chunkmanager *cm, const char *path) {
    DIR *dir;
    struct dirent *dirp;

    dir = opendir(path);
    if (dir == NULL) {
        printf("Unable to open directory %s\n", path);
        return 1;
    }

    while ((dirp = readdir(dir))) {
        if (dirp->d_type != DT_DIR && dirp->d_type != DT_REG)
            continue;

        if (dirp->d_type == DT_DIR) {
            if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) {
                continue;
            }
            size_t newpathlen = strlen(path) + 1 + strlen(dirp->d_name) + 1;
            char newpath[newpathlen];
            strncpy(newpath, path, newpathlen);
            strncat(newpath, "/", newpathlen - strlen(path) - 1);
            strncat(newpath, dirp->d_name, newpathlen - strlen(path) - 2);

            find_chunk_files(cm, newpath);
        } else if (dirp->d_type == DT_REG) {
            int32_t x, z;
            int ret = parse_chunk_file_name(dirp->d_name, &x, &z);
            if (ret)
                continue;
            //printf("found chunk (%d,%d)\n", x, y);

            struct chunkcoord c(x, z);
            ret = cm->add_new_chunk(c);
            if (ret) {
                printf("Warning: possible duplicate chunk found\n");
                return ret;
            }
        } else {
            continue;
        }
    }

    closedir(dir);

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

chunkmanager::chunkmanager() {
    max = NULL;
    min = NULL;
}

int chunkmanager::add_new_chunk(struct chunkcoord c) {
    struct chunkinfo *ci = new chunkinfo(c);
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
} // end namespace cnbt
