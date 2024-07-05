CC=gcc
CFLAGS=-g
CLIBS=-lpthread
BINS_DIR=bin/
HEADER_DIR=hdr/
SOURCE_DIR=src/
OBJS=${BINS_DIR}threadlib.o	\
	${BINS_DIR}gl.o

main:${OBJS} ${BINS_DIR}main.o
	${CC} ${CFLAGS} $^ -o $@ ${CLIBS}

${BINS_DIR}main.o:main.c
	${CC} ${CFLAGS} -c $^ -o $@

${BINS_DIR}threadlib.o:${SOURCE_DIR}threadlib.c
	${CC} ${CFLAGS} -c $^ -o $@
	
${BINS_DIR}gl.o:${SOURCE_DIR}gl.c
	${CC} ${CFLAGS} -c $^ -o $@


prepare:
	@if [ ! -d ${BINS_DIR} ]; then \
		echo "Instantiating new bin directory";	\
		mkdir ${BINS_DIR};	\
	fi

clean:
	@if [ -e ${BINS_DIR} ]; then \
		rm -rf ${BINS_DIR};	\
	fi
	@if [ -f main.o ]; then \
		rm main.o;	\
	fi
	@if [ -f main.exe ]; then \
		rm main.exe;	\
	fi

build: ${OBJS} main.o main

rebuild: clean prepare build
