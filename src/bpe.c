#include "bpe.h"

bool load_pairs(const char *file_path, Pairs *pairs, String_Builder *sb)
{
    if (!read_entire_file(file_path, sb)) return false;
    if (sb->count%sizeof(*pairs->items) != 0) {
        nob_log(ERROR, "%s: file size in bytes (%zu) must be divisible by %zu", file_path, sb->count, sizeof(*pairs->items));
        return false;
    }
    Pair *items = (void*)sb->items;
    size_t items_count = sb->count/sizeof(*pairs->items);

    if (items_count < BPE_PRELUDE_SIZE) {
        nob_log(ERROR, "%s: pair count %zu is too small. It must be at least %zu", items_count, file_path, BPE_PRELUDE_SIZE);
        return false;
    }

    for (uint32_t i = 0; i < BPE_PRELUDE_SIZE; ++i) {
        if (items[i].l != i) {
            nob_log(ERROR, "%s: pair %u: Left subtoken is equal to %u instead of itself", file_path, i, items[i].l);
            return false;
        }
        if (items[i].r != 0) {
            nob_log(ERROR, "%s: pair %u: Right subtoken is equal to %u instead of 0", file_path, i, items[i].r);
            return false;
        }
        da_append(pairs, items[i]);
    }

    for (uint32_t i = BPE_PRELUDE_SIZE; i < items_count; ++i) {
        if (items[i].l >= items_count) {
            nob_log(ERROR, "%s: pair %u: Left subtoken is %u >= %zu", file_path, i, items[i].l, items_count);
            return false;
        }
        if (items[i].r >= items_count) {
            nob_log(ERROR, "%s: pair %u: Right subtoken is %u >= %zu", file_path, i, items[i].r, items_count);
            return false;
        }
        da_append(pairs, items[i]);
    }

    return true;
}

#define NOB_IMPLEMENTATION
#include "nob.h"
