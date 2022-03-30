#include <openssl/sha.h>
#include "buf.h"

struct buf *sha256(const struct buf *src_buf, unsigned sha_len)
{
    SHA256_CTX sha256;
    int len;
    char *src;
    char hash[SHA256_DIGEST_LENGTH];
    uint dst_cnt = 0;

    struct buf *dst = buf_alloc(sha_len);
    if (!dst) {
        perror("Can't alloc for sha256\n");
        return NULL;
    }

    src = (char *)src_buf->data;
    len = buf_len(src_buf);
    while(dst_cnt < sha_len) {
        uint part_len = ((sha_len - dst_cnt) < sizeof hash) ?
                          sha_len - dst_cnt : sizeof hash;
        SHA256_Init(&sha256);
        while (len > 0) {
            if (len > 512)
                SHA256_Update(&sha256, src, 512);
            else
                SHA256_Update(&sha256, src, len);
            len -= 512;
            src += 512;
        }
        SHA256_Final((unsigned char *)hash, &sha256);
        memcpy(dst->data + dst_cnt, hash, part_len);
        dst_cnt += part_len;
        src = hash;
        len = sizeof hash;
    }
    return dst;
}
