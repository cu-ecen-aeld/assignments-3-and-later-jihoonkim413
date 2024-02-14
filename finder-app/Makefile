CC=$(CROSS_COMPILE)gcc
CFLAG= -g -Wall

.PHONY: all
all:
	$(CC) $(CFLAG) -c -o writer.o writer.c
	$(CC) $(CFLAG) -I/ writer.o -o writer

.PHONY: clean
clean:
	rm -f *.o writer *.elf *.map *.txt