#include "test.h"

TEST_START("bulk set benchmark", "bulk-bench")

  const int num = 500000;
  const int delta = 20000;
  int i, start;

  char* keys[num];

  /* init keys */

  for (start = 0; start < num; start += delta) {
    for (i = start; i < start + delta; i++) {
      keys[i] = (char*) malloc(20);
      sprintf(keys[i], "%0*d", 20, i);
    }

    fprintf(stdout, "%d items in db\n", start);

    BENCH_START(bulk, delta)
    bp_bulk_sets(&db,
                 delta,
                 (const char**) keys + start,
                 (const char**) keys + start);
    BENCH_END(bulk, delta)

    for (i = start; i < start + delta; i++) {
      free(keys[i]);
    }
  }

  /* ensure that results are correct */
  for (i = 0; i < num; i++) {
    char* key;
    char* value;
    key = (char*) malloc(20);
    sprintf(key, "%0*d", 20, i);

    assert(bp_gets(&db, key, &value) == BP_OK);
    assert(strcmp(value, key) == 0);

    free(key);
    free(value);
  }

TEST_END("bulk set benchmark", "bulk-bench")
