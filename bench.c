/*
 * bench - a light benchmark for low-RAM / low-CPU devices.
 *
 * Runs for ~5 minutes by default, exercises CPU and memory gently, and
 * prints a single score in the range 10-100 (higher is better).
 *
 * Build:
 *   make
 * Run:
 *   ./bench
 *
 * Options:
 *   -d SECONDS   set total run time (default 300)
 *   -h           show this help
 *
 * This is free software released under the BSD 2-Clause License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>

#define DURATION_DEF (5 * 60)   /* total wall-clock target: 5 minutes */
#define SCORE_MIN    10
#define SCORE_MAX    100
#define CHUNK_MB     2           /* memory chunk per iteration (small) */
#define REF_CPU_OPS  50000.0     /* ops/s/worker -> score contribution ~70 */
#define REF_MEM_MB   50.0        /* MB/s/worker  -> score contribution ~30 */

static int g_duration = DURATION_DEF;

typedef struct {
    double duration;     /* seconds this worker should run */
    long long ops;       /* output: integer ops performed */
    long long bytes;     /* output: bytes touched */
} worker_arg_t;

static void *cpu_worker(void *p)
{
    worker_arg_t *a = (worker_arg_t *)p;
    time_t end = time(NULL) + (time_t)a->duration;
    long long count = 0;
    unsigned long x = 1;

    while (time(NULL) < end) {
        x = (x * 1103515245UL + 12345UL) & 0x7FFFFFFFUL;
        count++;
    }
    a->ops = count;
    a->bytes = 0;
    return NULL;
}

static void *mem_worker(void *p)
{
    worker_arg_t *a = (worker_arg_t *)p;
    time_t end = time(NULL) + (time_t)a->duration;
    long long touched = 0;
    size_t sz = (size_t)CHUNK_MB * 1024 * 1024;

    while (time(NULL) < end) {
        unsigned char *buf = (unsigned char *)malloc(sz);
        if (!buf) break;
        for (size_t i = 0; i < sz; i += 4096)
            buf[i] = (unsigned char)(random() & 0xFF);
        touched += (long long)sz;
        free(buf);
    }
    a->bytes = touched;
    a->ops = 0;
    return NULL;
}

static void usage(const char *prog)
{
    printf("Usage: %s [-d SECONDS] [-h]\n", prog);
    printf("  -d SECONDS   total run time in seconds (default %d)\n", DURATION_DEF);
    printf("  -h           show this help\n");
}

int main(int argc, char **argv)
{
    int opt;
    while ((opt = getopt(argc, argv, "d:h")) != -1) {
        switch (opt) {
        case 'd':
            g_duration = atoi(optarg);
            if (g_duration < 1) g_duration = DURATION_DEF;
            break;
        case 'h':
            usage(argv[0]);
            return 0;
        default:
            usage(argv[0]);
            return 1;
        }
    }

    long cpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (cpus < 1) cpus = 1;
    int workers = cpus / 2 > 0 ? (int)(cpus / 2) : 1;
    double phase = (double)g_duration / 2.0;

    printf("bench - lightweight benchmark for low-spec devices\n");
    printf("Run time: %d s | Detected %ld CPUs, using %d worker(s).\n\n",
           g_duration, cpus, workers);

    pthread_t *tids = (pthread_t *)malloc(sizeof(pthread_t) * (size_t)workers);
    worker_arg_t *args = (worker_arg_t *)malloc(sizeof(worker_arg_t) * (size_t)workers);
    if (!tids || !args) { fprintf(stderr, "out of memory\n"); return 1; }

    /* Phase 1: CPU */
    printf("Phase 1/2: CPU test...\n");
    fflush(stdout);
    time_t cpu_start = time(NULL);
    for (int i = 0; i < workers; i++) {
        args[i].duration = phase;
        args[i].ops = 0;
        args[i].bytes = 0;
        pthread_create(&tids[i], NULL, cpu_worker, &args[i]);
    }
    long long cpu_ops = 0;
    for (int i = 0; i < workers; i++) {
        pthread_join(tids[i], NULL);
        cpu_ops += args[i].ops;
    }
    double cpu_elapsed = (double)(time(NULL) - cpu_start);
    printf("  done in %.1fs, %lld integer ops\n", cpu_elapsed, cpu_ops);

    /* Phase 2: Memory */
    printf("Phase 2/2: Memory test...\n");
    fflush(stdout);
    time_t mem_start = time(NULL);
    for (int i = 0; i < workers; i++) {
        args[i].duration = phase;
        args[i].ops = 0;
        args[i].bytes = 0;
        pthread_create(&tids[i], NULL, mem_worker, &args[i]);
    }
    long long mem_bytes = 0;
    for (int i = 0; i < workers; i++) {
        pthread_join(tids[i], NULL);
        mem_bytes += args[i].bytes;
    }
    double mem_elapsed = (double)(time(NULL) - mem_start);
    printf("  done in %.1fs, %lld bytes touched\n\n", mem_elapsed, mem_bytes);

    double cpu_rate = (double)cpu_ops / (cpu_elapsed > 0 ? cpu_elapsed : 1) / workers;
    double mem_rate = (double)mem_bytes / (mem_elapsed > 0 ? mem_elapsed : 1) / (1024.0 * 1024.0) / workers;

    double cpu_sub = 70.0 * (cpu_rate / REF_CPU_OPS < 1.0 ? cpu_rate / REF_CPU_OPS : 1.0);
    double mem_sub = 30.0 * (mem_rate / REF_MEM_MB  < 1.0 ? mem_rate / REF_MEM_MB  : 1.0);

    double raw = cpu_sub + mem_sub;
    int score = (int)raw;
    if (score < SCORE_MIN) score = SCORE_MIN;
    if (score > SCORE_MAX) score = SCORE_MAX;

    printf("========================================\n");
    printf("CPU rate   : %.0f ops/s/worker\n", cpu_rate);
    printf("Memory rate: %.1f MB/s/worker\n", mem_rate);
    printf("SCORE      : %d / 100  (range %d-%d)\n", score, SCORE_MIN, SCORE_MAX);
    printf("========================================\n");

    free(tids);
    free(args);
    return 0;
}
