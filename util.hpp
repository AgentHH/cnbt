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
#pragma once
// {{{ #includes
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <apr-1.0/apr_pools.h>
// }}}

#define _aprcall(expr) \
    do { \
        assert((expr) == APR_SUCCESS); \
    } while (false)

inline __attribute__((__always_inline__)) void *operator new(size_t size, apr_pool_t *p) {
    return apr_palloc(p, size);
}
inline __attribute__((__always_inline__)) void *operator new [](size_t size, apr_pool_t *p) {
    return apr_palloc(p, size);
}

namespace cnbt {
namespace util {
struct pool {
    apr_pool_t *p_;

    pool(apr_pool_t *p = NULL) {
        _aprcall(apr_pool_create(&p_, p));
    }
    ~pool() {
        apr_pool_destroy(p_);
    }
    void clear() {
        apr_pool_clear(p_);
    }
    operator apr_pool_t *() const {
        return p_;
    }
};
} // end namespace util
} // end namespace cnbt
