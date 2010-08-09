#ifndef __DATASTREAM_HPP
#define __DATASTREAM_HPP
// {{{ #includes
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <byteswap.h>
#include <assert.h>
#include <arpa/inet.h>
// }}}
// {{{ stuff to help with NBT
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
// {{{ evil hacks to support network floating point values
float ntohf(float f) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    union {
        float x;
        uint32_t y;
    };

    x = f;
    y = __bswap_32(y);
    return x;
#else
    return f;
#endif
}
double ntohd(double d) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    union {
        double x;
        uint64_t y;
    };

    x = d;
    y = __bswap_64(y);
    return x;
#else
    return d;
#endif
}
uint64_t ntohll(uint64_t i) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return __bswap_64(i);
#else
    return i;
#endif
}
// }}}
// {{{ stream eater to simplify extracting data
struct stream_eater {
    uint8_t *buf;
    size_t len, pos;

    stream_eater(uint8_t *buf, size_t len) : buf(buf), len(len) {
        pos = 0;
    }

    size_t remain() {
        return len - pos;
    }
    size_t eaten() {
        return pos;
    }

    uint8_t eat_bool() {
        assert(remain() >= sizeof(uint8_t));
        uint8_t ret = buf[pos] ? 1 : 0;
        pos += sizeof(uint8_t);
        return ret;
    }
    enum tagtype peek_tag() {
        if (remain() < sizeof(uint8_t))
            return TAG_INVALID;
        enum tagtype ret = (enum tagtype)buf[pos];
        return ret;
    }
    int eat_tag(enum tagtype t) {
        if (remain() < sizeof(uint8_t))
            return 1;
        enum tagtype ret = (enum tagtype)buf[pos];
        pos += sizeof(uint8_t);
        if (t == TAG_UNKNOWN)
            return ret;
        if (ret != t) {
            return 2;
        }
        return 0;
    }
    uint8_t eat_byte() {
        assert(remain() >= sizeof(uint8_t));
        uint8_t ret = buf[pos];
        pos += sizeof(uint8_t);
        return ret;
    }
    uint16_t eat_short() {
        assert(remain() >= sizeof(uint16_t));
        uint16_t ret = ntohs(*(uint16_t*)&buf[pos]);
        pos += sizeof(uint16_t);
        return ret;
    }
    uint32_t eat_int() {
        assert(remain() >= sizeof(uint32_t));
        uint32_t ret = ntohl(*(uint32_t*)&buf[pos]);
        pos += sizeof(uint32_t);
        return ret;
    }
    uint64_t eat_long() {
        assert(remain() >= sizeof(uint64_t));
        uint64_t ret = ntohll(*(uint64_t*)&buf[pos]);
        pos += sizeof(uint64_t);
        return ret;
    }
    float eat_float() {
        assert(remain() >= sizeof(float));
        float ret = ntohf(*(float*)&buf[pos]);
        pos += sizeof(float);
        return ret;
    }
    double eat_double() {
        assert(remain() >= sizeof(double));
        double ret = ntohd(*(double*)&buf[pos]);
        pos += sizeof(double);
        return ret;
    }
    uint8_t *eat_byte_array(uint32_t l) {
        if (remain() < l * sizeof(uint8_t))
            return NULL;
        uint8_t *ret = &buf[pos];
        pos += l * sizeof(uint8_t);
        return ret;
    }
    uint16_t *eat_short_array(uint32_t l) {
        if (remain() < l * sizeof(uint16_t))
            return NULL;
        uint16_t *ret = (uint16_t*)&buf[pos];
        pos += l * sizeof(uint16_t);
        return ret;
    }
    uint8_t *eat_string() {
        // warning: this function allocates memory
        if (remain() < sizeof(uint16_t))
            return NULL;
        uint16_t s = eat_short();
        if (remain() < s) {
            return NULL;
        }
        uint8_t *ret = (uint8_t*)strndup((const char*)&buf[pos], s);
        pos += s * sizeof(uint8_t);
        return ret;
    }
};
// }}}
// {{{ stuff to look up blocks / items
const uint8_t *lookup_thing(uint16_t id) {
    struct _thing_to_name {
        uint16_t id;
        const char *name;
    };
    const static struct _thing_to_name names[] = {
        {0, "air"},
        {1, "stone"},
        {2, "grass"},
        {3, "dirt"},
        {4, "cobblestone"},
        {5, "wood"},
        {6, "sapling"},
        {7, "adminium"},
        {8, "water"},
        {9, "stationary water"},
        {10, "lava"},
        {11, "stationary lava"},
        {12, "sand"},
        {13, "gravel"},
        {14, "gold ore"},
        {15, "iron ore"},
        {16, "coal ore"},
        {17, "log"},
        {18, "leaves"},
        {19, "sponge"},
        {20, "glass"},
        {35, "cloth"},
        {37, "yellow flower"},
        {38, "red rose"},
        {39, "brown mushroom"},
        {40, "red mushroom"},
        {41, "gold block"},
        {42, "iron block"},
        {43, "double stair"},
        {44, "stair"},
        {45, "brick"},
        {46, "TNT"},
        {47, "bookcase"},
        {48, "mossy cobblestone"},
        {49, "obsidian"},
        {50, "torch"},
        {51, "fire"},
        {52, "mob spawner"},
        {53, "wooden stairs"},
        {54, "chest"},
        {55, "redstone wire"},
        {56, "diamond ore"},
        {57, "diamond block"},
        {58, "workbench"},
        {59, "crops"},
        {60, "soil"},
        {61, "furnace"},
        {62, "burning furnace"},
        {63, "sign post"},
        {64, "wooden door (bottom)"},
        {65, "ladder"},
        {66, "minecart rail"},
        {67, "cobblestone stairs"},
        {68, "sign"},
        {69, "lever"},
        {70, "stone pressure plate"},
        {71, "iron door (bottom)"},
        {72, "wooden pressure plate"},
        {73, "redstone ore"},
        {74, "lighted redstone ore"},
        {75, "redstone torch (off)"},
        {76, "redstone torch (on)"},
        {77, "stone button"},
        {78, "snow"},
        {79, "ice"},
        {80, "snow block"},
        {81, "cactus"},
        {82, "clay"},
        {83, "reed"},
        {84, "jukebox"},
        {256, "iron shovel"},
        {257, "iron pickaxe"},
        {258, "iron axe"},
        {259, "flint and steel"},
        {260, "apple"},
        {261, "bow"},
        {262, "arrow"},
        {263, "coal"},
        {264, "diamond"},
        {265, "iron ingot"},
        {266, "gold ingot"},
        {267, "iron sword"},
        {268, "wooden sword"},
        {269, "wooden shovel"},
        {270, "wooden pickaxe"},
        {271, "wooden axe"},
        {272, "stone sword"},
        {273, "stone shovel"},
        {274, "stone pickaxe"},
        {275, "stone axe"},
        {276, "diamond sword"},
        {277, "diamond shovel"},
        {278, "diamond pickaxe"},
        {279, "diamond axe"},
        {280, "stick"},
        {281, "bowl"},
        {282, "mushroom soup"},
        {283, "gold sword"},
        {284, "gold shovel"},
        {285, "gold pickaxe"},
        {286, "gold axe"},
        {287, "string"},
        {288, "feather"},
        {289, "gunpowder"},
        {290, "wooden hoe"},
        {291, "stone hoe"},
        {292, "iron hoe"},
        {293, "diamond hoe"},
        {294, "gold hoe"},
        {295, "seeds"},
        {296, "wheat"},
        {297, "bread"},
        {298, "leather helmet"},
        {299, "leather chestplate"},
        {300, "leather pants"},
        {301, "leather boots"},
        {302, "chainmail helmet"},
        {303, "chainmail chestplate"},
        {304, "chainmail pants"},
        {305, "chainmail boots"},
        {306, "iron helmet"},
        {307, "iron chestplate"},
        {308, "iron pants"},
        {309, "iron boots"},
        {310, "diamond helmet"},
        {311, "diamond chestplate"},
        {312, "diamond pants"},
        {313, "diamond boots"},
        {314, "gold helmet"},
        {315, "gold chestplate"},
        {316, "gold pants"},
        {317, "gold boots"},
        {318, "flint"},
        {319, "pork"},
        {320, "grilled pork"},
        {321, "paintings"},
        {322, "golden apple"},
        {323, "sign"},
        {324, "wooden door"},
        {325, "bucket"},
        {326, "water bucket"},
        {327, "lava bucket"},
        {328, "minecart"},
        {329, "saddle"},
        {330, "iron door"},
        {331, "redstone"},
        {332, "snowball"},
        {333, "boat"},
        {334, "leather"},
        {335, "milk bucket"},
        {336, "clay brick"},
        {337, "clay balls"},
        {338, "reed"},
        {339, "paper"},
        {340, "book"},
        {341, "slime ball"},
        {342, "storage minecart"},
        {343, "powered minecart"},
        {344, "egg"},
        {2256, "gold record"},
        {2257, "green record"},

        {65535, NULL}
    };

    struct _thing_to_name *temp = (struct _thing_to_name*)&names[0];
    do {
        if (id == temp->id) {
            return (const uint8_t*)temp->name;
        }
        temp++;
    } while (temp->name);

    return (uint8_t*)"(unknown)";
}
// }}}

#endif
