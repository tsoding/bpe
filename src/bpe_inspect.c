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
    String_Builder sb_tmp = {0};

    if (!load_pairs(input_file_path, &pairs, &sb_tmp)) return 1;

    for (uint32_t token = 1; token < pairs.count; ++token) {
        sb.count = 0;

        if (!no_ids) sb_appendf(&sb, "%u => ", token);

        sb_append_cstr(&sb, "\"");
            sb_tmp.count = 0;
            render_token(pairs, token, &sb_tmp);
            c_strlit_escape_bytes(sb_tmp.items, sb_tmp.count, &sb);
        sb_append_cstr(&sb, "\"\n");
        sb_append_null(&sb);

        printf("%s", sb.items);
    }

    return 0;
}
