#include <openssl/evp.h>
#include "buf.h"
#include "base64.h"

#ifdef _WIN32
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif


struct buf *base64_encode(const struct buf *buf)
{
    const size_t len = buf_len(buf);
    size_t pl = 4 * ((len + 5) / 3);
    struct buf *output = buf_alloc(pl);
    if (!output) {
        print_e("Can't allocate struct buf of size %lu\n", pl);
        return NULL;
    }
    ssize_t ol = EVP_EncodeBlock(output->data, buf->data, len);
    if (ol == -1) {
        print_e("Can't encode the given buf\n");
        return kmem_deref(&output);
    }
    buf_put(output, ol);
    return output;
}


struct buf *base64_decode(const struct buf *buf)
{
    const size_t len = buf_len(buf);
    const size_t pl = (3 * len / 4) + 1;
    struct buf *output = buf_alloc(pl);
    if (!output) {
        print_e("Can't allocate struct buf of size %lu\n", pl);
        return NULL;
    }
    ssize_t ol = EVP_DecodeBlock(output->data, buf->data, len);
    if (ol == -1) {
        print_e("Can't decode the given buf\n");
        return kmem_deref(&output);
    }
    if (*(buf->data + len - 1) == '=')
        ol--;
    if (*(buf->data + len - 2) == '=')
        ol--;
    buf_put(output, ol);
    return output;
}
