#include "platform.hpp"

namespace cnbt {
void filepath_merge(char **name, const char *a, const char *b) {
    size_t len = strlen(a) + 1 + strlen(b) + 1;
    *name = (char*)malloc(sizeof(char) * len);

#   ifdef _WIN32
    // XXX FIXME this is an evil hack
    assert(len - 1 == (size_t)_snprintf(*name, len, "%s" PATH_SEPARATOR "%s", a, b));
#   else
    assert(len - 1 == (size_t)snprintf(*name, len, "%s" PATH_SEPARATOR "%s", a, b));
#   endif
}
} // namespace cnbt
