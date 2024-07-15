CC=gcc
DFLAGS=-D DEBUG=1
CFLAGS=-g ${DFLAGS}
CLIBS=-lpthread
BINS_DIR=bin/
HEADER_DIR=hdr/
SOURCE_DIR=src/
APPS_DIR=apps/
OBJS=${BINS_DIR}gl.o	\
	${BINS_DIR}threadlib.o	\
	${BINS_DIR}wait_queue.o	\
	${BINS_DIR}appl_timer.o	\
	${BINS_DIR}tr_light.o

# Executables

## Applications
main:${OBJS} ${BINS_DIR}main.o
	${CC} ${CFLAGS} $^ -o $@ ${CLIBS}

threadpool_test: ${OBJS} ${BINS_DIR}threadpool_test.o
	${CC} ${CFLAGS} $^ -o $@ ${CLIBS}

test_appln_timer: ${OBJS} ${BINS_DIR}test_appln_timer.o
	${CC} ${CFLAGS} $^ -o $@ ${CLIBS}

tr_light_test: ${OBJS} ${BINS_DIR}tr_light_test.o
	${CC} ${CFLAGS} $^ -o $@ ${CLIBS}


# Objects
${BINS_DIR}main.o:${APPS_DIR}main.c
	${CC} ${CFLAGS} -c $^ -o $@

${BINS_DIR}tr_light_test.o:${APPS_DIR}tr_light_test.c
	${CC} ${CFLAGS} -c $^ -o $@

${BINS_DIR}threadpool_test.o:${APPS_DIR}threadpool_test.c
	${CC} ${CFLAGS} -c $^ -o $@

${BINS_DIR}test_appln_timer.o:${APPS_DIR}test_appln_timer.c
	${CC} ${CFLAGS} -c $^ -o $@

${BINS_DIR}threadlib.o:${SOURCE_DIR}threadlib.c
	${CC} ${CFLAGS} -c $^ -o $@

${BINS_DIR}wait_queue.o:${SOURCE_DIR}wait_queue.c
	${CC} ${CFLAGS} -c $^ -o $@

${BINS_DIR}tr_light.o:${SOURCE_DIR}tr_light.c
	${CC} ${CFLAGS} -c $^ -o $@

${BINS_DIR}appl_timer.o:${SOURCE_DIR}appl_timer.c
	${CC}  -c $^ -o $@
# ${CC} ${CFLAGS} -c $^ -o $@
	
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
	@if [ -f main ]; then \
		rm main.exe;	\
	fi
	@if [ -f threadpool_test ]; then \
		rm threadpool_test;	\
	fi
	@if [ -f tr_light_test ]; then \
		rm tr_light_test;	\
	fi
	@if [ -f test_appln_timer ]; then \
		rm test_appln_timer;	\
	fi
	@rm *.txt;

build: ${OBJS} ${BINS_DIR}threadpool_test.o ${BINS_DIR}main.o main threadpool_test

threadpool_test: prepare ${OBJS} ${BINS_DIR}threadpool_test.o threadpool_test

test_appln_timer_test: prepare ${OBJS} ${BINS_DIR}test_appln_timer.o test_appln_timer

traffic_light_test: prepare ${OBJS} ${BINS_DIR}tr_light_test.o tr_light_test

# traffic_light_test: prepare ${OBJS} ${BINS_DIR}tr_light_test.o tr_light_test

rebuild: clean prepare build
