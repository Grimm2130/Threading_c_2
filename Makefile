CC=gcc
DFLAGS=-D DEBUG=1
CFLAGS=-g ${DFLAGS}
CLIBS=-lpthread
BINS_DIR=bin/
HEADER_DIR=hdr/
SOURCE_DIR=src/
APPS_DIR=apps/
OBJS=${BINS_DIR}threadlib.o	\
	${BINS_DIR}gl.o

main:${OBJS} ${BINS_DIR}main.o
	${CC} ${CFLAGS} $^ -o $@ ${CLIBS}

threadpool_test: ${OBJS} ${BINS_DIR}threadpool_test.o
	${CC} ${CFLAGS} $^ -o $@ ${CLIBS}

${BINS_DIR}main.o:${APPS_DIR}main.c
	${CC} ${CFLAGS} -c $^ -o $@

${BINS_DIR}threadpool_test.o:${APPS_DIR}threadpool_test.c
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
	@if [ -f threadpool_test.exe ]; then \
		rm threadpool_test.exe;	\
	fi

build: ${OBJS} ${BINS_DIR}threadpool_test.o ${BINS_DIR}main.o main threadpool_test

tests: prepare ${OBJS} ${BINS_DIR}threadpool_test.o threadpool_test

rebuild: clean prepare build
