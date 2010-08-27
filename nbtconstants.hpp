/*
    Copyright 2010 Hans Nielsen (AgentHH)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
