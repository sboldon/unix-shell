all: shell.c parse-util.o util.o  build
	gcc shell.c -o shell parse-util.o util.o

parse-util.o: parse-util.c util.h
	gcc -c parse-util.c

util.o: util.c util.h
	gcc -c util.c

build: ls wc cd
	echo "Build completed"

ls: ls.c
	gcc ls.c -o ls

wc: wc.c
	gcc wc.c -o wc

cd: cd.c
	gcc cd.c -o cd

clean:
	rm -f *.o shell ls wc cd
