ifeq ($(DEBUG), 1)
	CFLAGS = -g -DDEBUG
else
	CLAGS = -DNDEBUG
endif

CC = gcc $(CFLAGS)
OBJS = parse-util.o util.o
EXEC = shell

all: $(EXEC)

$(EXEC): shell.c $(OBJS) build
	$(CC) -o $@ $< $(OBJS)

$(OBJS): util.h

build: ls wc cd

ls: ls.c alloc_test.c alloc_test.h
	$(CC) -o $@ $^

wc: wc.c
	$(CC) -o $@ $<

cd: cd.c
	$(CC) -o $@ $<

clean:
	rm -f *.o shell ls wc cd
