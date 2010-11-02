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
#include "platform.hpp"
namespace cnbt {
typedef std::pair<size_t, size_t> coord;
typedef std::pair<int32_t, int32_t> scoord;

struct chunkcoord {
    int32_t x, z, d;

    chunkcoord() : x(0), z(0), d(0) {}
    chunkcoord(int32_t x, int32_t z) : x(x), z(z), d(0) {}
    chunkcoord(int32_t x, int32_t z, int32_t d) : x(x), z(z), d(d) {}
    bool operator<(const chunkcoord &other) const {
        if (x < other.x)
            return true;
        if (x == other.x && z < other.z)
            return true;
        if (x == other.x && z == other.z && d < other.d)
            return true;
        return false;
    }
};

struct blockcoord {
    int32_t x;
    int8_t y;
    int32_t z;

    blockcoord(int32_t x, int8_t y, int32_t z) : x(x), y(y), z(z) {}
};

struct blockcoord chunkcoord_to_blockcoord(struct chunkcoord c);
struct chunkcoord blockcoord_to_chunkcoord(struct blockcoord b);

} // end namespace cnbt
