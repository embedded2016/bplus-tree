# B+ tree implementation in C

## Depedency
The implementation depends on Google's [snappy](https://github.com/google/snappy) library for fast compression.

## Build
```bash
make MODE=release
```
optionally, you can execute test suite:
```bash
make check
```

## Usage

```C
#include <stdlib.h>
#include <stdio.h>

#include "bplus.h"

int main(void) {
  bp_db_t db;

  /* Open database */
  bp_open(&db, "/tmp/1.bp");

  /* Set some value */
  bp_sets(&db, "key", "value");

  /* Get some value */
  bp_value_t value;
  bp_gets(&db, "key", &value);
  fprintf(stdout, "%s\n", value.value);
  free(value.value)

  /* Close database */
  bp_close(&db);
}
```

## Benchmarks

One-threaded read/write (in non-empty database):

```
100000 items in db
write : 9865.357599 ops/sec
read : 57383.402903 ops/sec
```

Multi-threaded read (2 cores, 4 threads):

```
100000 items in db
get : 128841.821540 ops/sec
```

Compaction/Removal:

```
500000 items in db
compact: 23.143330s
remove : 16827.957592 ops/sec
```

And bulk insertion ~ `120000 items/sec` .

## Advanced build options

```bash
make MODE=debug # build with enabled assertions
make SNAPPY=0 # build without snappy (no compression will be used)
```

## Licensing
---------
`bplus-tree` is freely redistributable under MIT X License.
Use of this source code is governed by a MIT license that can be found
in the `LICENSE` file.
