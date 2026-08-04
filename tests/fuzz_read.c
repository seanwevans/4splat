// Fuzz harness for the 4Splat reader: feed arbitrary bytes to read_splat4DVideo
// and make sure it never crashes, over-reads, or leaks on malformed input.
//
// The reader sizes allocations from attacker-controlled header dimensions, so
// this is the main untrusted-input surface.
//
// Build with libFuzzer (best coverage):
//   clang -DUNIT_TEST -fsanitize=fuzzer,address,undefined tests/fuzz_read.c -o fuzz_read
//   ./fuzz_read tests/fuzz_corpus         # or: make fuzz && ./tests/fuzz_read tests/fuzz_corpus
//
// Or as a standalone corpus runner (no libFuzzer runtime required; works with
// any sanitizer and with AFL++'s file mode):
//   gcc -DUNIT_TEST -DSPLAT_FUZZ_STANDALONE -fsanitize=address,undefined
//       tests/fuzz_read.c -o fuzz_read
//   ./fuzz_read file1.4spl file2 ...

// fmemopen() is POSIX.1-2008; request it before any libc header is pulled in
// (including transitively via 4splat.c) so it is declared with the right
// prototype instead of defaulting to implicit int.
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#ifndef UNIT_TEST
#define UNIT_TEST
#endif
#include "../4splat.c"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  FILE *fp = fmemopen((void *)(uintptr_t)data, size, "rb");
  if (!fp)
    return 0;
  Splat4DVideo v;
  if (read_splat4DVideo(fp, &v)) {
    // Exercise the read-back path on anything that parsed cleanly.
    (void)compute_video_checksum(&v);
    free_splat4DVideo(&v);
  }
  fclose(fp);
  return 0;
}

#ifdef SPLAT_FUZZ_STANDALONE
int main(int argc, char **argv) {
  for (int i = 1; i < argc; ++i) {
    FILE *f = fopen(argv[i], "rb");
    if (!f)
      continue;
    if (fseek(f, 0, SEEK_END) != 0) {
      fclose(f);
      continue;
    }
    long n = ftell(f);
    if (n < 0 || fseek(f, 0, SEEK_SET) != 0) {
      fclose(f);
      continue;
    }
    uint8_t *buf = malloc((size_t)n ? (size_t)n : 1);
    if (buf && fread(buf, 1, (size_t)n, f) == (size_t)n)
      LLVMFuzzerTestOneInput(buf, (size_t)n);
    free(buf);
    fclose(f);
  }
  return 0;
}
#endif
