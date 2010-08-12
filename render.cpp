#include "render.hpp"

namespace cnbt {

// filename is a temporary hack to see this thing working
void render_top_down(struct chunk *c, uint8_t *buf, int32_t x, int32_t z, int32_t w, int32_t h) {
    //FILE *fp = fopen(filename, "wb");
    //static const char header[] = "P5 16 16 255\n";
    //fwrite(header, sizeof(char), sizeof(header) - 1, fp);
    //fwrite(c->heightmap, sizeof(char), 256, fp);
    /*for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            uint8_t *col = &c->blocks[(x*16) + (z*16)];
            int y;
            for (y = 127; y >= 0; y--) {
                if (col[y]) {
                    break;
                }
            }
            fputc((y * 2), fp);
        }
    }*/
    //fclose(fp);

    // there are w items along the z-axis
    for (int _y = 0; _y < 128; _y++) {
        for (int _x = 0; _x < 16; _x++) {
            for (int _z = 0; _z < 16; _z++) {
                uint8_t block = c->blocks[_x * 2048 + _z * 128 + _y];
                uint8_t r, g, b;
                switch (block) {
                    case 0: // air
                        r = 0;
                        g = 0;
                        b = 0;
                        continue;
                    case 1: case 4: // stone and cobblestone
                        r = 159;
                        g = 159;
                        b = 159;
                        break;
                    case 2: // grass
                        r = 81 + (_y - 64);
                        g = 192 + (_y - 64);
                        b = 0;
                        break;
                    case 3: // dirt
                        r = 138 + (_y - 64);
                        g = 114 + (_y - 64);
                        b = 30;
                        break;
                    case 7: // adminium
                        r = 0;
                        g = 0;
                        b = 0;
                        break;
                    case 8: case 9: // water
                        r = 0;
                        g = 70;
                        b = 246;
                        break;
                    case 12: // sand
                        r = 192 + (_y - 64);
                        g = 192 + (_y - 64);
                        b = 50;
                        break;
                    case 18: // leaves
                        r = 54;
                        g = 134;
                        b = 0;
                        break;
                    case 36: // red flower
                        r = 255;
                        g = 0;
                        b = 0;
                        break;
                    case 37: // yellow flower
                        r = 255;
                        g = 255;
                        b = 0;
                        break;
                    case 73: // redstone
                        r = 164;
                        g = 0;
                        b = 0;
                        break;
                    default:
                        r = 255;
                        g = 255;
                        b = 255;
                        break;
                }

                uint8_t *dest = &buf[(
                                        (16 * (w - z - 1)) + // chunk z
                                        (16 * 16 * w * x) + // chunk x
                                        (16 - _z - 1) + // block z
                                        (16 * w * _x) // block x
                                     ) * 3 // RGB
                                ];
                dest[0] = r;
                dest[1] = g;
                dest[2] = b;
            }
        }
    }

#if 0
    for (int _x = 0; _x < 16; _x++) {
        for (int _z = 0; _z < 16; _z++) {
            int ok = 1;
            //uint8_t *src = &c->heightmap[(16 * _z) + _x];
            uint8_t *src = &c->heightmap[16 * _z];
            uint8_t *dest = &buf[(16 * (h - (z + 1)) * (w * 16)) + //which row
                                 (16 * (w - (x + 1))) + // which column
                                 //(16 - (_x + 1)) + // which block x
                                 (16 - (_z + 1)) * (w * 16)]; // which block z
            {
                int32_t a = reinterpret_cast<unsigned long>(dest);
                int32_t b = reinterpret_cast<unsigned long>(buf);
                int32_t c = (int32_t)w*h*256;
                if (a - b > c) {
                    printf("dest - buf = %d, w*h = %d\n", a - b, c);
                    ok = 0;
                }
                if (a - b < 0) {
                    printf("dest - buf = %d\n", a - b);
                    ok = 0;
                }
            }
            {
                int32_t a = reinterpret_cast<unsigned long>(src);
                int32_t b = reinterpret_cast<unsigned long>(c->heightmap);
                int32_t c = 256;
                if (a - b > c) {
                    printf("src - hm = %d, w*h = %d\n", a - b, c);
                    ok = 0;
                }
                if (a - b < 0) {
                    printf("src - hm = %d\n", a - b);
                    ok = 0;
                }
            }
            if (ok) {
                memcpy(dest, src, 16);
            }
        }
    }
#endif
}
} // end namespace cnbt
