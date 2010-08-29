#include "platform.hpp"

namespace cnbt {
void filepath_merge(char **name, const char *a, const char *b) {
    size_t len = strlen(a) + 1 + strlen(b) + 1;
    *name = (char*)malloc(sizeof(char) * len);
    assert(len == (size_t)snprintf(*name, len, "%s" PATH_SEPARATOR "%s", a, b) + 1);
}
} // namespace cnbt
