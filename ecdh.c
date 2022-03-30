#include <openssl/ecdh.h>
#include <openssl/evp.h>
#include <openssl/bn.h>
#include "buf.h"

int ecdh_create_keys(struct buf **priv_key, struct buf **pub_key)
{
    int rc = 0;
    EC_KEY *ecdh;
    EC_GROUP *ec_group;
    BN_CTX *bn_ctx;
    const EC_POINT *pub_key_ec_point;
    const BIGNUM *bignum_priv_key;
    *priv_key = NULL;
    *pub_key = NULL;

    u8 *public_key_buf;
    size_t public_key_size;

    ecdh = EC_KEY_new();
    if (ecdh == NULL) {
        rc = -1;
        print_e("Can't alloc for EC_KEY\n");
        goto out1;
    }

    ec_group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    if (ec_group == NULL) {
        rc = -2;
        print_e("Can't create EC_GROUP\n");
        goto out2;
    }

    rc = EC_KEY_set_group(ecdh, ec_group);
    if (rc != 1) {
        rc = -3;
        print_e("Can't assign EC_GROUP to EC_KEY\n");
        goto out3;
    }

    rc = EC_KEY_generate_key(ecdh);
    if(rc != 1) {
        rc = -4;
        print_e("Can't generate ECDH keys\n");
        goto out3;
    }

    rc = EC_KEY_check_key(ecdh);
    if (rc != 1) {
        rc = -5;
        print_e("EC_KEY check failed\n");
        goto out3;
    }

    bn_ctx = BN_CTX_new();
    pub_key_ec_point = EC_KEY_get0_public_key(ecdh);
    public_key_size = EC_POINT_point2buf(ec_group, pub_key_ec_point,
                        POINT_CONVERSION_COMPRESSED, &public_key_buf, bn_ctx);
    if (!public_key_size) {
        rc = -6;
        print_e("Can't convert EC_POINT to binary\n");
        goto out4;
    }

    *pub_key = buf_alloc(public_key_size);
    if (!*pub_key) {
        rc = -7;
        print_e("Can't allocate struct buf of size %lu\n", public_key_size);
        goto out5;
    }
    memcpy((*pub_key)->data, public_key_buf, public_key_size);

    bignum_priv_key = EC_KEY_get0_private_key(ecdh);
    *priv_key = buf_alloc(BN_num_bytes(bignum_priv_key));
    if (!*pub_key) {
        rc = -8;
        print_e("Can't allocate struct buf of size %d\n", BN_num_bytes(bignum_priv_key));
        goto out5;
    }

    BN_bn2bin(bignum_priv_key, (*priv_key)->data);
    kmem_ref(*pub_key);
    kmem_ref(*priv_key);
    rc = 0;

out5:
    OPENSSL_free(public_key_buf);
out4:
    BN_CTX_free(bn_ctx);
out3:
    EC_GROUP_clear_free(ec_group);
out2:
    EC_KEY_free(ecdh);
out1:
    kmem_deref(pub_key);
    kmem_deref(priv_key);
    return rc;
}


int ecdh_generate_secret(const struct buf *my_priv_key,
                         const struct buf *his_pub_key,
                         struct buf **secret)
{
    EC_KEY *ecdh;
    EC_GROUP *ec_group;
    BN_CTX *bn_ctx;
    EC_POINT *pub_key_ec_point;
    BIGNUM *bignum_priv_key;
    size_t secret_len;
    int rc;

    ecdh = EC_KEY_new();
    if (ecdh == NULL) {
        rc = -1;
        goto out1;
    }

    ec_group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    if (ec_group == NULL) {
        rc = -2;
        goto out2;
    }

    rc = EC_KEY_set_group(ecdh, ec_group);
    if (rc != 1) {
        rc = -3;
        goto out3;
    }

    bignum_priv_key = BN_bin2bn(my_priv_key->data, buf_len(my_priv_key), NULL);
    if (!bignum_priv_key) {
        rc = -4;
        goto out3;
    }

    rc = EC_KEY_set_private_key(ecdh, bignum_priv_key);
    if (rc != 1) {
        rc = -5;
        goto out4;
    }

    bn_ctx = BN_CTX_new();
    pub_key_ec_point = EC_POINT_new(ec_group);
    if (pub_key_ec_point == NULL) {
        rc = -6;
        goto out5;
    }
    rc = EC_POINT_oct2point(ec_group, pub_key_ec_point,
                            his_pub_key->data, buf_len(his_pub_key), bn_ctx);
    if (rc != 1) {
        rc = -7;
        goto out6;
    }

    secret_len = (EC_GROUP_get_degree(ec_group) + 7) / 8;
    *secret = buf_alloc(secret_len);
    secret_len = ECDH_compute_key((*secret)->data, secret_len,
                                    pub_key_ec_point, ecdh, NULL);
    buf_put(*secret, secret_len);
    rc = 0;

out6:
    EC_POINT_free(pub_key_ec_point);
out5:
    BN_CTX_free(bn_ctx);
out4:
    BN_free(bignum_priv_key);
out3:
    EC_GROUP_clear_free(ec_group);
out2:
    EC_KEY_free(ecdh);
out1:
    return rc;
}
