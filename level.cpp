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
#include "level.hpp"

namespace cnbt {
int parse_chunk_file_name(const char *name, int32_t *x, int32_t *z) {
    struct stream_eater s((uint8_t *)name, strlen(name));
    const char *temp;
    temp = (const char *)s.eat_byte_array(2);
    if (temp == NULL || strncmp(temp, "c.", 2))
        return 1;

    *x = s.eat_base36_int();

    temp = (const char *)s.eat_byte_array(1);
    if (temp == NULL || strncmp(temp, ".", 1))
        return 1;

    *z = s.eat_base36_int();

    temp = (const char*)s.eat_byte_array(4);
    if (temp == NULL || strncmp(temp, ".dat", 4))
        return 1;

    return 0;
}

#ifdef _WIN32
int find_chunk_files(struct dimensionmanager *dm, const char *path, int32_t dim) {
    WIN32_FIND_DATA finddata;
    HANDLE handle;

    char *searchpath;
    filepath_merge(&searchpath, path, "*");
    handle = FindFirstFile(searchpath, &finddata);
    free(searchpath);
    if (handle == INVALID_HANDLE_VALUE) {
        printf("Unable to open directory %s\n", searchpath);
        return 1;
    }

    do {
        if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (!strcmp(finddata.cFileName, ".") || !strcmp(finddata.cFileName, "..")) {
                continue;
            }
            char *newpath;
            int32_t newdim = dim;
            if (!memcmp(path, LEVEL_DIMENSION_TAG, sizeof(LEVEL_DIMENSION_TAG) - 1)) {
                if (strlen(path) > sizeof(LEVEL_DIMENSION_TAG) - 1 + 1) {
                    newdim = strtol(path + sizeof(LEVEL_DIMENSION_TAG) - 1 + 1, NULL, 0);
                    printf("entering new dimension %d\n", newdim);
                } else {
                    printf("Possibly malformed dimension in directory \"%s\"", path);
                }
            }
            filepath_merge(&newpath, path, finddata.cFileName);
            int ret = find_chunk_files(dm, newpath, newdim);
            free(newpath);
            if (ret) {
                FindClose(handle);
                return ret;
            }
        } else if (finddata.dwFileAttributes & (FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY)) {
            int32_t x, z;
            int ret = parse_chunk_file_name(finddata.cFileName, &x, &z);
            if (ret)
                continue;
            //printf("found chunk (%d,%d)\n", x, y);

            struct chunkcoord c(x, z, dim);
            ret = dm->add_new_chunk(c);
            if (ret) {
                printf("Warning: possible duplicate chunk found\n");
            }
        } else {
            continue;
        }
    } while (FindNextFile(handle, &finddata) != 0);

    FindClose(handle);
    return 0;
}
#else
int find_chunk_files(struct dimensionmanager *dm, const char *path, int32_t dim) {
    DIR *dir;
    struct dirent *dirp;

    dir = opendir(path);
    if (dir == NULL) {
        printf("Unable to open directory %s\n", path);
        return 1;
    }

    while ((dirp = readdir(dir))) {
        if (dirp->d_type != DT_DIR && dirp->d_type != DT_REG)
            continue;

        if (dirp->d_type == DT_DIR) {
            if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) {
                continue;
            }
            char *newpath;
            int32_t newdim = dim;
            filepath_merge(&newpath, path, dirp->d_name);

            if (!memcmp(dirp->d_name, LEVEL_DIMENSION_TAG, sizeof(LEVEL_DIMENSION_TAG) - 1)) {
                if (strlen(dirp->d_name) > sizeof(LEVEL_DIMENSION_TAG) - 1) {
                    newdim = strtol(dirp->d_name + sizeof(LEVEL_DIMENSION_TAG) - 1, NULL, 0);
                } else {
                    printf("Possibly malformed dimension in directory \"%s\"", path);
                }
            }
            int ret = find_chunk_files(dm, newpath, newdim);
            free(newpath);
            if (ret) {
                closedir(dir);
                return ret;
            }
        } else if (dirp->d_type == DT_REG) {
            int32_t x, z;
            int ret = parse_chunk_file_name(dirp->d_name, &x, &z);
            if (ret)
                continue;
            //printf("found chunk (%d,%d)\n", x, y);

            struct chunkcoord c(x, z, dim);
            ret = dm->add_new_chunk(c);
            if (ret) {
                printf("Warning: possible duplicate chunk found\n");
            }
        } else {
            continue;
        }
    }
    closedir(dir);

    return 0;
}
#endif
// {{{ level class methods
level::level(const char *path, const char *levelfile) : manager(path) {
    this->path = strdup(path);
    if (levelfile) {
        this->levelfile = strdup(levelfile);
    } else {
        this->levelfile = strdup(LEVEL_MAIN_FILE);
    }
    root = NULL;
}
level::~level() {
    free(path);
    free(levelfile);
    if (root)
        delete root;
}

int level::load() {
    if (!path)
        return 1;

    char *filepath;
    filepath_merge(&filepath, path, levelfile);
    struct tag *t = eat_nbt_file(filepath);
    free(filepath);
    if (t == NULL) {
        ERR("Tag parsing failed\n");
        return 1;
    }

    root = t;

    int ret = find_chunk_files(&this->manager, path, 0);
    if (ret) {
        ERR("Chunk file discovery failed\n");
        return 2;
    }
    return 0;
}
// }}}
} // end namespace cnbt
