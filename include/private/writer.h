#ifndef _PRIVATE_WRITER_H_
#define _PRIVATE_WRITER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BP_WRITER_PRIVATE	\
    int fd;                     \
    char *filename;             \
    uint64_t filesize;          \
    char padding[BP_PADDING];

typedef struct bp__writer_s bp__writer_t;
typedef int (*bp__writer_cb)(bp__writer_t *w, void *data);

enum comp_type {
    kNotCompressed = 0,
    kCompressed = 1
};

int bp__writer_create(bp__writer_t *w, const char *filename);
int bp__writer_destroy(bp__writer_t *w);

int bp__writer_fsync(bp__writer_t *w);

int bp__writer_compact_name(bp__writer_t *w, char **compact_name);
int bp__writer_compact_finalize(bp__writer_t *s, bp__writer_t *t);

int bp__writer_read(bp__writer_t *w,
                    const enum comp_type comp,
                    const uint64_t offset,
                    uint64_t *size,
                    void **data);
int bp__writer_write(bp__writer_t *w,
                     const enum comp_type comp,
                     const void *data,
                     uint64_t *offset,
                     uint64_t *size);

int bp__writer_find(bp__writer_t *w,
                    const enum comp_type comp,
                    const uint64_t size,
                    void *data,
                    bp__writer_cb seek,
                    bp__writer_cb miss);

struct bp__writer_s {
    BP_WRITER_PRIVATE
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _PRIVATE_WRITER_H_ */
