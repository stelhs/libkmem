#include "buf.h"
#include <ctype.h>
#include <stdarg.h>

static void buf_destructor(void *mem)
{
    struct buf *buf = (struct buf *)mem;
    memset(buf->data, 0, buf->len);
}

struct buf *buf_alloc(size_t size)
{
    struct buf *buf = kzref_alloc(sizeof *buf + size, buf_destructor);
    if (!buf)
        return NULL;

    buf->data = (u8 *)(buf + 1);
    buf->len = size;
    buf->payload_len = 0;
    return buf;
}

struct buf *buf_strdub(const char *str)
{
    size_t len = strlen(str);
    struct buf *buf = buf_alloc(len);
    if (!buf)
        return NULL;

    memcpy(buf->data, str, len);
    buf_put(buf, len);
    return buf;
}

void *buf_concatenate(const struct buf *b1, const struct buf *b2)
{
    struct buf *result;

    if (!b1 || !b2)
        return NULL;

    const size_t s1 = buf_len(b1);
    const size_t s2 = buf_len(b2);
    if (!s1 || !s2)
        return NULL;

    result = buf_alloc(s1 + s2);
    if (!result)
        return NULL;

    memcpy(result->data, b1->data, s1);
    memcpy(result->data + s1, b2->data, s2);
    return result;
}


void _buf_dump(const struct buf *buf, const char *name)
{
    uint cnt = 0;
    uint row_cnt, row_len;

    printf("\n");
    if (name)
        printf("buf %s:\n", name);

    if (!buf) {
        printf("buf is NULL\n");
        return;
    }
    const size_t len = buf_len(buf);
    printf("len: %lu, payload_len: %lu\n", buf->len, buf->payload_len);

    while(cnt < len) {
        printf("%.4x - ", cnt);
        row_len = (cnt + 16) < len ? 16 : (len - cnt);
        for (row_cnt = 0; row_cnt < 16; row_cnt++) {
            if (row_cnt < row_len)
                printf("%.2x ", buf->data[cnt + row_cnt]);
            else
                printf("   ");
            if (row_cnt == 7)
                printf(": ");
        }
        printf("| ");
        for (row_cnt = 0; row_cnt < row_len; row_cnt++) {
            u8 b = buf->data[cnt + row_cnt];
            if (isprint(b))
                printf("%c", b);
            else
                printf(".");
        }
        cnt += row_len;
        printf("\n");
    }
    fflush(stdout);
}

void _buf_list_dump(const struct list *list, const char *list_name)
{
    struct le *le;
    struct buf *buf;
    char buf_name[16];
    unsigned cnt = 0;
    unsigned numbers;

    printf("\n");
    if (list_name)
        printf("list %s:\n", list_name);
    if (!list) {
        printf("list is NULL\n");
        return;
    }

    numbers = list_count(list);

    if (!numbers)
        return;
    printf("buffers in list: %d\n", numbers);

    printf("---");
    LIST_FOREACH(list, le) {
        buf = (struct buf *)list_ledata(le);
        sprintf(buf_name, "%d", cnt);
        _buf_dump(buf, buf_name);
        cnt ++;
    }
    printf("---\n");
    fflush(stdout);
}


struct buf *buf_cpy(const void *src, size_t len)
{
    struct buf *buf = buf_alloc(len);
    if (!buf)
        return NULL;

    memcpy(buf->data, src, len);
    buf_put(buf, len);
    return buf;
}


char *buf_to_str(struct buf *buf)
{
    char *str;
    if (!buf)
        return NULL;

    const size_t len = buf_len(buf);
    if (len == buf->len && !buf->data[buf->len - 1])
        return (char *)buf->data;

    str = (char *)kref_alloc(buf->len + 1, NULL);
    if (!str)
        return NULL;
    kmem_link_to_kmem(str, buf);
    memcpy(str, buf->data, len);
    str[len] = 0;
    return str;
}

void buf_put(struct buf *buf, size_t payload_len)
{
    if (payload_len > buf->len)
        return;
    buf->payload_len = payload_len;
}

struct list *buf_split(const struct buf *buf, char sep)
{
    const size_t len = buf_len(buf);
    size_t part_len = 0;
    struct list *list;
    struct buf *part_buf;
    uint i;

    list = list_create();
    if (!list) {
        print_e("Can't alloc new list\n");
        goto err;
    }

    const u8 *part = buf->data;
    for (i = 0; i < len; i++) {
        const u8 *p = buf->data + i;
        if (*p != sep) {
            part_len ++;
            continue;
        }

        part_buf = buf_cpy((void *)part, part_len);
        if (!part_buf) {
            print_e("Can't alloc buffer\n");
            goto err;
        }

        buf_list_append(list, part_buf);
        part_len = 0;
        part = p + 1;
    }

    if (part_len) {
        part_buf = buf_cpy((void *)part, part_len);
        if (!part_buf) {
            print_e("Can't alloc buffer\n");
            goto err;
        }
        buf_list_append(list, part_buf);
    }

    kmem_ref(list);
err:
    kmem_deref(&list);
    return list;
}

struct buf *buf_trim(const struct buf *buf)
{
    const size_t len = buf_len(buf);
    size_t new_len;
    struct buf *new_buf;
    const u8 *start = buf->data;
    const u8 *back;

    if (!len)
        return (struct buf *)buf;

    while(isspace(*start) && *start != 0) start++;

    back = buf->data + len;
    while(isspace(*--back) && back != start);
    new_len = back - start + 1;
    if (new_len <= 0)
        return buf_alloc(0);

    new_buf = buf_cpy(start, new_len);
    new_buf->data[new_len + 1] = 0;
    kmem_link_to_kmem(new_buf, (void *)buf);
    return new_buf;
}

struct buf *buf_sprintf(const char* format, ...)
{
    char msg[1024 * 16];
    va_list args;
    va_start(args, format);
    vsprintf(msg, format, args);
    va_end(args);

    size_t len = strlen(msg);
    struct buf *buf = buf_alloc(len);
    if (!buf)
        return NULL;

    memcpy(buf->data, msg, len);
    buf_put(buf, len);
    return buf;
}

void buf_erase(struct buf *buf)
{
    memset(buf->data, 0, buf->len);
    buf->payload_len = 0;
}

struct buf *file_get_contents(const char *filename)
{
    FILE *f;
    long fsize;
    struct buf *buf = NULL;
    int rc;

    f = fopen(filename, "r");
    if (f == NULL) {
        print_e("failed to fopen %s\n", filename);
        return NULL;
    }

    rc = fseek(f, 0, SEEK_END);
    if (rc < 0) {
        print_e("failed to fseek %s\n", filename);
        goto out;
    }

    fsize = ftell(f);
    if (fsize < 0) {
        print_e("failed to ftell %s\n", filename);
        goto out;
    }

    if (fsize == 0) {
        print_e("file %s is empty\n", filename);
        goto out;
    }

    if (fsize > 1024 * 1024) {
        print_e("file_get_contents() can not load file more then 1Mbyte size. "
               "File %s, size: %ld\n", filename, fsize);
        goto out;
    }

    rc = fseek(f, 0, SEEK_SET);
    if (rc < 0) {
        print_e("failed to fseek %s\n", filename);
        goto out;
    }

    buf = buf_alloc(fsize);
    if (!buf) {
        print_e("Can't alloc for payload data file %s", filename);
        goto out;
    }

    rc = fread(buf->data, fsize, 1, f);
    if (rc < 0) {
        print_e("failed to fread %s\n", filename);
        kmem_deref(&buf);
        buf = NULL;
        goto out;
    }

    kmem_ref(buf);
out:
    kmem_deref(&buf);
    fclose(f);
    return buf;
}
