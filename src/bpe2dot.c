#include <stdio.h>

#include "bpe.h"

void render_dot(Pairs pairs, String_Builder *sb)
{
    sb_append_cstr(sb, "digraph Pairs {\n");
    for (uint32_t token = 0; token < pairs.count; ++token) {
        if (token != pairs.items[token].l) {
            sb_append_cstr(sb, temp_sprintf("  %u -> %u\n", token, pairs.items[token].l));
            sb_append_cstr(sb, temp_sprintf("  %u -> %u\n", token, pairs.items[token].r));
        }
    }
    sb_append_cstr(sb, "}\n");
}

void usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s <input.bpe> <output.dot>\n", program_name);
}

int main(int argc, char **argv)
{
    const char *program_name = shift(argv, argc);

    if (argc <= 0) {
        usage(program_name);
        fprintf(stderr, "ERROR: no input is provided\n");
        return 1;
    }
    const char *input_file_path = shift(argv, argc);

    if (argc <= 0) {
        usage(program_name);
        fprintf(stderr, "ERROR: no output is provided\n");
        return 1;
    }
    const char *output_file_path = shift(argv, argc);

    Pairs pairs = {0};
    String_Builder sb = {0};

    if (!load_pairs(input_file_path, &pairs, &sb)) return 1;
    sb.count = 0;
    render_dot(pairs, &sb);
    if (!write_entire_file(output_file_path, sb.items, sb.count)) return 1;
    printf("INFO: generated %s\n", output_file_path);

    return 0;
}
