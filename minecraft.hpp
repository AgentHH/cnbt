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
namespace game {
enum {
    CHANNEL_RED   = 0,
    CHANNEL_GREEN = 1,
    CHANNEL_BLUE  = 2,
    CHANNEL_ALPHA = 3,
};

enum {
    ALPHA_OPAQUE = 255,
    ALPHA_TRANSPARENT = 0,
};

enum blockflags {
    FLAG_TRANSPARENT  = 0x02,
    FLAG_INVALID      = 0x01,
};

struct blockcolors {
    uint8_t flags;
    uint8_t topcolor[128 * 4];
    uint8_t sidecolor[128 * 4];
};

void color_add_above(uint8_t *color, const uint8_t *above);
struct blockcolors *init_block_colors();
} // end namespace cnbt
} // end namespace game
