#include "level.hpp"

#define ERR(args...) fprintf(stderr, args)

namespace cnbt {
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
            snprintf(newpath, newpathlen, "%s/%s", path, dirp->d_name);

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
// {{{ level class methods
level::level(char *path) : path(path) {
    root = NULL;
    manager.path = path;
}
level::~level() {
    if (root)
        delete root;
}

int level::load() {
    if (!path)
        return 0;

    size_t filepathlen = strlen(path) + 1 + strlen(LEVEL_MAIN_FILE) + 1;
    char filepath[filepathlen];
    snprintf(filepath, filepathlen, "%s/%s", path, LEVEL_MAIN_FILE);

    struct tag *t = eat_nbt_file(filepath);
    if (t == NULL) {
        ERR("Tag parsing failed\n");
        return 1;
    }

    root = t;

    int ret = find_chunk_files(&this->manager, path);
    if (ret) {
        ERR("Chunk file discovery failed\n");
        delete t;
        return 1;
    }
    return 0;
}
// }}}
} // end namespace cnbt
