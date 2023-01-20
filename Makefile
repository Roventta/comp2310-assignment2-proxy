CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

OBJS = proxy.o csapp.o

all: proxy tiny

csapp.o: csapp.c
	$(CC) $(CFLAGS) -c csapp.c
sbuf.o: sbuf.c
	$(CC) $(CFLAGS) -c sbuf.c 
proxy.o: proxy.c  
	$(CC) $(CFLAGS) -c proxy.c

cache.o: cache.c cache.h
	$(CC) $(CFLAGS) -c cache.c

proxy: proxy.o cache.o csapp.o
	$(CC) $(CFLAGS) sbuf.o proxy.o cache.o csapp.o -o proxy $(LDFLAGS)

pte: sbuf.o prethreadExe.c
	$(CC) $(CFLAGS)  prethreadExe.c sbuf.o csapp.o -o pte $(LDFLAGS)

ct : cache.c sbuf.c csapp.c
	$(CC) $(CFLAGS) sbuf.o csapp.o cache_test.c -o ct $(LDFLAGS)

tiny:
	(cd tiny; make clean; make)
	(cd tiny/cgi-bin; make clean; make)

clean:
	rm -f *~ *.o proxy core 
	(cd tiny; make clean)
	(cd tiny/cgi-bin; make clean)
