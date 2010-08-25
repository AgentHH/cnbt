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
    apr_dir_t *dir;
    apr_finfo_t finfo;
    apr_status_t status;
    util::pool p;

    status = apr_dir_open(&dir, path, p);
    if (status != APR_SUCCESS) {
        printf("Unable to open directory %s\n", path);
        return 1;
    }

    while (apr_dir_read(&finfo, APR_FINFO_NAME | APR_FINFO_TYPE, dir) == APR_SUCCESS) {
        if (finfo.filetype != APR_DIR && finfo.filetype != APR_REG)
            continue;

        if (finfo.filetype == APR_DIR) {
            if (!strcmp(finfo.name, ".") || !strcmp(finfo.name, "..")) {
                continue;
            }
            char *newpath;
            apr_filepath_merge(&newpath, path, finfo.name, APR_FILEPATH_NATIVE, p);
            int ret = find_chunk_files(cm, newpath);
            if (ret) {
                return ret;
            }
        } else if (finfo.filetype == APR_REG) {
            int32_t x, z;
            int ret = parse_chunk_file_name(finfo.name, &x, &z);
            if (ret)
                continue;
            //printf("found chunk (%d,%d)\n", x, y);

            struct chunkcoord c(x, z);
            ret = cm->add_new_chunk(c);
            if (ret) {
                printf("Warning: possible duplicate chunk found\n");
            }
        } else {
            continue;
        }
    }
    apr_dir_close(dir);

    return 0;
}
// {{{ level class methods
level::level(char *path) : manager(path) {
    this->path = strdup(path);
    root = NULL;
}
level::~level() {
    free(path);
    if (root)
        delete root;
}

int level::load() {
    if (!path)
        return 1;

    apr_status_t status;
    apr_pool_t *pool;

    status = apr_pool_create(&pool, NULL);
    if (status != APR_SUCCESS) {
        return 1;
    }
    char *filepath;
    apr_filepath_merge(&filepath, path, LEVEL_MAIN_FILE, APR_FILEPATH_NATIVE, pool);

    struct tag *t = eat_nbt_file(filepath);
    if (t == NULL) {
        ERR("Tag parsing failed\n");
        return 1;
    }

    root = t;

    int ret = find_chunk_files(&this->manager, path);
    apr_pool_destroy(pool);
    if (ret) {
        ERR("Chunk file discovery failed\n");
        return 2;
    }
    return 0;
}
// }}}
} // end namespace cnbt
