# 4Splat codec build.
#
#   make          full-featured build (all compression backends linked)
#   make plain    self-contained build (None + RLE only, no dependencies)
#   make test         run the test suite against the full-featured build
#   make test-plain   run the test suite against the self-contained build
#   make clean
#
# The full-featured build needs the development packages for zlib, bzip2, xz
# (liblzma), brotli, zstd, lz4 and lcms2. Individual backends can be toggled by
# overriding FEATURES/LIBS, e.g.:
#   make FEATURES="-DSPLAT_WITH_ZLIB -DSPLAT_WITH_ZSTD" LIBS="-lz -lzstd"

CC ?= gcc
CFLAGS ?= -Wall -Wpedantic -std=c11 -O2
FEATURES ?= -DSPLAT_WITH_ALL
LIBS ?= -lz -lbz2 -llzma -lbrotlienc -lbrotlidec -lzstd -llz4 -llcms2

.PHONY: all plain test test-plain clean

all: 4splat

4splat: 4splat.c
	$(CC) $(CFLAGS) $(FEATURES) 4splat.c $(LIBS) -o $@

plain: 4splat.c
	$(CC) $(CFLAGS) 4splat.c -o 4splat

test: tests/test_4splat.c 4splat.c
	$(CC) $(CFLAGS) -DUNIT_TEST $(FEATURES) tests/test_4splat.c $(LIBS) -o tests/test_4splat
	./tests/test_4splat

test-plain: tests/test_4splat.c 4splat.c
	$(CC) $(CFLAGS) -DUNIT_TEST tests/test_4splat.c -o tests/test_4splat
	./tests/test_4splat

clean:
	rm -f 4splat tests/test_4splat
