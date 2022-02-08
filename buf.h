#ifndef BUF_H_
#define BUF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "kref_alloc.h"
#include "list.h"

struct buf {
    u8 *data;
    size_t len;
    size_t payload_len;
    struct le le;
};

struct buf *buf_alloc(size_t size);
struct buf *buf_strdub(const char *str);

static inline struct buf *bufz_alloc(size_t size)
{
    struct buf *buf = buf_alloc(size);
    if (!buf)
        return NULL;
    memset(buf->data, 0, size);
    return buf;
}

#define buf_list_append(list, buf) list_append(list, &buf->le, buf)

#define buf_deref(buf) kmem_deref(buf)
struct buf *buf_cpy(const void *src, size_t len);
#define buf_ref(buf) kmem_ref(buf);
void *buf_concatenate(const struct buf *b1, const struct buf *b2);
char *buf_to_str(struct buf *buf);
void _buf_dump(const struct buf *buf, const char *name);
#define buf_dump(buf) _buf_dump(buf, #buf)
void _buf_list_dump(const struct list *list, const char *name);
#define buf_list_dump(list) _buf_list_dump(list, #list)
void buf_put(struct buf *buf, size_t payload_len);
struct list *buf_split(const struct buf *buf, char sep);
struct buf *buf_trim(const struct buf *buf);
struct buf *buf_sprintf(const char* format, ...);
void buf_erase(struct buf *buf);

static inline size_t buf_payload_size(const struct buf *buf)
{
    return buf->payload_len ? buf->payload_len : buf->len;
}

#ifdef __cplusplus
}
#endif

#endif /* BUF_H_ */
