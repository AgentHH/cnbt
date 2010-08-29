#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifdef _MSC_VER
# include "pstdint.h"
#else
# include <stdint.h>
#endif

#ifdef _WIN32
# define PATH_SEPARATOR "\\"
#else
# define PATH_SEPARATOR "/"
#endif

namespace cnbt {
void filepath_merge(char **name, const char *a, const char *b);
} // namespace cnbt
