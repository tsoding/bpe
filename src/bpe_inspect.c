#include <stdio.h>

#include "bpe.h"

void usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s <input.bpe>\n", program_name);
}

int main(int argc, char **argv)
{
    const char *program_name = shift(argv, argc);
    const char *input_file_path = NULL;
    bool no_ids = false;

    while (argc > 0) {
        const char *arg = shift(argv, argc);
        if (strcmp(arg, "--no-ids") == 0) {
            no_ids = true;
        } else {
            if (input_file_path != NULL) {
                fprintf(stderr, "ERROR: %s supports inspecting only single file\n", program_name);
                return 1;
            }
            input_file_path = arg;
        }
    }

    if (input_file_path == NULL) {
        usage(program_name);
        fprintf(stderr, "ERROR: no input is provided\n");
        return 1;
    }

    Pairs pairs = {0};
    String_Builder sb = {0};

    if (!load_pairs(input_file_path, &pairs, &sb)) return 1;

    for (uint32_t token = 1; token < pairs.count; ++token) {
        if (!no_ids) printf("%u => ", token);
        printf("\"");
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
