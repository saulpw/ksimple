CFLAGS=-O0 -g -Werror -Wno-pointer-sign
CC=cc

test: ksimple
	./ksimple t.k

ksimple: a.c a.h
	${CC} ${CFLAGS} -o $@ $<
