// The code that is shared between all the utilitize of the suite.
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
    uint32_t *items;
    size_t count;
    size_t capacity;
} Tokens;

typedef struct {
    uint32_t l, r;
} Pair;

typedef struct {
    Pair *items;
    size_t count;
    size_t capacity;
} Pairs;
bool load_pairs(const char *file_path, Pairs *pairs, String_Builder *tmp_sb);
bool dump_pairs(const char *file_path, Pairs pairs);
bool dump_tokens(const char *file_path, Tokens tokens);
bool load_tokens(const char *file_path, Tokens *tokens, String_Builder *tmp_sb);
void render_token(Pairs pairs, uint32_t token, String_Builder *sb);
void c_strlit_escape_bytes(const char *bytes, size_t bytes_size, String_Builder *sb_out);

#define swap(Type, x, y) \
    do {                 \
        Type *a = &(x);  \
        Type *b = &(y);  \
        Type t = *a;     \
        *a = *b;         \
        *b = t;          \
    } while(0)

#endif // BPE_H_
