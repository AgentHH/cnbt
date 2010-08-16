#include "minecraft.hpp"

namespace cnbt {
namespace game {

int color::add_above(struct color c) {
    // c is color a, this is color b
    // I am really bad at making this work well; only use it
    // when starting from a solid base...
    int thisalpha = (255 - c.a) * this->a / 255; // ?
    int newalpha = c.a + thisalpha;
    //printf("newalpha from %d and %d -> %d is %d\n", c.a, this->a, thisalpha, newalpha);
    if (newalpha > 255 || newalpha < 0) {
        printf("warning, alpha is more something (%d) than is possible\n", newalpha);
        if (newalpha > 255)
            newalpha = 255;
        if (newalpha < 0)
            newalpha = 0;
    }
    if (thisalpha <= 0) {
        this->r = c.r;
        this->g = c.g;
        this->b = c.b;
        this->a = newalpha;
    } else {
        this->r = ((c.r * c.a) + (this->r * thisalpha)) / 255;
        this->g = ((c.g * c.a) + (this->g * thisalpha)) / 255;
        this->b = ((c.b * c.a) + (this->b * thisalpha)) / 255;
        this->a = newalpha;
    }

    return this->a;
}

// pos is FULL SIZE of uint8_t
size_t linear_interpolate(size_t a, size_t b, uint8_t pos) {
    return a + pos * (b - a) / (255);
}

// position is from 0 to 256, 0 is towards x
struct color interpolate_color(struct color x, struct color y, uint8_t pos) {
    struct color ret;

    ret.r = linear_interpolate(x.r, y.r, pos);
    ret.g = linear_interpolate(x.g, y.g, pos);
    ret.b = linear_interpolate(x.b, y.b, pos);
    ret.a = linear_interpolate(x.a, y.a, pos);

    return ret;
}

int init_blocks(entitymap &map) {
    struct block_init {
        uint16_t id;
        const char *name;
        bool onecolor;
        struct color brightcolor;
        struct color darkcolor;
    };
    const static struct block_init blocks[] = {
        {0,  "air",                    true,  color(0, 0, 0, 0)},
        {1,  "stone",                  false, color(204, 204, 204), color(47, 47, 47)},
        {2,  "grass",                  false, color(0, 160, 0), color(134, 109, 48)},
        {3,  "dirt",                   true,  color(202, 178, 30)},
        {4,  "cobblestone",            false, color(204, 204, 204), color(47, 47, 47)},
        {5,  "wood",                   false, color(192, 148, 64), color(96, 74, 32)},
        {6,  "sapling",                true,  },
        {7,  "adminium",               true,  color(0, 0, 0)},
        {8,  "water",                  false, color(15, 145, 255, 96), color(0, 58, 144, 96)},
        {9,  "stationary water",       false, color(15, 145, 255, 96), color(0, 58, 144, 96)},
        {10, "lava",                   true,  },
        {11, "stationary lava",        true,  },
        {12, "sand",                   false, color(249, 250, 150), color(145, 145, 115)},
        {13, "gravel",                 true,  color(159, 159, 159)},
        {14, "gold ore",               true,  },
        {15, "iron ore",               true,  },
        {16, "coal ore",               true,  },
        {17, "log",                    false, color(192, 148, 64), color(96, 74, 32)},
        {18, "leaves",                 true,  color(32, 103, 32, 64)},
        {19, "sponge",                 true,  },
        {20, "glass",                  true,  color(0, 0, 0, 0)},
        {35, "cloth",                  true,  },
        {37, "yellow flower",          true,  color(255, 255, 0, 96)},
        {38, "red rose",               true,  color(255, 0, 0, 96)},
        {39, "brown mushroom",         true,  },
        {40, "red mushroom",           true,  },
        {41, "gold block",             true,  },
        {42, "iron block",             true,  },
        {43, "double stair",           true,  },
        {44, "stair",                  true,  },
        {45, "brick",                  true,  },
        {46, "TNT",                    true,  },
        {47, "bookcase",               true,  },
        {48, "mossy cobblestone",      true,  },
        {49, "obsidian",               true,  },
        {50, "torch",                  true,  color(255, 255, 9)},
        {51, "fire",                   true,  },
        {52, "mob spawner",            true,  },
        {53, "wooden stairs",          true,  },
        {54, "chest",                  true,  },
        {55, "redstone wire",          true,  },
        {56, "diamond ore",            true,  },
        {57, "diamond block",          true,  },
        {58, "workbench",              true,  },
        {59, "crops",                  true,  },
        {60, "soil",                   true,  },
        {61, "furnace",                true,  },
        {62, "burning furnace",        true,  },
        {63, "sign post",              true,  },
        {64, "wooden door (bottom)",   true,  },
        {65, "ladder",                 true,  },
        {66, "minecart rail",          true,  },
        {67, "cobblestone stairs",     false, color(206, 206, 206), color(49, 49, 49)},
        {68, "sign",                   true,  },
        {69, "lever",                  true,  },
        {70, "stone pressure plate",   true,  },
        {71, "iron door (bottom)",     true,  },
        {72, "wooden pressure plate",  true,  },
        {73, "redstone ore",           true,  },
        {74, "lighted redstone ore",   true,  },
        {75, "redstone torch (off)",   true,  },
        {76, "redstone torch (on)",    true,  },
        {77, "stone button",           true,  },
        {78, "snow",                   true,  },
        {79, "ice",                    true,  },
        {80, "snow block",             true,  },
        {81, "cactus",                 true,  },
        {82, "clay",                   true,  },
        {83, "reed",                   true,  },
        {84, "jukebox",                true,  },
        {255, NULL,                    true,  color(255, 255, 255)},
    };

    struct block_init *temp = (struct block_init*)&blocks[0];
    do {
        struct block *b;
        if (temp->onecolor)
            b = new block(temp->id, temp->name, temp->brightcolor, temp->brightcolor, temp->onecolor);
        else
            b = new block(temp->id, temp->name, temp->brightcolor, temp->darkcolor, temp->onecolor);
        map[temp->id] = b;
        temp++;
    } while (temp->name);
    return 0;
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
