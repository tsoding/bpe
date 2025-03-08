#include "bpe.h"

bool load_pairs(const char *file_path, Pairs *pairs, String_Builder *sb)
{
    if (!read_entire_file(file_path, sb)) return false;
    if (sb->count%sizeof(*pairs->items) != 0) {
        fprintf(stderr, "ERROR: size of %s (%zu) must be divisible by %zu\n", file_path, sb->count, sizeof(*pairs->items));
        return false;
    }
    Pair *items = (void*)sb->items;
    size_t items_count = sb->count/sizeof(*pairs->items);
    for (size_t i = 0; i < items_count; ++i) {
        da_append(pairs, items[i]);
    }
    return true;
}

#define NOB_IMPLEMENTATION
#include "nob.h"
