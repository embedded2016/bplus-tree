#include "test.h"

TEST_START("database corruption test", "db-corrupt")
  const int n = 1024;
  char key[100];
  char val[100];
  char expected[100];
  int i, fd, filesize, matched;

  for (i = 0; i < n; i++) {
    sprintf(key, "some key %d", i);
    sprintf(val, "some long long long long long value %d", i);
    assert(bp_sets(&db, key, val) == BP_OK);
  }

  assert(bp_close(&db) == BP_OK);

  /* corrupt file by zeroing last 2k bytes of it */
  fd = open(__db_file, O_RDWR, S_IWUSR | S_IRUSR);
  assert(fd != -1);

  char buff[13589];
  memset((void*) buff, 0xff, sizeof(buff));

  filesize = lseek(fd, 0, SEEK_END);
  assert(filesize != -1);
  assert(pwrite(fd,
                (void*) buff,
                sizeof(buff),
                filesize - sizeof(buff)) == sizeof(buff));
  assert(close(fd) == 0);

  assert(bp_open(&db, __db_file) == BP_OK);

  matched = 0;
  for (i = 0; i < n; i++) {
    sprintf(key, "some key %d", i);
    sprintf(expected, "some long long long long long value %d", i);
    char* actual;
    if (bp_gets(&db, key, &actual) != BP_OK) continue;

    assert(strcmp(expected, actual) == 0);
    free(actual);

    matched++;
  }
  assert(matched > 3 * (n >> 2));
TEST_END("database corruption test", "db-corrupt")
