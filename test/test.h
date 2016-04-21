#include <bplus.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <assert.h>

#define TRY_REMOVE(db_file)                                     \
    if (access("/tmp/" db_file ".bp", F_OK) == 0) {             \
        assert(unlink("/tmp/" db_file ".bp") == 0);             \
    }                                                           \
    if (access("/tmp/" db_file ".bp.compact", F_OK) == 0) {     \
        assert(unlink("/tmp/" db_file ".bp.compact") == 0);     \
    }

#define TEST_START(name, db_file)                               \
    int main(void) {                                            \
        fprintf(stdout, "-- %s --\n", name);                    \
        TRY_REMOVE(db_file)                                     \
        bp_db_t db;                                             \
        assert(bp_open(&db, "/tmp/" db_file ".bp") == 0);       \
        const char *__db_file = "/tmp/" db_file ".bp";

#define TEST_END(name, db_file)                                 \
      assert(bp_close(&db) == 0);                               \
      /* cleanup */                                             \
      TRY_REMOVE(db_file)                                       \
      fclose(stdout);                                           \
      return 0;\
    }

#define BENCH_START(name, num)                                 \
    timeval __bench_##name##_start;                            \
    gettimeofday(&__bench_##name##_start, NULL);

#define BENCH_END(name, num)                                   \
    timeval __bench_##name##_end;                              \
    gettimeofday(&__bench_##name##_end, NULL);                 \
    double __bench_##name##_total =                            \
        __bench_##name##_end.tv_sec -                          \
        __bench_##name##_start.tv_sec +                        \
        __bench_##name##_end.tv_usec * 1e-6 -                  \
        __bench_##name##_start.tv_usec * 1e-6;                 \
    if ((num) != 0) {                                          \
        fprintf(stdout, #name " : %f ops/sec\n",               \
                (num) / __bench_##name##_total);               \
    } else {                                                   \
        fprintf(stdout, #name " : %fs\n",                      \
                __bench_##name##_total);                       \
    }
