#pragma once

namespace cnbt {
// {{{ tag enums
enum tagtype {
    TAG_END = 0,
    TAG_BYTE,
    TAG_SHORT,
    TAG_INT,
    TAG_LONG,
    TAG_FLOAT,
    TAG_DOUBLE,
    TAG_BYTE_ARRAY,
    TAG_STRING,
    TAG_LIST,
    TAG_COMPOUND,

    TAG_INVALID = 100,
    TAG_UNKNOWN,
};
// }}}
} // end namespace cnbt
