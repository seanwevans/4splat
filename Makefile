# 4Splat codec build.
#
#   make          full-featured build (all compression backends linked)
#   make plain    self-contained build (None + RLE only, no dependencies)
#   make test         run the test suite against the full-featured build
#   make test-plain   run the test suite against the self-contained build
#   make bench        size comparison against PNG/GIF/QOI (quick subset)
#   make bench-full   the whole corpus, every scheme the build carries
#   make clean
#
# The full-featured build needs the development packages for zlib, bzip2, xz
# (liblzma), brotli, zstd, lz4 and lcms2. Individual backends can be toggled by
# overriding FEATURES/LIBS, e.g.:
#   make FEATURES="-DSPLAT_WITH_ZLIB -DSPLAT_WITH_ZSTD" LIBS="-lz -lzstd"

CC ?= gcc
CFLAGS ?= -Wall -Wpedantic -std=c11 -O2
FEATURES ?= -DSPLAT_WITH_ALL
LIBS ?= -lz -lbz2 -llzma -lbrotlienc -lbrotlidec -lzstd -llz4 -llcms2 -lm

.PHONY: all plain test test-plain bench bench-full fuzz fuzz-standalone clean

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

# Benchmark .4spl against PNG, GIF, QOI and the generic compressors.  The
# harness is stdlib-only Python; it reports whichever schemes this build of the
# binary carries.  'make plain' is enough to run it (None + RLE only).
bench: 4splat
	python3 tools/benchmark.py --binary ./4splat --quick

bench-full: 4splat
	python3 tools/benchmark.py --binary ./4splat --schemes all --colors 64

# Fuzz the reader with libFuzzer (needs clang and its fuzzer runtime):
#   make fuzz && ./tests/fuzz_read tests/fuzz_corpus
fuzz: tests/fuzz_read.c 4splat.c
	clang $(CFLAGS) -DUNIT_TEST -fsanitize=fuzzer,address,undefined tests/fuzz_read.c -o tests/fuzz_read

# Portable standalone runner (no libFuzzer runtime): feeds each file argument
# through the same entry point, under ASan/UBSan.
#   make fuzz-standalone && ./tests/fuzz_read tests/fuzz_corpus/* corpus/*
fuzz-standalone: tests/fuzz_read.c 4splat.c
	$(CC) $(CFLAGS) -DUNIT_TEST -DSPLAT_FUZZ_STANDALONE -fsanitize=address,undefined \
		tests/fuzz_read.c -o tests/fuzz_read

clean:
	rm -f 4splat tests/test_4splat tests/fuzz_read
