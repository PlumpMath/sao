CFLAGS= -std=c11 -g -Wall -Wno-missing-braces

test: test_sao_math
	./test_sao_math

check-syntax:
	clang -o /dev/null $(CFLAGS) -S ${CHK_SOURCES}

tags:
	etags ./*.h
