#include "test.h"

const int num = 100000;
const int rnum = 4;
static char* keys[num];

void* reader_thread(void* db_) {
  bp_db_t* db = (bp_db_t*) db_;

  for (int i = 0; i < num; i++) {
    char* value;
    bp_gets(db, keys[i], &value);
    free(value);
  }

  return NULL;
}

TEST_START("multi-threaded get benchmark", "mt-get-bench")
  int i;
  pthread_t readers[rnum];

  for (i = 0; i < num; i++) {
    keys[i] = (char*) malloc(20);
    sprintf(keys[i], "%0*d", 20, i);
  }

  bp_bulk_sets(&db,
               num,
               (const char**) keys,
               (const char**) keys);

  BENCH_START(get, rnum * num)
  for (i = 0; i < rnum; i++) {
    pthread_create(&readers[i], NULL, reader_thread, (void*) &db);
  }

  for (i = 0; i < rnum; i++) {
    pthread_join(readers[i], NULL);
  }
  BENCH_END(get, rnum * num)
TEST_END("multi-threaded get benchmark", "mt-get-bench")
