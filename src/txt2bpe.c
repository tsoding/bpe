#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include <pthread.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#define FLAG_IMPLEMENTATION
#include "flag.h"

#include "bpe.h"

typedef struct {
    Pair key;
    size_t value;
} Freq;

int compare_freqs(const void *a, const void *b)
{
    const Freq *af = a;
    const Freq *bf = b;
    return (int)bf->value - (int)af->value;
}

typedef struct {
    Freq *items;
    size_t count;
    size_t capacity;
} Freqs;

#define swap(Type, x, y) \
    do { \
        Type t = (x); \
        (x) = (y); \
        (y) = t; \
    } while(0)

void usage(void)
{
    fprintf(stderr, "Usage: %s [OPTIONS] [--] <input.txt> <output/>\n", flag_program_name());
    fprintf(stderr, "OPTIONS:\n");
    flag_print_options(stderr);
}

void report_progress(size_t iteration, Tokens tokens_in, Pairs pairs)
{
    printf("INFO: iteration %zu\n", iteration);
    printf("    Text tokens count: %zu\n", tokens_in.count);
    printf("    BPE table size: %zu\n", pairs.count);
}

double get_secs(void)
{
    struct timespec tp = {0};
    int ret = clock_gettime(CLOCK_MONOTONIC, &tp);
    assert(ret == 0);
    return (double)tp.tv_sec + (double)tp.tv_nsec*1e-9;
}

double begin_secs;
#if 0
    #define PROFILE_BEGIN() begin_secs = get_secs();
    #define PROFILE_END(label) printf("%s: %lfsecs\n", (label), get_secs() - begin_secs);
#else
    #define PROFILE_BEGIN(...)
    #define PROFILE_END(...)
#endif

#define ENABLE_THREADS
#define THREAD_COUNT 16
Tokens tokens_in = {0};

Freq *freqs[THREAD_COUNT] = {0};
pthread_t threads[THREAD_COUNT] = {0};
pthread_barrier_t collect_freqs_start = {0};
pthread_barrier_t collect_freqs_stop = {0};

void *freq_collector_thread(void *arg)
{
    size_t id = (size_t)(arg);

    while (true) {
        pthread_barrier_wait(&collect_freqs_start);

        hmfree(freqs[id]);

        size_t chunk_size = tokens_in.count/THREAD_COUNT;
        size_t begin      = chunk_size*id;
        size_t end        = chunk_size*(id + 1);
        if (id + 1 >= THREAD_COUNT) end += tokens_in.count%THREAD_COUNT;

        for (size_t i = begin; i < end; ++i) {
            if (i + 1 >= tokens_in.count) break;
            Pair pair = {
                .l = tokens_in.items[i],
                .r = tokens_in.items[i + 1]
            };
            ptrdiff_t place = hmgeti(freqs[id], pair);
            if (place < 0) hmput(freqs[id], pair, 1);
            else freqs[id][place].value += 1;
        }

        pthread_barrier_wait(&collect_freqs_stop);
    }
    UNREACHABLE("freq_collector_thread");
}

bool dump_state(size_t iteration, const char *output_dir_path, Pairs pairs, Tokens tokens)
{
    const char *output_file_path = temp_sprintf("%s/%d.bpe", output_dir_path, iteration);
    if (!dump_pairs(output_file_path, pairs)) return false;
    printf("INFO: generated %s\n", output_file_path);

    output_file_path = temp_sprintf("%s/%d.tkn", output_dir_path, iteration);
    if (!dump_tokens(output_file_path, tokens)) return false;
    printf("INFO: generated %s\n", output_file_path);

    return true;
}

int main(int argc, char **argv)
{
    uint64_t *report_freq = flag_uint64("report-freq", 10, "Per how many iterations report the progress");
    uint64_t *dump_freq = flag_uint64("dump-freq", 10, "Per how many iterations we dump the state of the process");
    uint64_t *term_freq = flag_uint64("term-freq", 1, "Termination pair frequency");
    bool *help = flag_bool("help", false, "Print this help");
    char **input_file = flag_str("input-file", NULL, "Input text file (MANDATORY)");
    char **output_dir = flag_str("output-dir", NULL, "Output directory (MANDATORY)");

    if (!flag_parse(argc, argv)) {
        usage();
        flag_print_error(stderr);
        return 1;
    }

    if (*help) {
        usage();
        return 0;
    }

    if (*input_file == NULL) {
        usage();
        fprintf(stderr, "ERROR: no %s is provided\n", flag_name(input_file));
        return 1;
    }
    const char *input_file_path = *input_file;

    if (*output_dir == NULL) {
        usage();
        fprintf(stderr, "ERROR: no %s is provided\n", flag_name(output_dir));
    }
    const char *output_dir_path = *output_dir;

    if (!mkdir_if_not_exists(output_dir_path)) return 1;

    String_Builder sb = {0};
    Freq *merged_freq = NULL;
    Pairs pairs = {0};

    Tokens tokens_out = {0};

    if (!read_entire_file(input_file_path, &sb)) return 1;

    // 0   => { .l = 0, .r = ??? }
    // 1   => { .l = 1, .r = ??? }
    // ...
    // 69  => { .l = 69, .r = ??? }
    // ...
    // 255 => { .l = 255, .r = ??? }
    for (uint32_t i = 0; i < BPE_PRELUDE_SIZE; ++i) {
        da_append(&pairs, ((Pair) {.l = i}));
    }

    for (size_t i = 0; i < sb.count; ++i) {
        da_append(&tokens_in, (uint8_t)sb.items[i]);
    }

#ifdef ENABLE_THREADS
    int ret;

    ret = pthread_barrier_init(&collect_freqs_start, NULL, THREAD_COUNT + 1);
    if (ret != 0) {
        fprintf(stderr, "ERROR: could not initialize collect_freqs_start: %s\n", strerror(ret));
        return 1;
    }

    ret = pthread_barrier_init(&collect_freqs_stop, NULL, THREAD_COUNT + 1);
    if (ret != 0) {
        fprintf(stderr, "ERROR: could not initialize collect_freqs_stop: %s\n", strerror(ret));
        return 1;
    }

    for (size_t id = 0; id < THREAD_COUNT; ++id) {
        ret = pthread_create(&threads[id], NULL, freq_collector_thread, (void*)id);
        if (ret != 0) {
            fprintf(stderr, "ERROR: could not create thread: %s\n", strerror(ret));
            return 1;
        }
    }
#endif // ENABLE_THREADS


    size_t iteration = 0;
    for (;; ++iteration) {
        if (iteration%(*report_freq) == 0) report_progress(iteration, tokens_in, pairs);
        if (iteration%(*dump_freq)   == 0) if (!dump_state(iteration, output_dir_path, pairs, tokens_in)) return 1;

        PROFILE_BEGIN();
            hmfree(merged_freq);
#ifndef ENABLE_THREADS
            for (size_t i = 0; i < tokens_in.count - 1; ++i) {
                Pair pair = {
                    .l = tokens_in.items[i],
                    .r = tokens_in.items[i + 1]
                };
                ptrdiff_t place = hmgeti(merged_freq, pair);
                if (place < 0) hmput(merged_freq, pair, 1);
                else merged_freq[place].value += 1;
            }
#else
            pthread_barrier_wait(&collect_freqs_start);
            pthread_barrier_wait(&collect_freqs_stop);

            for (size_t id = 0; id < THREAD_COUNT; ++id) {
                size_t n = hmlen(freqs[id]);
                for (size_t i = 0; i < n; ++i) {
                    Pair key = freqs[id][i].key;
                    ptrdiff_t place = hmgeti(merged_freq, key);
                    if (place < 0) hmputs(merged_freq, freqs[id][i]);
                    else merged_freq[place].value += freqs[id][i].value;
                }
            }
#endif
        PROFILE_END("Collecting stats");

        PROFILE_BEGIN();
            ptrdiff_t max_index = 0;
            for (ptrdiff_t i = 1; i < hmlen(merged_freq); ++i) {
                if (merged_freq[i].value > merged_freq[max_index].value) {
                    max_index = i;
                }
            }
        PROFILE_END("Finding most frequent pairs");

        if (merged_freq[max_index].value <= (*term_freq)) break; // compression is done

        da_append(&pairs, merged_freq[max_index].key);

        PROFILE_BEGIN();
            tokens_out.count = 0;
            for (size_t i = 0; i < tokens_in.count; ) {
                if (i + 1 >= tokens_in.count) {
                    da_append(&tokens_out, tokens_in.items[i]);
                    i += 1;
                } else {
                    Pair pair = {.l = tokens_in.items[i], .r = tokens_in.items[i + 1]};
                    if (memcmp(&pair, &merged_freq[max_index].key, sizeof(pair)) == 0) {
                        da_append(&tokens_out, pairs.count - 1);
                        i += 2;
                    } else {
                        da_append(&tokens_out, tokens_in.items[i]);
                        i += 1;
                    }
                }
            }
        PROFILE_END("Replacing the frequent pair");

        swap(Tokens, tokens_in, tokens_out);
    }
    report_progress(iteration, tokens_in, pairs);
    if (!dump_state(iteration, output_dir_path, pairs, tokens_in)) return 1;

    return 0;
}
