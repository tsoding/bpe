#include <stdio.h>

#include "bpe.h"

void usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s <input.bpe>\n", program_name);
}

// TODO: employ memoization so to not rerender the same tokens over and over again
void render_token(Pairs pairs, uint32_t token, String_Builder *sb)
{
    if (token == pairs.items[token].l) {
        da_append(sb, (char)token);
        return;
    }

    render_token(pairs, pairs.items[token].l, sb);
    render_token(pairs, pairs.items[token].r, sb);
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

    Pairs pairs = {0};
    String_Builder sb = {0};

    if (!load_pairs(input_file_path, &pairs, &sb)) return 1;

    for (uint32_t token = 1; token < pairs.count; ++token) {
        printf("%u => \"", token);
        sb.count = 0;
        render_token(pairs, token, &sb);
        for (size_t i = 0; i < sb.count; ++i) {
            if (sb.items[i] == '"') {
                printf("\\\"");
            } else if (sb.items[i] == '\\') {
                printf("\\\\");
            } else if (isprint(sb.items[i])) {
                printf("%c", sb.items[i]);
            } else {
                printf("\\x%02X", (uint8_t)sb.items[i]);
            }
        }
        printf("\"\n");
    }

    return 0;
}
