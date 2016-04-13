#include "test.h"

TEST_START("basic benchmark", "basic-bench")

  const int num = 500000;
  const int value_len = 1000;
  const int delta = 20000;
  int i, start;

  char keys[num][10];
  char value[value_len];

  /* init keys */
  for (int i = 0; i < num; i++) {
    sprintf(keys[i], "%d", i);
  }
  /* init value */
  for (int i = 0; i < value_len; i++) {
    value[i] = 'a' + ((i << 3) | i) % 52;
  }
  value[value_len - 1] = 0;

  for (start = 0; start < num; start += delta) {
    fprintf(stdout, "%d items in db\n", start);

    BENCH_START(write, delta)
    for (i = start; i < start + delta; i++) {
      bp_sets(&db, keys[i], value);
    }
    BENCH_END(write, delta)

    BENCH_START(read, start + delta)
    for (i = 0; i < start + delta; i++) {
      char* value1;
      bp_gets(&db, keys[i], &value1);
      free(value1);
    }
    BENCH_END(read, start + delta)
  }

  BENCH_START(compact, 0)
  bp_compact(&db);
  BENCH_END(compact, 0)

  BENCH_START(read_after_compact, num)
  for (i = 0; i < num; i++) {
    char* value;
    bp_gets(&db, keys[i], &value);
    free(value);
  }
  BENCH_END(read_after_compact, num)

  BENCH_START(read_after_compact_with_os_cache, num)
  for (i = 0; i < num; i++) {
    char* value;
    bp_gets(&db, keys[i], &value);
    free(value);
  }
  BENCH_END(read_after_compact_with_os_cache, num)

  BENCH_START(remove, num)
  for (i = 0; i < num; i++) {
    bp_removes(&db, keys[i]);
  }
  BENCH_END(remove, num)

TEST_END("basic benchmark", "basic-bench")
