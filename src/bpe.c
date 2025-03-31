// The code that is shared between all the utilitize of the suite.
#include "bpe.h"

bool dump_tokens(const char *file_path, Tokens tokens)
{
    // TODO: introduce magic and version to the format of the tokens
    return write_entire_file(file_path, tokens.items, tokens.count*sizeof(*tokens.items));
}

bool load_tokens(const char *file_path, Tokens *tokens, String_Builder *tmp_sb)
{
    tmp_sb->count = 0;
    if (!read_entire_file(file_path, tmp_sb)) return false;
    if (tmp_sb->count%sizeof(*tokens->items) != 0) {
        nob_log(ERROR, "%s: file size in bytes (%zu) must be divisible by %zu", file_path, tmp_sb->count, sizeof(*tokens->items));
        return false;
    }
    uint32_t *items = (void*)tmp_sb->items;
    size_t items_count = tmp_sb->count/sizeof(*tokens->items);
    for (size_t i = 0; i < items_count; ++i){
        da_append(tokens, items[i]);
    }
    return true;
}

bool dump_pairs(const char *file_path, Pairs pairs)
{
    bool result = true;
    String_Builder sb = {0};

    sb_append_cstr(&sb, "BPE");
    da_append(&sb, BPE_VERSION_CURRENT);
    sb_append_buf(&sb, pairs.items, pairs.count*sizeof(*pairs.items));
    if (!write_entire_file(file_path, sb.items, sb.count)) return_defer(false);

defer:
    free(sb.items);
    return result;
}

typedef struct {
    uint32_t l, r;
} Pair_V0;

typedef struct {
    Pair_V0 *items;
    size_t count;
    size_t capacity;
} Pairs_V0;

// TODO: code duplication between parse_pairs_v0 and parse_pairs_v1

bool parse_pairs_v0(const char *file_path, Pairs *pairs, String_View data)
{
    bool result = true;
    Pairs_V0 pairs_v0 = {0};

    if (data.count%sizeof(*pairs_v0.items) != 0) {
        nob_log(ERROR, "%s: file size in bytes (%zu) must be divisible by %zu", file_path, data.count, sizeof(*pairs_v0.items));
        return_defer(false);
    }
    Pair_V0 *items = (void*)data.data;
    size_t items_count = data.count/sizeof(*pairs_v0.items);

    if (items_count < BPE_PRELUDE_SIZE) {
        nob_log(ERROR, "%s: pair count %zu is too small. It must be at least %d", file_path, items_count, BPE_PRELUDE_SIZE);
        return_defer(false);
    }

    for (uint32_t i = 0; i < BPE_PRELUDE_SIZE; ++i) {
        if (items[i].l != i) {
            nob_log(ERROR, "%s: pair %u: Left subtoken is equal to %u instead of itself", file_path, i, items[i].l);
            return_defer(false);
        }
        if (items[i].r != 0) {
            nob_log(ERROR, "%s: pair %u: Right subtoken is equal to %u instead of 0", file_path, i, items[i].r);
            return_defer(false);
        }
        da_append(&pairs_v0, items[i]);
    }

    for (uint32_t i = BPE_PRELUDE_SIZE; i < items_count; ++i) {
        if (items[i].l >= items_count) {
            nob_log(ERROR, "%s: pair %u: Left subtoken is %u >= %zu", file_path, i, items[i].l, items_count);
            return_defer(false);
        }
        if (items[i].r >= items_count) {
            nob_log(ERROR, "%s: pair %u: Right subtoken is %u >= %zu", file_path, i, items[i].r, items_count);
            return_defer(false);
        }
        da_append(&pairs_v0, items[i]);
    }

    da_foreach(Pair_V0, pair_v0, &pairs_v0) {
        da_append(pairs, ((Pair) {
            .l = pair_v0->l,
            .r = pair_v0->r,
            .freq = 1,
        }));
    }

defer:
    free(pairs_v0.items);
    return result;
}

bool parse_pairs_v1(const char *file_path, Pairs *pairs, String_View data)
{
    if (data.count%sizeof(*pairs->items) != 0) {
        nob_log(ERROR, "%s: file size in bytes (%zu) must be divisible by %zu", file_path, data.count, sizeof(*pairs->items));
        return false;
    }
    Pair *items = (void*)data.data;
    size_t items_count = data.count/sizeof(*pairs->items);

    if (items_count < BPE_PRELUDE_SIZE) {
        nob_log(ERROR, "%s: pair count %zu is too small. It must be at least %d", file_path, items_count, BPE_PRELUDE_SIZE);
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

bool load_pairs(const char *file_path, Pairs *pairs, String_Builder *tmp_sb, size_t *version)
{
    tmp_sb->count = 0;
    if (!read_entire_file(file_path, tmp_sb)) return false;

    String_View data = sb_to_sv(*tmp_sb);

    size_t magic_count = 4;
    String_View magic_prefix = sv_from_cstr("BPE");
    if (sv_starts_with(data, magic_prefix)) {
        if (data.count < magic_count) {
            nob_log(ERROR, "%s: missing BPE version", file_path);
            return false;
        }
        *version = data.data[3];

        data.data  += magic_count;
        data.count -= magic_count;

        switch (*version) {
        case 0: return parse_pairs_v0(file_path, pairs, data);
        case 1: return parse_pairs_v1(file_path, pairs, data);
        default:
            nob_log(ERROR, "%s: unsupported BPE version %zu", file_path, *version);
            return false;
        }
    }

    String_View magic_prefix_v0 = sv_from_parts("\0\0\0\0", magic_count);
    if (sv_starts_with(data, magic_prefix_v0)) {
        *version = 0;
        return parse_pairs_v0(file_path, pairs, data);
    }

    nob_log(ERROR, "%s: invalid magic", file_path);
    return false;
}

void render_token(Pairs pairs, uint32_t token, String_Builder *sb)
{
    assert(token < pairs.count);

    // TODO: employ memoization so to not rerender the same tokens over and over again
    if (token == pairs.items[token].l) {
        da_append(sb, (char)token);
        return;
    }

    render_token(pairs, pairs.items[token].l, sb);
    render_token(pairs, pairs.items[token].r, sb);
}

void c_strlit_escape_bytes(const char *bytes, size_t bytes_size, String_Builder *sb_out)
{
    for (size_t i = 0; i < bytes_size; ++i) {
        if (bytes[i] == '"') {
            sb_append_cstr(sb_out, "\\\"");
        } else if (bytes[i] == '\\') {
            sb_append_cstr(sb_out, "\\\\");
        } else if (isprint(bytes[i])) {
            da_append(sb_out, bytes[i]);
        } else {
            sb_appendf(sb_out, "\\x%02X", (uint8_t)bytes[i]);
        }
    }
}

#define NOB_IMPLEMENTATION
#include "nob.h"
