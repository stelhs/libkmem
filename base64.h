#ifndef BASE64_H_
#define BASE64_H_

struct buf *base64_encode(const struct buf *buf);
struct buf *base64_decode(const struct buf *buf);

#endif /* BASE64_H_ */
