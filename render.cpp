#include "render.hpp"

namespace cnbt {
// filename is a temporary hack to see this thing working
void render_top_down(struct chunk *c, char *filename) {
    FILE *fp = fopen(filename, "wb");
    static const char header[] = "P5 16 16 255\n";
    fwrite(header, sizeof(char), sizeof(header), fp);
    for (int i = 0; i < 256; i++) {
        char buf[16];
        snprintf(buf, 16, "%d ", c->heightmap[i]);
        fwrite(buf, sizeof(char), strlen(buf), fp);
        if ((i + 1 % 16) == 0) {
            fwrite("\n", sizeof(char), 1, fp);
        }
    }
    fclose(fp);
}
} // end namespace cnbt
