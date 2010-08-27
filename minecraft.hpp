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
// {{{ #includes
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <tr1/unordered_map>
// }}}
namespace cnbt {
namespace game {

struct color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a; // 255 is opaque; 0 is fully transparent

    color() : r(255), g(0), b(255), a(255) {}
    color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b), a(255) {}
    color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}

    int add_above(struct color c);
};
struct memorycolor {
    uint8_t *c;
    memorycolor(uint8_t *c) : c(c) {}

    int add_above(struct color c);
};
struct entity {
    uint16_t id;
    const char *name;

    entity(uint16_t id, const char *name) : id(id), name(name) {}
};
struct block : entity {
    struct color brightcolor, darkcolor;
    bool onecolor;

    //block(uint16_t id, const char *name, struct color color) : entity(id, name),
    //        brightcolor(color), darkcolor(color) {}
    block(uint16_t id, const char *name, struct color brightcolor, struct color darkcolor, bool onecolor) :
        entity(id, name), brightcolor(brightcolor), darkcolor(darkcolor), onecolor(onecolor) {}
};

typedef std::tr1::unordered_map<uint16_t, struct entity*> entitymap;

struct color interpolate_color(struct color x, struct color y, uint8_t pos);
int init_blocks(entitymap &map);
} // end namespace cnbt
} // end namespace game
