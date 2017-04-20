#include "include/jemalloc/jemalloc.h"
void __wrap_free(void *p) {
    jefree(p);
}
void * __wrap_malloc(size_t size) {
    return jemalloc(size);
}
void * __wrap_calloc(size_t count, size_t size) {
    return jecalloc(count, size);
}
void * __wrap_realloc(void *ptr, size_t size) {
    return jerealloc(ptr, size);
}

