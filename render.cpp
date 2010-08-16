#include "render.hpp"

namespace cnbt {

void render_top_down(struct chunk *c, uint8_t *buf, int32_t x, int32_t z, int32_t w, int32_t h, game::entitymap &em) {
    //printf("looking at chunk %d, %d (%d, %d) %p %p\n", x, z, w, h, c, buf);

    // there are w items along the z-axis
    for (int _x = 0; _x < 16; _x++) {
        for (int _z = 0; _z < 16; _z++) {
            game::color pixel(255, 255, 255);
            uint8_t *dest = &buf[(
                                    (16 * (w - z - 1)) + // chunk z
                                    (16 * 16 * w * x) + // chunk x
                                    (16 - _z - 1) + // block z
                                    (16 * w * _x) // block x
                                 ) * 3 // RGB
                            ];
            for (int _y = 0; _y < 128; _y++) {
                uint8_t id = c->blocks[_x * 2048 + _z * 128 + _y];
                if (id == 0)
                    continue;

                struct game::block *b = static_cast<struct game::block *>(em[id]);
                if (b == NULL) {
                    printf("Block not found in hash\n");
                    continue;
                }

                if (b->onecolor)
                    pixel.add_above(b->brightcolor);
                else
                    pixel.add_above(game::interpolate_color(b->darkcolor, b->brightcolor, _y));
            }
            dest[0] = pixel.r;
            dest[1] = pixel.g;
            dest[2] = pixel.b;
        }
    }
}
} // end namespace cnbt
