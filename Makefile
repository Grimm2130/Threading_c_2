CC=gcc
CFLAGS=-g
CLIBS=-lpthread
BINS_DIR=bin/
HEADER_DIR=hdr/
SOURCE_DIR=src/
OBJS=${BINS_DIR}threadlib.o

main:${OBJS} ${BINS_DIR}main.o
	${CC} ${CFLAGS} $^ -o $@ ${CLIBS}

${BINS_DIR}main.o:main.c
	${CC} ${CFLAGS} -c $^ -o $@

${BINS_DIR}threadlib.o:${SOURCE_DIR}threadlib.c
	${CC} ${CFLAGS} -c $^ -o $@

prepare:
	@if [ ! -d ${BINS_DIR} ]; then \
		echo "Instantiating new bin directory";	\
		mkdir ${BINS_DIR};	\
	fi

clean:
	@if [ "(ls -A ${BINS_DIR})" ]; then \
		rm ${BINS_DIR}*.o;	\
	fi

build: ${OBJS} main.o main

rebuild: clean prepare build