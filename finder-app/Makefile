CROSS_COMPILE?=
CC=$(CROSS_COMPILE)gcc

.PHONY: all
all:
	$(CC) $(CCFLAGS) -c writer.c
	$(CC) $(LDFLAGS) writer.o -o writer

.PHONY: clean
clean:
	rm -f writer.o writer