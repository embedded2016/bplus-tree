#include "bplus.h"
#include "private/writer.h"
#include "private/compressor.h"

#include <fcntl.h> /* open */
#include <unistd.h> /* close, write, read */
#include <sys/stat.h> /* S_IWUSR, S_IRUSR */
#include <stdlib.h> /* malloc, free */
#include <stdio.h> /* sprintf */
#include <string.h> /* memset */
#include <errno.h> /* errno */

int bp__writer_create(bp__writer_t *w, const char *filename)
{
    off_t filesize;
    size_t filename_length;

    /* copy filename + '\0' char */
    filename_length = strlen(filename) + 1;
    w->filename = malloc(filename_length);
    if (w->filename == NULL) return BP_EALLOC;
    memcpy(w->filename, filename, filename_length);

    w->fd = open(filename,
                 O_RDWR | O_APPEND | O_CREAT,
                 S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    if (w->fd == -1) goto error;

    /* Determine filesize */
    filesize = lseek(w->fd, 0, SEEK_END);
    if (filesize == -1) goto error;

    w->filesize = (uint64_t) filesize;

    /* Nullify padding to shut up valgrind */
    memset(&w->padding, 0, sizeof(w->padding));

    return BP_OK;

error:
    free(w->filename);
    return BP_EFILE;
}

int bp__writer_destroy(bp__writer_t *w)
{
    free(w->filename);
    w->filename = NULL;
    if (close(w->fd)) return BP_EFILE;
    return BP_OK;
}

int bp__writer_fsync(bp__writer_t *w)
{
#ifdef F_FULLFSYNC
    /* OSX support */
    return fcntl(w->fd, F_FULLFSYNC);
#else
    return fdatasync(w->fd) == 0 ? BP_OK : BP_EFILEFLUSH;
#endif
}

int bp__writer_compact_name(bp__writer_t *w, char **compact_name)
{
    char *filename = malloc(strlen(w->filename) + sizeof(".compact") + 1);
    if (filename == NULL) return BP_EALLOC;

    sprintf(filename, "%s.compact", w->filename);
    if (access(filename, F_OK) != -1 || errno != ENOENT) {
        free(filename);
        return BP_ECOMPACT_EXISTS;
    }

    *compact_name = filename;
    return BP_OK;
}

int bp__writer_compact_finalize(bp__writer_t *s, bp__writer_t *t)
{
    int ret;
    char *name, *compacted_name;

    /* save filename and prevent freeing it */
    name = s->filename;
    compacted_name = t->filename;
    s->filename = NULL;
    t->filename = NULL;

    /* close both trees */
    bp__destroy((bp_db_t *) s);
    ret = bp_close((bp_db_t *) t);
    if (ret != BP_OK) goto fatal;

    if (rename(compacted_name, name) != 0) return BP_EFILERENAME;

    /* reopen source tree */
    ret = bp__writer_create(s, name);
    if (ret != BP_OK) goto fatal;
    ret = bp__init((bp_db_t *) s);

fatal:
    free(compacted_name);
    free(name);

    return ret;
}

int bp__writer_read(bp__writer_t *w,
                    const enum comp_type comp,
                    const uint64_t offset,
                    uint64_t *size,
                    void **data)
{
    ssize_t bytes_read;
    char *cdata;

    if (w->filesize < offset + *size) return BP_EFILEREAD_OOB;

    /* Ignore empty reads */
    if (*size == 0) {
        *data = NULL;
        return BP_OK;
    }

    cdata = malloc(*size);
    if (cdata == NULL) return BP_EALLOC;

    bytes_read = pread(w->fd, cdata, (size_t) *size, (off_t) offset);
    if ((uint64_t) bytes_read != *size) {
        free(cdata);
        return BP_EFILEREAD;
    }

    /* no compression for head */
    if (comp == kNotCompressed) {
        *data = cdata;
    } else {
        int ret = 0;

        char *uncompressed = NULL;
        size_t usize;

        if (bp__uncompressed_length(cdata, *size, &usize) != BP_OK) {
            ret = BP_EDECOMP;
        } else {
            uncompressed = malloc(usize);
            if (uncompressed == NULL) {
                ret = BP_EALLOC;
            } else if (bp__uncompress(cdata, *size, uncompressed, &usize) != BP_OK) {
                ret = BP_EDECOMP;
            } else {
                *data = uncompressed;
                *size = usize;
            }
        }

        free(cdata);

        if (ret != BP_OK) {
            free(uncompressed);
            return ret;
        }
    }

    return BP_OK;
}

int bp__writer_write(bp__writer_t *w,
                     const enum comp_type comp,
                     const void *data,
                     uint64_t *offset,
                     uint64_t *size)
{
    ssize_t written;
    uint32_t padding = sizeof(w->padding) - (w->filesize % sizeof(w->padding));

    /* Write padding */
    if (padding != sizeof(w->padding)) {
        written = write(w->fd, &w->padding, (size_t) padding);
        if ((uint32_t) written != padding) return BP_EFILEWRITE;
        w->filesize += padding;
    }

    /* Ignore empty writes */
    if (size == NULL || *size == 0) {
        if (offset != NULL) *offset = w->filesize;
        return BP_OK;
    }

    /* head shouldn't be compressed */
    if (comp == kNotCompressed) {
        written = write(w->fd, data, *size);
    } else {
        int ret;
        size_t max_csize = bp__max_compressed_size(*size);
        size_t result_size;
        char *compressed = malloc(max_csize);
        if (compressed == NULL) return BP_EALLOC;

        result_size = max_csize;
        ret = bp__compress(data, *size, compressed, &result_size);
        if (ret != BP_OK) {
            free(compressed);
            return BP_ECOMP;
        }

        *size = result_size;
        written = write(w->fd, compressed, result_size);
        free(compressed);
    }

    if ((uint64_t) written != *size) return BP_EFILEWRITE;

    /* change offset */
    *offset = w->filesize;
    w->filesize += written;

    return BP_OK;
}

int bp__writer_find(bp__writer_t*w,
                    const enum comp_type comp,
                    const uint64_t size,
                    void *data,
                    bp__writer_cb seek,
                    bp__writer_cb miss)
{
    int ret = 0;
    int match = 0;
    uint64_t offset, size_tmp;

    /* Write padding first */
    ret = bp__writer_write(w, kNotCompressed, NULL, NULL, NULL);
    if (ret != BP_OK) return ret;

    offset = w->filesize;
    size_tmp = size;

    /* Start seeking from bottom of file */
    while (offset >= size) {
        ret = bp__writer_read(w, comp, offset - size, &size_tmp, &data);
        if (ret != BP_OK) break;

        /* Break if matched */
        if (seek(w, data) == 0) {
            match = 1;
            break;
        }

        offset -= size;
    }

    /* Not found - invoke miss */
    if (!match)
        ret = miss(w, data);

    return ret;
}
