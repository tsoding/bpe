#ifndef BPE_H_
#define BPE_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define NOB_STRIP_PREFIX
#include "nob.h"
#undef rename

#define BPE_PRELUDE_SIZE 256

// TODO: support unicode
// Right now we assume everything is ASCII.

typedef struct {
    uint32_t l, r;
} Pair;

typedef struct {
    Pair *items;
    size_t count;
    size_t capacity;
} Pairs;

bool load_pairs(const char *file_path, Pairs *pairs, String_Builder *sb);

#endif // BPE_H_
