#ifndef BPE_H_
#define BPE_H_

#include <stdint.h>

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

#endif // BPE_H_
