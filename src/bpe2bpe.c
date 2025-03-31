#include "bpe.h"
#define FLAG_IMPLEMENTATION
#include "flag.h"

void usage(void)
{
    fprintf(stderr, "Usage: %s [OPTIONS]\n", flag_program_name());
    flag_print_options(stderr);
}

int main(int argc, char **argv)
{
    char **bpe_path = flag_str("bpe", NULL, "Path to the input BPE file (MANDATORY)");
    char **out_path = flag_str("out", NULL, "Path to the output BPE file (MANDATORY)");

    if (!flag_parse(argc, argv)) {
        usage();
        flag_print_error(stderr);
        return 1;
    }

    if (*bpe_path == NULL) {
        usage();
        fprintf(stderr, "ERROR: No -%s was provided\n", flag_name(bpe_path));
        return 1;
    }

    if (*out_path == NULL) {
        usage();
        fprintf(stderr, "ERROR: No -%s was provided\n", flag_name(out_path));
        return 1;
    }

    Pairs pairs = {0};
    String_Builder sb = {0};

    size_t version = 0;
    if (!load_pairs(*bpe_path, &pairs, &sb, &version)) return false;
    printf("INFO: Converting BPE version %zu -> %d\n", version, BPE_VERSION_CURRENT);
    if (!dump_pairs(*out_path, pairs)) return false;

    return 0;
}
