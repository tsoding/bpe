#include "bpe.h"
#define FLAG_IMPLEMENTATION
#include "flag.h"

void usage(void)
{
    fprintf(stderr, "Usage: %s [OPTIONS]\n", flag_program_name());
    fprintf(stderr, "OPTIONS:\n");
    flag_print_options(stderr);
}

int main(int argc, char **argv)
{
    char **bpe_path = flag_str("bpe", NULL, "Path to the .bpe file that contains table of pairs (MANDATORY)");
    char **tkn_path = flag_str("tkn", NULL, "Path to the .tkn file that contains the sequence of tokens (MANDATORY)");
    bool *help = flag_bool("help", false, "Print this help");

    if (!flag_parse(argc, argv)) {
        usage();
        flag_print_error(stderr);
        return 1;
    }

    if (*help) {
        usage();
        return 0;
    }

    if (*bpe_path == NULL) {
        usage();
        fprintf(stderr, "ERROR: no %s is provided\n", flag_name(bpe_path));
        return 1;
    }

    if (*tkn_path == NULL) {
        usage();
        fprintf(stderr, "ERROR: no %s is provided\n", flag_name(tkn_path));
        return 1;
    }

    Tokens tokens = {0};
    Pairs pairs = {0};
    String_Builder sb = {0};
    if (!load_tokens(*tkn_path, &tokens, &sb)) return 1;
    if (!load_pairs(*bpe_path, &pairs, &sb)) return 1;

    for (size_t i = 0; i < tokens.count; ++i) {
        if (tokens.items[i] >= pairs.count) {
            fprintf(stderr, "ERROR: token %zu: value %u is out of bounds (%zu)", i, tokens.items[i], pairs.count - 1);
            return 1;
        }
    }

    printf("OK: Loaded %zu tokens from %s\n", tokens.count, *tkn_path);
    printf("OK: Loaded %zu pairs from %s\n", pairs.count, *bpe_path);

    return 0;
}
