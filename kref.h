#ifndef _KREF_H_
#define _KREF_H_

#ifdef __cplusplus
extern "C" {
#endif

struct kref {
    unsigned int refcount;
};

void kref_init(struct kref *kref);
void kref_get(struct kref *kref);
int kref_put(struct kref *kref, void (*release) (struct kref *kref));

#ifdef __cplusplus
}
#endif

#endif /* _KREF_H_ */
