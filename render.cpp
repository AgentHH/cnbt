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

    for (int _x = 0; _x < 16; _x++) {
        for (int _z = 0; _z < 16; _z++) {
            int ok = 1;
            uint8_t *src = &c->heightmap[(16 * _z ) + _x];
            uint8_t *dest = &buf[(16 * (h - (z + 1)) * (w * 16)) + //which row
                                 (16 * (w - (x + 1))) + // which column
                                 (16 - (_x + 1)) + // which block x
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
                uint8_t temp = *src;
                *dest = temp;
            }
        }
    }
}
} // end namespace cnbt
