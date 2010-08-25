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
