#include "level.hpp"

#define ERR(args...) fprintf(stderr, args)

namespace cnbt {
// {{{ level class methods
level::level(char *path) : path(path) {
    root = NULL;
}
level::~level() {
    delete root;
}

int level::load_chunk_list() {
    return find_chunk_files(this, path);
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
    return 0;
}
// }}}
} // end namespace cnbt
