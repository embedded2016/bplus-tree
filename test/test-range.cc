#include "test.h"

void range_cb(void* matched, const bp_key_t* key, const bp_value_t* value) {
  /* add '\0' to values */
  (*(int*) matched)++;
}

TEST_START("range get test", "range")
  /* write some stuff */
  const int n = 250;
  int i = 0;
  char key[100];
  char value[100];
  int matched;

  sprintf(key, "key: x");
  sprintf(value, "val: x");
  for (i = 1; i < n; i++) {
    key[5] = i;
    value[5] = i;
    assert(bp_sets(&db, key, value) == BP_OK);
  }

  matched = 0;

  /* try getting 1 key-value */
  bp_get_ranges(&db, "key: \x15", "key: \x15", range_cb, &matched);

  assert(matched == 1);

  matched = 0;

  /* try getting < 32 key-value (in one leaf-page) */
  bp_get_ranges(&db, "key: \x05", "key: \x1f", range_cb, &matched);

  assert(matched == (0x1f - 0x05 + 1));

  matched = 0;

  /* try getting > 32 key-value (in multiple leaf-pages) */
  bp_get_ranges(&db, "key: \x12", "key: \x5f", range_cb, &matched);

  assert(matched == (0x5f - 0x12 + 1));

  matched = 0;

  /* try getting all key-values */
  bp_get_ranges(&db, "key: \x01", "key: \xfa", range_cb, &matched);

  assert(matched == 249);
TEST_END("range get test", "range")
