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
#include "colorfile.hpp"

namespace cnbt {
namespace game {
namespace colorfile {

tokendata::tokendata(char *s) {
    if (s) {
        this->s = strdup(s);
    } else {
        this->s = strdup("");
    }
    len = strlen(s);
    pos = 0;
    first = true;
}

tokendata::~tokendata() {
    free(s);
}

char *tokendata::next_token() {
    if (pos == len) {
        return s + len;
    }

    if (first) {
        first = false;
        pos = strspn(s, COLORFILE_WHITESPACE);
        char *ns = s + pos;
        size_t nlen = strcspn(ns, COLORFILE_WHITESPACE);
        ns[nlen] = '\0';
        return ns;
    }

    pos += strlen(s + pos) + 1;
    if (pos >= len) {
        pos = len;
        return s + len;
    }
    pos += strspn(s + pos, COLORFILE_WHITESPACE);
    char *ns = s + pos;
    size_t nlen = strcspn(ns, COLORFILE_WHITESPACE);
    ns[nlen] = '\0';
    return ns;
}

char *eat_whitespace(char *c) {
    if (!c)
        return NULL;
    size_t num = strspn(c, COLORFILE_WHITESPACE);
    return c + num;
}

bool is_comment(char *line) {
    if (!line)
        return false;

    char *first = eat_whitespace(line);
    if (!strncmp(first, COLORFILE_COMMENT, sizeof(COLORFILE_COMMENT) - 1)) {
        return true;
    }
    return false;
}

bool is_end_of_line(char *line) {
    if (!line)
        return false;

    char *first = eat_whitespace(line);
    if (strlen(first) < 1 || is_comment(first)) {
        return true;
    }
    return false;
}

tokendata *read_line(FILE *fp, size_t *line) {
    char temp[CHARS_PER_LINE];
    do {
        size_t readlen = CHARS_PER_LINE;
        long start;

        start = ftell(fp);
        size_t ret = fread(temp, sizeof(char), readlen, fp);
        if (ret == 0) {
            return NULL;
        }

        char *newline = strchr(temp, '\n');
        if (!newline) {
            return NULL;
        }

        size_t len = newline - temp;
        fseek(fp, start + len + 1, SEEK_SET);

        temp[len] = '\0';

        (*line)++;
    } while (is_end_of_line(temp));
    return new tokendata(temp);
}

int eat_preamble(FILE *fp, size_t *line) {
    int ret = 0;
    tokendata *t;
    char *name, *version, *newline;

    t = read_line(fp, line);
    if (!t) {
        return -1;
    }
    name = t->next_token();
    if (strcmp(name, COLORFILE_NAME)) {
        ret = -2;
        goto eat_preamble_exit;
    }
    version = t->next_token();
    if (strcmp(version, COLORFILE_VERSION)) {
        ret = -3;
        goto eat_preamble_exit;
    }
    newline = t->next_token();
    if (!is_end_of_line(newline)) {
        ret = -4;
        goto eat_preamble_exit;
    }

eat_preamble_exit:
    delete t;
    return ret;
}

int32_t eat_options(FILE *fp, size_t *line) {
    int32_t ret = 0;
    tokendata *t;
    char *options, *token;

    t = read_line(fp, line);
    if (!t) {
        return -1;
    }
    options = t->next_token();
    if (strcmp(options, "options:")) {
        printf("Expected options on line %lu, got \"%s\" instead\n", *line, options);
        printf("Even if no options are being used, a line with \"options:\" at the front is required\n");
        ret = -2;
        goto eat_options_exit;
    }
    while ((token = t->next_token())) {
        if (is_end_of_line(token)) {
            goto eat_options_exit;
        }
        const colorfile_option *o = &COLORFILE_OPTIONS[0];
        while (o->value != COLORFILE_OPT_NONE) {
            if (!strcmp(token, o->name)) {
                break;
            }
            o++;
        }
        if (o->value != COLORFILE_OPT_NONE) {
            ret |= o->value;
        } else {
            printf("Unknown option in color file \"%s\"\n", token);
        }
    }

eat_options_exit:
    delete t;
    return ret;
}

int eat_separator(FILE *fp, size_t *line) {
    int ret = 0;
    tokendata *t;
    char *separator, *newline;

    t = read_line(fp, line);
    if (!t) {
        return -1;
    }
    separator = t->next_token();
    if (strlen(separator) == 0 || strspn(separator, COLORFILE_SEPARATOR) != strlen(separator)) {
        ret = -2;
        goto eat_separator_exit;
    }
    newline = t->next_token();
    if (!is_end_of_line(newline)) {
        ret = -3;
        goto eat_separator_exit;
    }

eat_separator_exit:
    delete t;
    return ret;
}

int16_t eat_value(char *s, size_t *line, bool block) {
    char *end;
    long value = strtol(s, &end, 0);
    if (block) {
        if (*end != '\0') {
            printf("\"%s\" is not a valid block ID on line %lu\n", s, *line);
            return -1;
        }
    }
    if ((uint8_t)value != value) {
        printf("Warning, %ld on line %lu is interpreted as %d\n", value, *line, (uint8_t)value);
    }

    return (uint8_t)value;
}

int eat_colorline(FILE *fp, size_t *line, color *cs) {
    int ret = 0;
    tokendata *t;
    long value;
    char *block, *flag, *triplet, *temp;
    uint8_t flagchar, cur;
    uint8_t found = 0;
    uint8_t result[4], *dest;
    int i;

    if (!cs) {
        return -2;
    }
    t = read_line(fp, line);
    if (!t) {
        return -1;
    }
    block = t->next_token();
    if (strlen(block) == 0) {
        return -3;
    }
    if (!strcmp(block, "e")) {
        cs->blocktype = COLORFILE_BLOCK_ERROR;
        cs->blockid = 255;
    } else if (!strcmp(block, "b")) {
        cs->blocktype = COLORFILE_BLOCK_BACKGROUND;
        cs->blockid = 255;
    } else if (!strcmp(block, "s")) {
        cs->blocktype = COLORFILE_BLOCK_SIDESHADE;
        cs->blockid = 255;
    } else if (!strcmp(block, "a")) {
        cs->blocktype = COLORFILE_BLOCK_ALTERNATESHADE;
        cs->blockid = 255;
    } else {
        value = eat_value(block, line, true);
        if (value < 0) {
            ret = -4;
            goto eat_colorline_exit;
        }
        cs->blocktype = COLORFILE_BLOCK_NORMAL;
        cs->blockid = value;
    }
    while (1) {
        memset(result, 255, 4);
        flag = t->next_token();
        triplet = t->next_token();
        if (is_end_of_line(flag)) {
            break;
        }
        if (is_end_of_line(triplet)) {
            printf("Flag found, but no color provided on line %lu\n", *line);
            ret = -5;
            goto eat_colorline_exit;
        }
        flagchar = flag[0]; // '\0' if empty string
        switch (flagchar) {
            case 's':
                if (found & COLORFILE_C_SIDE_DARK) {
                    printf("Warning, already found side dark color (s) on line %lu\n", *line);
                    continue;
                }
                cur = COLORFILE_C_SIDE_DARK;
                break;
            case 'S':
                if (found & COLORFILE_C_SIDE_BRIGHT) {
                    printf("Warning, already found side bright color (S) on line %lu\n", *line);
                    continue;
                }
                cur = COLORFILE_C_SIDE_BRIGHT;
                break;
            case 't':
                if (found & COLORFILE_C_TOP_DARK) {
                    printf("Warning, already found top dark color (t) on line %lu\n", *line);
                    continue;
                }
                cur = COLORFILE_C_TOP_DARK;
                break;
            case 'T':
                if (found & COLORFILE_C_TOP_BRIGHT) {
                    printf("Warning, already found top bright color (T) on line %lu\n", *line);
                    continue;
                }
                cur = COLORFILE_C_TOP_BRIGHT;
                break;
            case '\0':
                printf("This should never happen %s() %s:%d\n", __FUNCTION__, __FILE__, __LINE__);
            default:
                printf("Invalid color flag '%c' specified on line %lu\n", flagchar, *line);
                printf("Valid flags are t, T, s, and S, for dark/bright top/side colors\n");
                ret = -6;
                goto eat_colorline_exit;
        }
        found |= cur;
        if (strspn(triplet, "0123456789,") != strlen(triplet)) {
            printf("Color \"%s\" on line %lu contains invalid characters\n", triplet, *line);
            printf("Colors must contain only numbers and be separated by ,\n");
            ret = -7;
            goto eat_colorline_exit;
        }
        temp = triplet;
        i = 0;
        do {
            if (temp != triplet) {
                temp++;
            }
            if (i > 3) {
                printf("Color \"%s\" on line %lu has too many parts\n", triplet, *line);
                ret = -8;
                goto eat_colorline_exit;
            }
            value = eat_value(temp, line, false);
            if (value < 0) {
                ret = -9;
                goto eat_colorline_exit;
            }
            result[i++] = value;
        } while ((temp = strchr(temp, ',')));
        if (i < 2) {
            printf("Color \"%s\" on line %lu has too few parts\n", triplet, *line);
            printf("Perhaps there are too many spaces in it?\n");
            ret = -10;
            goto eat_colorline_exit;
        }
        if (i == 3) {
            result[CHANNEL_ALPHA] = ALPHA_OPAQUE;
        }
        switch (cur) {
            case COLORFILE_C_TOP_BRIGHT:
                dest = cs->top_bright;
                break;
            case COLORFILE_C_TOP_DARK:
                dest = cs->top_dark;
                break;
            case COLORFILE_C_SIDE_BRIGHT:
                dest = cs->side_bright;
                break;
            case COLORFILE_C_SIDE_DARK:
                dest = cs->side_dark;
                break;
            default:
                printf("This should never happen %s() %s:%d\n", __FUNCTION__, __FILE__, __LINE__);
                continue;
        }
        memcpy(dest, result, 4);
    }

    if (found == 0) {
        printf("No colors specified for block \"%s\" on line %lu\n", block, *line);
        ret = -11;
        goto eat_colorline_exit;
    }

    if (found & COLORFILE_C_TOP_BOTH) {
        if (found & COLORFILE_C_TOP_BRIGHT && !(found & COLORFILE_C_TOP_DARK)) {
            memcpy(cs->top_dark, cs->top_bright, 4);
        } else if (!(found & COLORFILE_C_TOP_BRIGHT) && found & COLORFILE_C_TOP_DARK) {
            memcpy(cs->top_bright, cs->top_dark, 4);
        }
    }
    if (found & COLORFILE_C_SIDE_BOTH) {
        if (found & COLORFILE_C_SIDE_BRIGHT && !(found & COLORFILE_C_SIDE_DARK)) {
            memcpy(cs->side_dark, cs->side_bright, 4);
        } else if (!(found & COLORFILE_C_SIDE_BRIGHT) && found & COLORFILE_C_SIDE_DARK) {
            memcpy(cs->side_bright, cs->side_dark, 4);
        }
    }
    if (found & COLORFILE_C_TOP_BOTH && !(found & COLORFILE_C_SIDE_BOTH)) {
        memcpy(cs->side_dark, cs->top_dark, 4);
        memcpy(cs->side_bright, cs->top_bright, 4);
    } else if (!(found & COLORFILE_C_TOP_BOTH) && found & COLORFILE_C_SIDE_BOTH) {
        memcpy(cs->top_dark, cs->side_dark, 4);
        memcpy(cs->top_bright, cs->side_bright, 4);
    }

eat_colorline_exit:
    delete t;
    return ret;
}
} // end namespace colorfile

void print_color(uint8_t *color) {
    if (color[3] != 255) {
        printf("%d %d %d %d\n", color[0], color[1], color[2], color[3]);
    } else {
        printf("%d %d %d\n", color[0], color[1], color[2]);
    }
}

struct blockcolors *init_block_colors_from_file(char *name) {
    int ret;
    size_t line = 0;
    int32_t options;
    bool shade_sides = false, alternate = false;
    blockcolors *bc = NULL;
    colorfile::color cs;
    colorfile::color e = {{255, 0, 255, 255}, {255, 0, 255, 255}, {255, 0, 255, 255}, {255, 0, 255, 255}, 255, colorfile::COLORFILE_BLOCK_ERROR};
    colorfile::color b = {{0, 0, 0, 255}, {0, 0, 0, 255}, {0, 0, 0, 255}, {0, 0, 0, 255}, 255, colorfile::COLORFILE_BLOCK_BACKGROUND};
    colorfile::color s = {{0, 0, 0, 128}, {0, 0, 0, 128}, {0, 0, 0, 128}, {0, 0, 0, 128}, 255, colorfile::COLORFILE_BLOCK_SIDESHADE};
    colorfile::color a = {{255, 255, 255, 24}, {255, 255, 255, 24}, {255, 255, 255, 24}, {255, 255, 255, 24}, 255, colorfile::COLORFILE_BLOCK_ALTERNATESHADE};
    FILE *fp = fopen(name, "r");
    if (!fp) {
        return NULL;
    }

    ret = colorfile::eat_preamble(fp, &line);
    if (ret < 0) {
        printf("\"%s\" does not appear to be a valid color file\n", name);
        printf("Is the first line \"%s %s\"?\n", colorfile::COLORFILE_NAME, colorfile::COLORFILE_VERSION);
        goto init_block_colors_from_file_exit;
    }

    options = colorfile::eat_options(fp, &line);
    if (options < 0) {
        goto init_block_colors_from_file_exit;
    }
    if (options & colorfile::COLORFILE_OPT_SHADESIDES) {
        shade_sides = true;
    }
    if (options & colorfile::COLORFILE_OPT_ALTERNATE) {
        alternate = true;
    }

    ret = colorfile::eat_separator(fp, &line);
    if (ret < 0) {
        printf("Section separator not present\n");
        printf("Make sure there is a line of ---- between the options and colors\n");
        goto init_block_colors_from_file_exit;
    }

    bc = new blockcolors[NUM_COLORS + 1];
    for (int i = 0; i < NUM_COLORS; i++) {
        bc[i].flags = FLAG_INVALID;
    }
    while ((ret = colorfile::eat_colorline(fp, &line, &cs)) >= 0) {
        switch (cs.blocktype) {
            case colorfile::COLORFILE_BLOCK_ERROR:
                e = cs;
                break;
            case colorfile::COLORFILE_BLOCK_BACKGROUND:
                b = cs;
                break;
            case colorfile::COLORFILE_BLOCK_SIDESHADE:
                s = cs;
                break;
            case colorfile::COLORFILE_BLOCK_ALTERNATESHADE:
                a = cs;
                break;
            default: {
                struct blockcolors *bct = &bc[cs.blockid];

                if (bct->flags != FLAG_INVALID) {
                    printf("Color for block %d was previously set; overriding\n", cs.blockid);
                }
                bct->flags = 0;
                if (cs.top_bright[CHANNEL_ALPHA] == ALPHA_TRANSPARENT
                    && cs.top_dark[CHANNEL_ALPHA] == ALPHA_TRANSPARENT) {
                    bct->flags |= FLAG_TRANSPARENT_TOP;
                }
                if (cs.side_bright[CHANNEL_ALPHA] == ALPHA_TRANSPARENT
                    && cs.side_dark[CHANNEL_ALPHA] == ALPHA_TRANSPARENT) {
                    bct->flags |= FLAG_TRANSPARENT_SIDE;
                }
                if (bct->flags & (FLAG_TRANSPARENT_TOP | FLAG_TRANSPARENT_SIDE)) {
                    continue;
                }
                if (shade_sides) {
                    color_add_above(cs.side_bright, s.side_bright);
                    color_add_above(cs.side_dark, s.side_dark);
                }
                for (int i = 0; i < 128; i++) {
                    uint8_t *topcolor = &bct->topcolor[i * 4];
                    uint8_t *sidecolor = &bct->sidecolor[i * 4];
                    if (!(bct->flags & FLAG_TRANSPARENT_TOP)) {
                        interpolate_color(cs.top_dark, cs.top_bright, i, topcolor);
                        if (alternate && i % 2) {
                            if (topcolor[CHANNEL_ALPHA] == ALPHA_OPAQUE) {
                                uint8_t temp[4];
                                interpolate_color(a.top_dark, a.top_bright, i, temp);
                                color_add_above(topcolor, temp);
                            }
                        }
                    }
                    if (!(bct->flags & FLAG_TRANSPARENT_SIDE)) {
                        interpolate_color(cs.side_dark, cs.side_bright, i, sidecolor);
                        if (alternate && i % 2) {
                            if (sidecolor[CHANNEL_ALPHA] == ALPHA_OPAQUE) {
                                uint8_t temp[4];
                                interpolate_color(a.side_dark, a.side_bright, i, temp);
                                color_add_above(sidecolor, temp);
                            }
                        }
                    }
                }
                break;
            }
        }
    }
    for (int i = 0; i < NUM_COLORS; i++) {
        struct blockcolors *bct = &bc[i];
        if (bct->flags == FLAG_INVALID) {
            for (int j = 0; j < 128; j++) {
                uint8_t *topcolor = &bct->topcolor[j * 4];
                uint8_t *sidecolor = &bct->sidecolor[j * 4];
                interpolate_color(e.top_dark, e.top_bright, j, topcolor);
                interpolate_color(e.side_dark, e.side_bright, j, sidecolor);
            }
        }
    }
    {
        struct blockcolors *bct = &bc[BACKGROUND_COLOR]; // background
        bct->flags = FLAG_BACKGROUND;
        for (int i = 0; i < 128; i++) {
            uint8_t *topcolor = &bct->topcolor[i * 4];
            uint8_t *sidecolor = &bct->sidecolor[i * 4];
            interpolate_color(b.top_dark, b.top_bright, i, topcolor);
            interpolate_color(b.side_dark, b.side_bright, i, sidecolor);
        }
    }
    if (ret < 0 && ret != -1) {
        free(bc);
        bc = NULL;
        goto init_block_colors_from_file_exit;
    }

init_block_colors_from_file_exit:
    fclose(fp);
    return bc;
}

} // end namespace game
} // end namespace cnbt
