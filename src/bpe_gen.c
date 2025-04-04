// Generate random text based on the BPE table created by txt2bpe.
#include <time.h>
#include "bpe.h"
#define FLAG_IMPLEMENTATION
#include "flag.h"

void usage(void) {
    fprintf(stderr, "Usage: %s [OPTIONS]\n", flag_program_name());
    fprintf(stderr, "OPTIONS:\n");
    flag_print_options(stderr);
}

int main(int argc, char **argv)
{
    srand(time(0));

    char **bpe_path = flag_str("bpe", NULL, "Path to BPE file (MANDATORY)");
    bool *help      = flag_bool("help", false, "Print this help message");
    uint64_t *limit = flag_uint64("limit", 100, "Max amount of tokens to generate");
    char **delim    = flag_str("delim", "", "Token delimiter");
    uint64_t *seed  = flag_uint64("seed", 0, "Random seed (0 to use current time)");

    if (!flag_parse(argc, argv)) {
        usage();
        flag_print_error(stderr);
        return 1;
    }

    if (*bpe_path == NULL) {
        usage();
        fprintf(stderr, "ERROR: no -%s is provided\n", flag_name(bpe_path));
        return 1;
    }

    if (*help) {
        usage();
        return 0;
    }

    if (*seed != 0) {
        srand(*seed);
    }

    Pairs pairs = {0};
    String_Builder sb = {0};
    Pairs next = {0};

    size_t version = 0;
    if (!load_pairs(*bpe_path, &pairs, &sb, &version)) return 1;
    printf("INFO: loaded %s BPE version %zu\n", *bpe_path, version);

    uint32_t token = rand()%pairs.count;

    for (uint64_t i = 0; i < *limit; ++i) {
        sb.count = 0;
        render_token(pairs, token, &sb);
        sb_append_null(&sb);
        printf("%s%s", sb.items, *delim);

        for (;;) {
            next.count = 0;
            for (size_t i = 0; i < pairs.count; ++i) {
                if (pairs.items[i].l == token) {
                    da_append(&next, pairs.items[i]);
                }
            }
            if (next.count > 0) break;
            if (token < BPE_PRELUDE_SIZE) break;
            token = pairs.items[token].r;
        }

        if (next.count == 0) break;

        uint64_t sum = 0;
        for (size_t j = 0; j < next.count; ++j) {
            sum += next.items[j].freq;
        }

        uint64_t roll = (uint64_t)rand()*sum/RAND_MAX;
        uint64_t curr = 0;

        // TODO: make sure there is no off-by-one here
        bool found = false;
        for (size_t j = 0; j < next.count; ++j) {
            if (curr + next.items[j].freq > roll) {
                token = next.items[j].r;
                found = true;
                break;
            }
            curr += next.items[j].freq;
        }
        assert(found);
    }

    return 0;
}
