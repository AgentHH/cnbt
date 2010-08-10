#include "level.hpp"

#define ERR(args...) fprintf(stderr, args)

namespace cnbt {
// {{{ level class methods
level::level(char *path) : path(path) {
    root = NULL;
}
int level::load() {
    uint8_t raw[LEVEL_COMPRESSED_BUFFER_SIZE], data[LEVEL_UNCOMPRESSED_BUFFER_SIZE];
    size_t len;
    int ret;

    size_t filepathlen = strlen(path) + 1 + sizeof(LEVEL_MAIN_FILE);
    char filepath[filepathlen];
    strncpy(filepath, path, filepathlen);
    strncat(filepath, "/", filepathlen - strlen(path) - 1);
    strncat(filepath, LEVEL_MAIN_FILE, filepathlen - strlen(path) - 2);

    FILE *fp = fopen(filepath, "rb");
    ret = fread(raw, sizeof(uint8_t), LEVEL_COMPRESSED_BUFFER_SIZE, fp);
    fclose(fp);
    if (!ret) {
        ERR("No data returned while reading " LEVEL_MAIN_FILE "\n");
        return 1;
    } else if (ret == LEVEL_COMPRESSED_BUFFER_SIZE) {
        ERR("File was longer than %d\n", LEVEL_COMPRESSED_BUFFER_SIZE);
        return 1;
    }

    ret = decompress_data(raw, ret, data, &len);
    if (ret) {
        return 1;
    }

    struct tag *t = parse_tags(data, len);
    if (t == NULL) {
        ERR("Tag parsing failed\n");
        return 1;
    }

    root = t;
    return 0;
}
// }}}
} // end namespace cnbt
