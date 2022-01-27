#include "buf.h"
#include <ctype.h>

static void buf_destructor(void *mem)
{
    struct buf *buf = (struct buf *)mem;
    memset(buf->data, 0, buf->len);
}

struct buf *buf_alloc(uint size)
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
    uint len = strlen(str) + 1;
    struct buf *buf = buf_alloc(len);
    if (!buf)
        return NULL;

    memcpy(buf->data, str, len);
    buf_put(buf, len);
    return buf;
}

void *buf_concatenate(struct buf *b1, struct buf *b2)
{
    struct buf *result;

    if (!b1->len || !b2->len)
        return NULL;

    result = buf_alloc(b1->len + b2->len);
    if (!result)
        return NULL;

    memcpy(result->data, b1->data, b1->len);
    memcpy(result->data + b1->len, b2->data, b2->len);
    return result;
}


void buf_dump(struct buf *buf, const char *name)
{
    uint cnt = 0;
    uint row_cnt, row_len;

    printf("\n");
    if (name)
        printf("buf: %s, ", name);

    if (!buf) {
        printf("buf is NULL\n");
        return;
    }
    printf("len: %u\n", buf->len);

    while(cnt < buf->len) {
        printf("%.4x - ", cnt);
        row_len = (cnt + 16) < buf->len ? 16 : (buf->len - cnt);
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
}

void buf_list_dump(struct list *list)
{
    struct le *le;
    struct buf *buf;
    char buf_name[16];
    uint cnt = 0;
    uint numbers;

    if (!list) {
        printf("list is empty\n");
        return;
    }

    numbers = list_count(list);

    printf("---\n");
    printf("buffers in list: %d\n", numbers);
    if (!numbers)
        return;

    LIST_FOREACH(list, le) {
        buf = (struct buf *)list_ledata(le);
        sprintf(buf_name, "buf item %d", cnt);
        buf_dump(buf, buf_name);
        cnt ++;
    }
    printf("---\n");
}


struct buf *buf_cpy(void *src, uint len)
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
    uint len;
    if (!buf)
        return NULL;
    if (!buf->data[buf->len - 1])
        return (char *)buf->data;

    str = (char *)kref_alloc(buf->len + 1, NULL);
    if (!str)
        return NULL;
    kmem_link_to_kmem(str, buf);
    len = buf->payload_len ? buf->payload_len : buf->len;
    memcpy(str, buf->data, len);
    str[len] = 0;
    return str;
}

void buf_put(struct buf *buf, uint payload_len)
{
    if (payload_len > buf->len)
        return;
    buf->payload_len = payload_len;
}

struct list *buf_split(struct buf *buf, char sep)
{
    int len = buf->payload_len ? buf->payload_len : buf->len;
    uint part_len = 0;
    struct list *list;
    struct buf *part_buf;
    int i;

    list = list_create();
    if (!list) {
        print_e("Can't alloc new list\n");
        goto err;
    }

    u8 *part = buf->data;
    for (i = 0; i < len; i++) {
        u8 *p = buf->data + i;
        if (*p != sep) {
            part_len ++;
            continue;
        }

        if (!part_len) {
            part_len = 0;
            part = p + 1;
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