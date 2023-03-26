CC = gcc
CFLAGS = -Wno-deprecated-declarations -fPIC -Wall -Wextra -O2 -g
LDFLAGS = -shared
TARGET_LIB = libkmem.so

SRCS = kref.c kref_alloc.c list.c buf.c ecdh.c sha256.c base64.c
OBJS = $(SRCS:.c=.o)

.PHONY: all
all: ${TARGET_LIB}

$(TARGET_LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

$(SRCS:.c=.d):%.d:%.c
	$(CC) $(CFLAGS) -MM $< >$@

include $(SRCS:.c=.d)

.PHONY: clean
clean:
	rm -f ${TARGET_LIB} ${OBJS} $(SRCS:.c=.d)
