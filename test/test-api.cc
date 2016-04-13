#include "test.h"

int update_cb(void* arg, const bp_value_t* previous, const bp_value_t* curr) {
  char* expected = (char*) arg;
  assert(strcmp(previous->value, expected) == 0);

  return 1;
}

int remove_cb(void* arg, const bp_value_t* value) {
  char* expected = (char*) arg;
  return strcmp(value->value, expected) == 0;
}

TEST_START("API test", "api")

  const int n = 1000;
  char key[100];
  char val[100];
  char expected[100];
  int i;

  for (i = 0; i < n; i++) {
    sprintf(key, "some key %d", i);
    sprintf(val, "some long long long long long value %d", i);
    assert(bp_sets(&db, key, val) == BP_OK);
  }

  assert(bp_compact(&db) == BP_OK);

  for (i = 0; i < n; i++) {
    sprintf(key, "some key %d", i);
    sprintf(val, "some updated long long long long long value %d", i);
    sprintf(expected, "some long long long long long value %d", i);
    assert(bp_updates(&db, key, val, update_cb, (void*) expected) == BP_OK);
  }

  assert(bp_compact(&db) == BP_OK);

  for (i = 0; i < n; i++) {
    char* result = NULL;

    sprintf(key, "some key %d", i);
    sprintf(expected, "some updated long long long long long value %d", i);

    assert(bp_gets(&db, key, &result) == BP_OK);
    assert(strcmp(result, expected) == 0);

    free(result);
  }

  /* overwrite every key */
  for (i = 0; i < n; i++) {
    sprintf(key, "some key %d", i);
    sprintf(val, "some another value %d", i);
    assert(bp_sets(&db, key, val) == BP_OK);
  }

  for (i = 0; i < n; i++) {
    bp_key_t kkey;
    bp_value_t result;
    bp_value_t previous;

    sprintf(key, "some key %d", i);

    kkey.length = strlen(key) + 1;
    kkey.value = key;

    /* new values should be in place */
    sprintf(expected, "some another value %d", i);
    assert(bp_get(&db, &kkey, &result) == BP_OK);
    assert(strcmp(result.value, expected) == 0);

    /* previous values should be available before compaction */
    sprintf(expected, "some updated long long long long long value %d", i);
    assert(bp_get_previous(&db, &result, &previous) == BP_OK);
    assert(strcmp(previous.value, expected) == 0);

    free(result.value);

    /* previous of previous ain't exist */
    assert(bp_get_previous(&db, &previous, &result) == BP_ENOTFOUND);

    free(previous.value);
  }

  assert(bp_compact(&db) == BP_OK);

  for (i = 0; i < n; i++) {
    bp_key_t kkey;
    bp_value_t result;
    bp_value_t previous;

    sprintf(key, "some key %d", i);

    kkey.length = strlen(key) + 1;
    kkey.value = key;

    /* new values should be in place */
    sprintf(expected, "some another value %d", i);
    assert(bp_get(&db, &kkey, &result) == BP_OK);
    assert(strcmp(result.value, expected) == 0);

    /* previous should be not available after compaction */
    assert(bp_get_previous(&db, &result, &previous) == BP_ENOTFOUND);

    free(result.value);
  }

  for (i = 0; i < n; i++) {
    sprintf(key, "some key %d", i);
    sprintf(expected, "some another value %d", i);
    assert(bp_removevs(&db, key, remove_cb, (void*) expected) == BP_OK);
  }

  assert(bp_compact(&db) == BP_OK);

TEST_END("API test", "api")
