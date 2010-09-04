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
#include "minecraft.hpp"

namespace cnbt {
namespace game {

// color must be solid
void color_add_above(uint8_t *color, const uint8_t *above) {
    if (above[CHANNEL_ALPHA] == ALPHA_OPAQUE) {
        memcpy(color, above, 3);
        return;
    }

    uint8_t above_alpha = above[CHANNEL_ALPHA];
    uint8_t color_alpha = ALPHA_OPAQUE - above_alpha;

    color[CHANNEL_RED]   = ((above[CHANNEL_RED] * above_alpha)
                            + (color[CHANNEL_RED] * color_alpha)) / ALPHA_OPAQUE;
    color[CHANNEL_GREEN] = ((above[CHANNEL_GREEN] * above_alpha)
                            + (color[CHANNEL_GREEN] * color_alpha)) / ALPHA_OPAQUE;
    color[CHANNEL_BLUE]  = ((above[CHANNEL_BLUE] * above_alpha)
                            + (color[CHANNEL_BLUE] * color_alpha)) / ALPHA_OPAQUE;
}

// pos is FULL SIZE of uint8_t
inline uint8_t linear_interpolate(uint8_t a, uint8_t b, uint8_t pos) {
    return a + pos * (b - a) / (255);
}

// position is from 0 to 256, 0 is towards x
inline void interpolate_color(uint8_t *x, uint8_t *y, uint8_t pos, uint8_t *a) {
    a[CHANNEL_RED]   = linear_interpolate(x[CHANNEL_RED], y[CHANNEL_RED], pos);
    a[CHANNEL_GREEN] = linear_interpolate(x[CHANNEL_GREEN], y[CHANNEL_GREEN], pos);
    a[CHANNEL_BLUE]  = linear_interpolate(x[CHANNEL_BLUE], y[CHANNEL_BLUE], pos);
    a[CHANNEL_ALPHA] = linear_interpolate(x[CHANNEL_ALPHA], y[CHANNEL_ALPHA], pos);
}

// this should probably be turned into a load from file so colors can be easily adjusted
// I also want to add better color curves instead of just interpolation between two points
struct blockcolors *init_block_colors() {
    struct block_init {
        uint16_t id;
        const char *name;
        bool onecolor;
        uint8_t brightcolor[4];
        uint8_t darkcolor[4];
    };
    const static struct block_init blocks[] = {
        {0,  "air",                    true,  {  0,   0,   0,   0}},
        {1,  "stone",                  false, {204, 204, 204, 255}, { 47,  47,  47, 255}},
        {2,  "grass",                  false, { 10, 224,  10, 255}, {  0,  96,   0, 255}},
        {3,  "dirt",                   true,  {202, 178,  30, 255}},
        {4,  "cobblestone",            false, {204, 204, 204, 255}, { 47,  47,  47, 255}},
        {5,  "wood",                   false, {192, 148,  64, 255}, { 96,  74,  32, 255}},
        {6,  "sapling",                true,  {161, 182,  78, 192}},
        {7,  "adminium",               true,  {  0,   0,   0, 255}},
        {8,  "water",                  false, { 15, 145, 255, 128}, {  0,  58, 144, 128}},
        {9,  "stationary water",       false, { 15, 145, 255, 128}, {  0,  58, 144, 128}},
        {10, "lava",                   false, {255, 209,  49, 255}, {213,  79,  13, 255}},
        {11, "stationary lava",        false, {255, 209,  49, 255}, {213,  79,  13, 255}},
        {12, "sand",                   false, {249, 250, 150, 255}, {145, 145, 115, 255}},
        {13, "gravel",                 true,  {159, 159, 159, 255}},
        {14, "gold ore",               true,  {192, 192,   0, 255}},
        {15, "iron ore",               true,  {158, 141, 112, 255}},
        {16, "coal ore",               true,  { 38,  38,  38, 255}},
        {17, "log",                    false, {192, 148,  64, 255}, { 96,  74,  32, 255}},
        {18, "leaves",                 true,  { 32, 103,  32,  64}},
        {19, "sponge",                 true,  {230, 230,   0, 255}},
        {20, "glass",                  true,  {221, 245, 244,  64}},
        {35, "cloth",                  true,  {213, 213, 213, 255}},
        {37, "yellow flower",          true,  {230, 232,  28, 192}},
        {38, "red rose",               true,  {227,  91,  94, 192}},
        {39, "brown mushroom",         true,  {145, 117,  67, 192}},
        {40, "red mushroom",           true,  {227,  48,  49, 192}},
        {41, "gold block",             true,  {226, 207,   0, 255}},
        {42, "iron block",             true,  {155, 146, 133, 255}},
        {43, "double stair",           false, {232, 232, 232, 255}, { 92,  92,  92, 255}},
        {44, "stair",                  false, {230, 230, 230, 255}, { 90,  90,  90, 255}},
        {45, "brick",                  false, {192,  92, 101, 255}, { 75,   0,   7, 255}},
        {46, "TNT",                    true,  {226,   0,  11, 255}},
        {47, "bookcase",               true,  {167, 152, 107, 255}},
        {48, "mossy cobblestone",      true,  {133, 155, 133, 255}},
        {49, "obsidian",               false, {  0,   0,  68, 255}, {  0,   0, 202, 255}},
        {50, "torch",                  true,  {255, 255,   9, 255}},
        {51, "fire",                   true,  {255, 127,   0, 191}},
        {52, "mob spawner",            true,  {  0,   0,   0,   0}},
        {53, "wooden stairs",          false, {194, 150,  66, 255}, { 98,  76,  34, 255}},
        {54, "chest",                  false, {188, 144,  60, 255}, { 92,  70,  28, 255}},
        {55, "redstone wire",          true,  {255,   0,   0, 128}},
        {56, "diamond ore",            true,  {113, 228, 229, 255}},
        {57, "diamond block",          true,  {  5, 252, 255, 255}},
        {58, "workbench",              false, {180, 140,  55, 255}, { 85,  60,  20, 255}},
        {59, "crops",                  true,  { 68, 192,   0, 192}},
        {60, "soil",                   true,  {202, 178,  30, 255}},
        {61, "furnace",                false, {192, 192, 192, 255}, { 40,  40,  40, 255}},
        {62, "burning furnace",        false, {192, 192, 192, 255}, { 40,  40,  40, 255}},
        {63, "sign post",              true,  {  0,   0,   0,   0}},
        {64, "wooden door (bottom}",   true,  {  0,   0,   0,   0}},
        {65, "ladder",                 true,  {  0,   0,   0,   0}},
        {66, "minecart rail",          true,  {141, 126,  99, 224}},
        {67, "cobblestone stairs",     false, {206, 206, 206, 255}, { 49,  49,  49, 255}},
        {68, "sign",                   true,  {  0,   0,   0,   0}},
        {69, "lever",                  true,  {  0,   0,   0,   0}},
        {70, "stone pressure plate",   false, {198, 198, 198, 255}, { 41,  41,  41, 255}},
        {71, "iron door (bottom}",     true,  {  0,   0,   0,   0}},
        {72, "wooden pressure plate",  true,  {168, 142,   0, 255}},
        {73, "redstone ore",           true,  {161,  45,  45, 255}},
        {74, "lighted redstone ore",   true,  {200,  60,  60, 255}},
        {75, "redstone torch (off}",   true,  {178,   0,   0, 255}},
        {76, "redstone torch (on}",    true,  {255,   0,   0, 255}},
        {77, "stone button",           true,  {  0,   0,   0,   0}},
        {78, "snow",                   false, {255, 255, 255, 224}, {224, 224, 224, 224}},
        {79, "ice",                    true,  {255, 255, 255,  10}},
        {80, "snow block",             false, {255, 255, 255, 255}, {224, 224, 224, 255}},
        {81, "cactus",                 false, {101, 255, 101, 255}, {  0, 226,   0, 255}},
        {82, "clay",                   true,  {140, 147, 117, 255}},
        {83, "reed",                   true,  {177, 244, 170, 255}},
        {84, "jukebox",                true,  {160, 162, 189, 255}},
        {85, "fence",                  true,  {  0,   0,   0, 255}},
        {255, NULL,                    true,  {255,   0, 255, 255}},
    };

    struct blockcolors *bc = (struct blockcolors*)malloc(sizeof(struct blockcolors) * 256);
    if (!bc)
        return NULL;

    static const uint8_t invalidcolor[4] = {255, 0, 255, 255};
    for (int i = 0; i < 256; i++) {
        struct blockcolors *bct = &bc[i];
        bct->flags = FLAG_INVALID;
        for (int j = 0; j < 128; j++) {
            memcpy(&bct->topcolor[j * 4], invalidcolor, 4);
            memcpy(&bct->sidecolor[j * 4], invalidcolor, 4);
        }
    }

    struct block_init *temp = (struct block_init*)&blocks[0];
    do {
        uint8_t brightcolor[4], darkcolor[4];
        struct blockcolors *bct = &bc[temp->id];

        bct->flags = 0;
        memcpy(brightcolor, temp->brightcolor, 4);
        if (temp->onecolor) {
            memcpy(darkcolor, temp->brightcolor, 4);
        } else {
            if (!memcmp(temp->brightcolor, temp->darkcolor, 4)) {
                printf("block %s (%d) has two colors that are the same\n", temp->name, temp->id);
            }
            memcpy(darkcolor, temp->darkcolor, 4);
        }
        if (darkcolor[CHANNEL_ALPHA] == 0 && brightcolor[CHANNEL_ALPHA] == 0) {
            bct->flags |= FLAG_TRANSPARENT;
            temp++;
            continue;
        }
        for (int i = 0; i < 128; i++) {
            uint8_t newcolor[4];
            static const uint8_t sideshade[4] = {0, 0, 0, 128};
            interpolate_color(darkcolor, brightcolor, i, newcolor);
            memcpy(&bct->topcolor[i * 4], newcolor, 4);
            color_add_above(newcolor, sideshade);
            memcpy(&bct->sidecolor[i * 4], newcolor, 4);
        }

        temp++;
    } while (temp->name);
    return bc;
}

/*
    {256,   "iron shovel",            },
    {257,   "iron pickaxe",           },
    {258,   "iron axe",               },
    {259,   "flint and steel",        },
    {260,   "apple",                  },
    {261,   "bow",                    },
    {262,   "arrow",                  },
    {263,   "coal",                   },
    {264,   "diamond",                },
    {265,   "iron ingot",             },
    {266,   "gold ingot",             },
    {267,   "iron sword",             },
    {268,   "wooden sword",           },
    {269,   "wooden shovel",          },
    {270,   "wooden pickaxe",         },
    {271,   "wooden axe",             },
    {272,   "stone sword",            },
    {273,   "stone shovel",           },
    {274,   "stone pickaxe",          },
    {275,   "stone axe",              },
    {276,   "diamond sword",          },
    {277,   "diamond shovel",         },
    {278,   "diamond pickaxe",        },
    {279,   "diamond axe",            },
    {280,   "stick",                  },
    {281,   "bowl",                   },
    {282,   "mushroom soup",          },
    {283,   "gold sword",             },
    {284,   "gold shovel",            },
    {285,   "gold pickaxe",           },
    {286,   "gold axe",               },
    {287,   "string",                 },
    {288,   "feather",                },
    {289,   "gunpowder",              },
    {290,   "wooden hoe",             },
    {291,   "stone hoe",              },
    {292,   "iron hoe",               },
    {293,   "diamond hoe",            },
    {294,   "gold hoe",               },
    {295,   "seeds",                  },
    {296,   "wheat",                  },
    {297,   "bread",                  },
    {298,   "leather helmet",         },
    {299,   "leather chestplate",     },
    {300,   "leather pants",          },
    {301,   "leather boots",          },
    {302,   "chainmail helmet",       },
    {303,   "chainmail chestplate",   },
    {304,   "chainmail pants",        },
    {305,   "chainmail boots",        },
    {306,   "iron helmet",            },
    {307,   "iron chestplate",        },
    {308,   "iron pants",             },
    {309,   "iron boots",             },
    {310,   "diamond helmet",         },
    {311,   "diamond chestplate",     },
    {312,   "diamond pants",          },
    {313,   "diamond boots",          },
    {314,   "gold helmet",            },
    {315,   "gold chestplate",        },
    {316,   "gold pants",             },
    {317,   "gold boots",             },
    {318,   "flint",                  },
    {319,   "pork",                   },
    {320,   "grilled pork",           },
    {321,   "paintings",              },
    {322,   "golden apple",           },
    {323,   "sign",                   },
    {324,   "wooden door",            },
    {325,   "bucket",                 },
    {326,   "water bucket",           },
    {327,   "lava bucket",            },
    {328,   "minecart",               },
    {329,   "saddle",                 },
    {330,   "iron door",              },
    {331,   "redstone",               },
    {332,   "snowball",               },
    {333,   "boat",                   },
    {334,   "leather",                },
    {335,   "milk bucket",            },
    {336,   "clay brick",             },
    {337,   "clay balls",             },
    {338,   "reed",                   },
    {339,   "paper",                  },
    {340,   "book",                   },
    {341,   "slime ball",             },
    {342,   "storage minecart",       },
    {343,   "powered minecart",       },
    {344,   "egg",                    },
    {2256,  "gold record",            },
    {2257,  "green record",           },

    {65535, NULL,                     color(255, 255, 255)},
};
*/
/*
const uint8_t *lookup_thing(uint16_t id) {
    struct _thing_to_name *temp = (struct _thing_to_name*)&names[0];
    do {
        if (id == temp->id) {
            return (const uint8_t*)temp->name;
        }
        temp++;
    } while (temp->name);

    return (uint8_t*)"(unknown)";
}
*/
} // end namespace game
} // end namespace cnbt
