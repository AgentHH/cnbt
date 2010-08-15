#include "level.hpp"

#define ERR(args...) fprintf(stderr, args)

namespace cnbt {
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
    size_t filepathlen = strlen(path) + 1 + sizeof(LEVEL_MAIN_FILE);
    char filepath[filepathlen];
    strncpy(filepath, path, filepathlen);
    strncat(filepath, "/", filepathlen - strlen(path) - 1);
    strncat(filepath, LEVEL_MAIN_FILE, filepathlen - strlen(path) - 2);

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
