#include "render.hpp"

namespace cnbt {
int write_png_to_file(uint8_t *buf, size_t w, size_t h, const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        ERR("Unable to open file \"%s\" for writing\n", filename);
        return 1;
    }

    png_structp pngp;
    png_infop infop;

    pngp = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!pngp) {
        ERR("png_create_write_struct failed\n");
        goto cleanup_write_png_pcws;
    }
    infop = png_create_info_struct(pngp);
    if (!infop) {
        ERR("png_create_write_struct failed\n");
        goto cleanup_write_png_pcis;
    }
    if (setjmp(png_jmpbuf(pngp))) {
        ERR("error during png_init_io\n");
        goto cleanup_write_png_error;
    }
    png_init_io(pngp, fp);
    if (setjmp(png_jmpbuf(pngp))) {
        ERR("error during png_set_IHDR\n");
        goto cleanup_write_png_error;
    }
    png_set_IHDR(pngp, infop, w, h, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(pngp, infop);
    if (setjmp(png_jmpbuf(pngp))) {
        ERR("error during png_write_image\n");
        goto cleanup_write_png_error;
    }
    for (size_t i = 0; i < h; i++) {
        png_write_row(pngp, buf + i*w*3);
    }
    if (setjmp(png_jmpbuf(pngp))) {
        ERR("error during png_write_end\n");
        goto cleanup_write_png_error;
    }
    png_write_end(pngp, NULL);

    printf("write succeeded\n");
    png_destroy_write_struct(&pngp, &infop);
    fclose(fp);

    return 0;

cleanup_write_png_error:
cleanup_write_png_pcis:
    //png_destroy_write_struct(pngp, infop);
cleanup_write_png_pcws:
    fclose(fp);
    return 1;
}

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

renderer::renderer(struct chunkmanager *cm, rendertype rt = RENDER_TOP_DOWN, uint8_t dir = 0) : cm(cm), rt(rt), dir(dir) {

}

/*uint8_t *renderer::render_all() {

    return NULL;
}*/

} // end namespace cnbt
