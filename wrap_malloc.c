#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <cuda_runtime.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <regex.h>
#include <stdbool.h>

#include "./cuda-install/include/jemalloc/jemalloc.h"

void    *__libc_malloc(size_t size);
void    __libc_free(void* ptr);
void    *__libc_realloc(void* ptr, size_t size);
void    *__libc_calloc(size_t n, size_t size);

static regex_t cuda_lib_r;
static const char * cuda_lib_regex = ".*x. .*lib(nvidia-fatbinaryloader|cuda(rt)?|pthread|dl)[-.so0-9]*$";


static struct range {
    ptrdiff_t start;
    ptrdiff_t end;
} ranges[10] = {};
static int nranges = 0;


static int first = 1;
static int in_init =0;
__thread int wrap_in_cudamalloc = 0;
static void check_init() {
    if (!first) return;
    first = 0;
    in_init = 1;
    /* fprintf(stderr, "initializing malloc wrapper\n"); */
    if(regcomp(&cuda_lib_r, cuda_lib_regex, REG_NEWLINE | REG_EXTENDED))
        perror("regcomp");
    char *line = malloc(500);
    size_t len = 500;
    FILE *f = fopen("/proc/self/maps", "r");
    ssize_t read;
    while((read = getline(&line, &len, f)) > 0) {
        if(!regexec(&cuda_lib_r, line, 0, NULL, 0)) {
            /* fprintf(stderr, "found matching line[%d]: %s\n", nranges, line); */
            if(nranges >= 5)
                exit(21);
            if(sscanf(line, "%tx-%tx", &ranges[nranges].start, &ranges[nranges].end) != 2)
                perror("sscanf");
            nranges++;
        }else {
            /* fprintf(stderr, "found non-matching line:%zu %s\n", read, line); */
        }
    }
    // try to init cuda here
    /* cudaFree(0); */
    /* void * handle = dlopen("./cuda-install/lib/libjemalloc.so.2", RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND); */
    /* if (!handle){ */
    /*     perror(dlerror()); */
    /*     assert(0); */
    /* } */
    /* jemalloc = dlsym(handle, "jemalloc"); */
    /* jecalloc = dlsym(handle, "jecalloc"); */
    /* jerealloc = dlsym(handle, "jerealloc"); */
    /* jefree = dlsym(handle, "jefree"); */
    in_init=0;
}
static bool in_range(ptrdiff_t p) {
    int i;
    for (i=0; i<nranges; ++i) {
        if (p >= ranges[i].start && p <= ranges[i].end)
            return true;
    }
    return false;
}
/* __attribute__((constructor)) */
/* static void init_at_start() { */
/*     check_init(); */
/* } */

void free(void *p) {
    check_init();
    if (!in_init && !wrap_in_cudamalloc && !in_range((ptrdiff_t)__builtin_return_address(0))){
        jefree(p);
    }else {
        __libc_free(p);
    }
}
void * malloc(size_t size) {
    check_init();
    if (!in_init && !wrap_in_cudamalloc && !in_range((ptrdiff_t)__builtin_return_address(0))){
        return jemalloc(size);
    }else {
        return __libc_malloc(size);
    }
}
void * calloc(size_t count, size_t size) {
    check_init();
    if (!in_init && !wrap_in_cudamalloc && !in_range((ptrdiff_t)__builtin_return_address(0))){
        return jecalloc(count, size);
    }else {
        return __libc_calloc(count, size);
    }
}
void * realloc(void *ptr, size_t size) {
    check_init();
    if (!in_init && !wrap_in_cudamalloc && !in_range((ptrdiff_t)__builtin_return_address(0))){
        return jerealloc(ptr, size);
    }else {
        return __libc_realloc(ptr, size);
    }
}

