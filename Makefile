
CC=gcc
CFLAGS=-g -O3 -Wall
LDFLAGS=-g

all: ssdv libssdv.so test

ssdv: main.o ssdv.o rs8.o ssdv.h rs8.h
	$(CC) $(LDFLAGS) main.o ssdv.o rs8.o -o ssdv

libssdv.so: ssdvutils.lo ssdv.lo rs8.lo ssdv.h rs8.h
	$(CC) -shared $(LDFLAGS) ssdvutils.lo ssdv.lo rs8.lo -o libssdv.so

test: test.c libssdv.so
	$(CC) $(LDFLAGS) test.c -l:libssdv.so -o test

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

%.lo : %.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

install: all
	mkdir -p ${DESTDIR}/usr/bin
	mkdir -p ${DESTDIR}/usr/include/ssdv
	install -m 755 ssdv ${DESTDIR}/usr/bin
	install -m 755 libssdv.so ${DESTDIR}/usr/lib
	install -m 644 ssdv.h ${DESTDIR}/usr/include/ssdv
	install -m 644 ssdvutils.h ${DESTDIR}/usr/include/ssdv
	install -m 644 rs8.h ${DESTDIR}/usr/include/ssdv

clean:
	rm -f *.o *.lo ssdv libssdv.so test

