all: main

main: main.o
	gcc -o main main.o disk.o inode.o util.o vfs.o myfs.o

main.o: main.c
	gcc -c myfs.c disk.c inode.c util.c vfs.c main.c

clean: 
	rm -rf *.o main
