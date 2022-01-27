#define TRUE  1
#define FALSE 0

typedef unsigned char u8;
typedef unsigned int u32;
typedef u8 byte;
typedef unsigned int uint;
typedef unsigned long ulong;

#define UNUSED(x) (void)(x)
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:        the pointer to the member.
 * @type:       the type of the container struct this is embedded in.
 * @member:     the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({                      \
        typeof( ((type *)0)->member ) *__mptr = (ptr);  \
        (type *)( (char *)__mptr - offsetof(type,member) );})


#ifdef PRINT_ERR_TO_FILE
    #define print_e(format, ...) do { \
        FILE *f = fopen("/tmp/mv2_log_error", "a"); \
        fprintf(f, "%s +%d, %s() Error: ", __FILE__, __LINE__, __FUNCTION__); \
        fprintf(f, (format), ##__VA_ARGS__); \
        fclose(f); \
    } while(0)
#else
    #define print_e(format, ...) do { \
        fprintf(stderr, "%s +%d, %s() Error: ", __FILE__, __LINE__, __FUNCTION__); \
        fprintf(stderr, (format), ##__VA_ARGS__); \
    } while(0)
#endif

#ifdef PRINT_DBG
    #ifdef PRINT_DBG_TO_FILE
        #include <unistd.h>
        #define print_d(format, ...) do { \
            FILE *f = fopen("/tmp/mv2_log_debug", "a"); \
            fprintf(f, "%s +%d, %s(): ", __FILE__, __LINE__, __FUNCTION__); \
            fprintf(f, (format), ##__VA_ARGS__); \
            fclose(f); \
        } while(0)
    #else
        #define print_d(format, ...) do { \
            fprintf(stdout, "%s +%d, %s(): ", __FILE__, __LINE__, __FUNCTION__); \
            fprintf(stdout, (format), ##__VA_ARGS__); \
            fflush(stdout); \
        } while(0)
    #endif
#else
    #define print_d(format, ...)
#endif
