#ifndef ECDH_H_
#define ECDH_H_

int ecdh_create_keys(struct buf **priv_key, struct buf **pub_key);
int ecdh_generate_secret(const struct buf *priv,
                         const struct buf *client_pub_key,
                         struct buf **secret);

#endif /* ECDH_H_ */
