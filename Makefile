CFLAGS= -std=c11 -g -Wall -Wno-missing-braces

test: test_sao_math
	./test_sao_math

gameguy_test.dylib: sao_gameguy_test.c
	cc -dynamiclib -undefined dynamic_lookup $(CFLAGS) -o gameguy_test.dylib sao_gameguy_test.c

gameguy: sao_gameguy.h sao_gameguy.c
	cc $(CFLAGS) -framework OpenGL `pkg-config --cflags sdl2` `pkg-config --libs sdl2` sao_gameguy.c -o gameguy

test_sao_math: sao_math.h test_sao_math.c
	cc test_sao_math.c -o test_sao_math

check-syntax:
	clang -o /dev/null $(CFLAGS) -S ${CHK_SOURCES}

tags:
	etags ./*.h
