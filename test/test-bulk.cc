#include "test.h"

int update_cb(void* arg, const bp_value_t* previous, const bp_value_t* curr) {
  int i = (unsigned char) previous->value[5];
  return i % 2 == 0 ? 1 : 0;
}

TEST_START("bulk set test", "bulk-set")
  /* write some stuff */
  const int n = 128;
  int i = 0;
  char key[100];
  char* keys[n];

  sprintf(key, "key: x");
  for (i = 0; i < n; i++) {
    key[5] = i << 1;
    assert(bp_sets(&db, key, key) == BP_OK);
  }

  for (i = 0; i < n; i++) {
    keys[i] = (char*) malloc(100);
    assert(keys[i] != NULL);

    sprintf(keys[i], "key: x");
    keys[i][5] = (i << 1) + 1;
  }

  assert(bp_bulk_sets(&db, n, (const char**) keys, (const char**) keys) ==
         BP_OK);

  /* just for sanity_check */
  assert(bp_bulk_updates(&db,
                         n,
                         (const char**) keys,
                         (const char**) keys,
                         update_cb,
                         NULL) == BP_OK);

  for (i = 0; i < n; i++) {
    free(keys[i]);
  }

  for (i = 0; i < n; i++) {
    char* value;

    key[5] = i << 1;
    assert(bp_gets(&db, key, &value) == BP_OK);
    assert(strcmp(key, value) == 0);
    free(value);

    key[5] = (i << 1) + 1;
    assert(bp_gets(&db, key, &value) == BP_OK);
    assert(strcmp(key, value) == 0);
    free(value);
  }

TEST_END("range get test", "range")
