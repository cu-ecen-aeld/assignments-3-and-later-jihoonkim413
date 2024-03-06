CROSS_COMPILE?=
CC?=${CROSS_COMPILE}gcc
LDFLAGS+=-lpthread -pthread
# CCFLAGS += -Wall \
# 			-Werror

.PHONY: all
all:
	$(CC) $(CCFLAGS) -c aesdsocket.c 
	$(CC) $(LDFLAGS) aesdsocket.o -o aesdsocket

.PHONY: clean
clean:
	rm -f aesdsocket.o aesdsocket
