#ifndef UNIT_TEST
#define UNIT_TEST
#endif
#include "../4splat.c"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

typedef bool (*test_func)(void);

typedef struct {
  const char *name;
  test_func func;
} test_case;

static Splat4DHeader make_header(void) {
  return create_splat4DHeader(/*width=*/2, /*height=*/2, /*depth=*/1,
                               /*frames=*/1, /*pSize=*/2, /*flags=*/0);
}

static void make_palette(Splat4D palette[2]) {
  palette[0] =
      create_splat4D(0, 1, 2, 3, 4, 5, 6, 7, 0.5f, 0.6f, 0.7f, 1.0f);
  palette[1] =
      create_splat4D(1, 1, 2, 2, 3, 3, 4, 4, 0.8f, 0.2f, 0.1f, 0.9f);
}

static void make_indices(uint64_t indices[4]) {
  indices[0] = 0;
  indices[1] = 1;
  indices[2] = 0;
  indices[3] = 1;
}

static bool test_crc32_known_value(void) {
  const char *input = "123456789";
  uint32_t actual = crc32(input, strlen(input));
  return actual == 0xCBF43926u;
}

static bool test_compute_video_checksum_matches_footer(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);

  Splat4DVideo video = create_splat4DVideo(make_header(), palette, indices);
  uint32_t expected = compute_video_checksum(&video);
  return expected == video.footer.checksum;
}

static bool test_idxoffset_helpers_agree(void) {
  Splat4DHeader header = make_header();
  uint32_t forward = compute_idxoffset_forward(&header);
  uint32_t reverse = compute_idxoffset_reverse(&header);
  uint32_t expected = sizeof(Splat4DHeader) + header.pSize * sizeof(Splat4D);
  return forward == reverse && forward == expected;
}

static bool test_validate_succeeds_for_valid_video(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DVideo video = create_splat4DVideo(make_header(), palette, indices);
  return validate_splat4DVideo(&video);
}

static bool test_validate_fails_for_bad_checksum(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DVideo video = create_splat4DVideo(make_header(), palette, indices);
  video.footer.checksum ^= 0xFFFFFFFFu;
  return !validate_splat4DVideo(&video);
}

static bool test_validate_fails_for_zero_dimension(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DHeader header = make_header();
  header.width = 0;
  Splat4DVideo video = create_splat4DVideo(header, palette, indices);
  return !validate_splat4DVideo(&video);
}

static bool test_write_and_read_round_trip(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DVideo original = create_splat4DVideo(make_header(), palette, indices);

  FILE *fp = tmpfile();
  if (!fp)
    return false;

  bool wrote = write_splat4DVideo(fp, &original);
  if (!wrote) {
    fclose(fp);
    return false;
  }

  rewind(fp);
  Splat4DVideo loaded;
  bool read = read_splat4DVideo(fp, &loaded);
  fclose(fp);
  if (!read)
    return false;

  bool headers_match = memcmp(&original.header, &loaded.header,
                              sizeof(Splat4DHeader)) == 0;
  bool palette_match = memcmp(original.palette.palette, loaded.palette.palette,
                              original.header.pSize * sizeof(Splat4D)) == 0;
  bool index_match = memcmp(original.index.index, loaded.index.index,
                            header_total_indices(&original.header) *
                                sizeof(uint64_t)) == 0;
  bool footer_match = memcmp(&original.footer, &loaded.footer,
                             sizeof(Splat4DFooter)) == 0;

  free_splat4DVideo(&loaded);

  return headers_match && palette_match && index_match && footer_match;
}

static test_case TESTS[] = {
    {"crc32_known_value", test_crc32_known_value},
    {"checksum_matches_footer", test_compute_video_checksum_matches_footer},
    {"idxoffset_helpers_agree", test_idxoffset_helpers_agree},
    {"validate_succeeds_for_valid_video", test_validate_succeeds_for_valid_video},
    {"validate_fails_for_bad_checksum", test_validate_fails_for_bad_checksum},
    {"validate_fails_for_zero_dimension", test_validate_fails_for_zero_dimension},
    {"write_and_read_round_trip", test_write_and_read_round_trip},
};

int main(void) {
  size_t failed = 0;
  for (size_t i = 0; i < ARRAY_SIZE(TESTS); ++i) {
    bool ok = TESTS[i].func();
    printf("[%s] %s\n", ok ? "PASS" : "FAIL", TESTS[i].name);
    if (!ok)
      failed++;
  }

  if (failed != 0) {
    fprintf(stderr, "%zu tests failed\n", failed);
    return 1;
  }

  printf("All %zu tests passed\n", ARRAY_SIZE(TESTS));
  return 0;
}
