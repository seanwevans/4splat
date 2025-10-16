#ifndef UNIT_TEST
#define UNIT_TEST
#endif
#include "../4splat.c"
#include <stddef.h>

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
                              /*frames=*/1, /*pSize=*/2,
                              /*flags=*/SPLAT_FLAG_PRECISION_FLOAT32);
}

static void make_palette(Splat4D palette[2]) {
  palette[0] = create_splat4D(0, 1, 2, 3, 4, 5, 6, 7, 0.5f, 0.6f, 0.7f, 1.0f);
  palette[1] = create_splat4D(1, 1, 2, 2, 3, 3, 4, 4, 0.8f, 0.2f, 0.1f, 0.9f);
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

static bool test_validate_fails_for_null_video(void) { return !validate_splat4DVideo(NULL); }

static bool test_validate_fails_for_bad_magic(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DVideo video = create_splat4DVideo(make_header(), palette, indices);
  video.header.magic = 0;
  return !validate_splat4DVideo(&video);
}

static bool test_validate_fails_for_bad_version(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DVideo video = create_splat4DVideo(make_header(), palette, indices);
  video.header.version[0] = 2;
  return !validate_splat4DVideo(&video);
}

static bool test_validate_fails_for_zero_width(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DHeader header = make_header();
  header.width = 0;
  Splat4DVideo video = create_splat4DVideo(header, palette, indices);
  return !validate_splat4DVideo(&video);
}

static bool test_validate_fails_for_zero_height(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DHeader header = make_header();
  header.height = 0;
  Splat4DVideo video = create_splat4DVideo(header, palette, indices);
  return !validate_splat4DVideo(&video);
}

static bool test_validate_fails_for_zero_depth(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DHeader header = make_header();
  header.depth = 0;
  Splat4DVideo video = create_splat4DVideo(header, palette, indices);
  return !validate_splat4DVideo(&video);
}

static bool test_validate_fails_for_zero_frames(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DHeader header = make_header();
  header.frames = 0;
  Splat4DVideo video = create_splat4DVideo(header, palette, indices);
  return !validate_splat4DVideo(&video);
}

static bool test_validate_fails_for_zero_palette_size(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DHeader header = make_header();
  header.pSize = 0;
  Splat4DVideo video = create_splat4DVideo(header, palette, indices);
  return !validate_splat4DVideo(&video);
}

static bool test_validate_fails_for_bad_footer_marker(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DVideo video = create_splat4DVideo(make_header(), palette, indices);
  video.footer.end = 0;
  return !validate_splat4DVideo(&video);
}

static bool test_validate_fails_for_big_endian_flag(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DVideo video = create_splat4DVideo(make_header(), palette, indices);
  video.header.flags |= SPLAT_FLAG_ENDIAN_BIG;
  video.footer.checksum = compute_video_checksum(&video);
  return !validate_splat4DVideo(&video);
}

static bool test_validate_fails_for_unsupported_precision(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DVideo video = create_splat4DVideo(make_header(), palette, indices);
  video.header.flags &= ~SPLAT_FLAG_PRECISION_MASK;
  video.header.flags |= SPLAT_FLAG_PRECISION_FLOAT64;
  video.footer.checksum = compute_video_checksum(&video);
  return !validate_splat4DVideo(&video);
}

static bool test_validate_detects_corrupted_index(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DVideo video = create_splat4DVideo(make_header(), palette, indices);

  // Mutate index data without updating the footer checksum to simulate corruption.
  video.index.index[0] ^= 1u;
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

  bool headers_match = memcmp(&original.header, &loaded.header, sizeof(Splat4DHeader)) == 0;
  bool palette_match = memcmp(original.palette.palette, loaded.palette.palette,
                              original.header.pSize * sizeof(Splat4D)) == 0;
  bool index_match = memcmp(original.index.index, loaded.index.index,
                            header_total_indices(&original.header) * sizeof(uint64_t)) == 0;
  bool footer_match = memcmp(&original.footer, &loaded.footer, sizeof(Splat4DFooter)) == 0;

  free_splat4DVideo(&loaded);

  return headers_match && palette_match && index_match && footer_match;
}

static bool test_write_header_rejects_null_fp(void) {
  Splat4DHeader header = make_header();
  return !write_splat4DHeader(NULL, &header);
}

static bool test_write_header_rejects_null_header(void) {
  FILE *fp = tmpfile();
  if (!fp)
    return false;
  bool ok = !write_splat4DHeader(fp, NULL);
  fclose(fp);
  return ok;
}

static bool test_read_header_rejects_null_fp(void) {
  Splat4DHeader header;
  return !read_splat4DHeader(NULL, &header);
}

static bool test_read_header_rejects_null_header(void) {
  FILE *fp = tmpfile();
  if (!fp)
    return false;
  Splat4DHeader header = make_header();
  fwrite(&header, sizeof(Splat4DHeader), 1, fp);
  rewind(fp);
  bool ok = !read_splat4DHeader(fp, NULL);
  fclose(fp);
  return ok;
}

static bool test_write_palette_rejects_nulls(void) {
  Splat4D palette_data[2];
  make_palette(palette_data);
  Splat4DPalette palette = create_splat4DPalette(palette_data);
  bool all_failed = true;
  all_failed &= !write_splat4DPalette(NULL, &palette, 2);
  FILE *fp = tmpfile();
  if (!fp)
    return false;
  all_failed &= !write_splat4DPalette(fp, NULL, 2);
  Splat4DPalette empty = create_splat4DPalette(NULL);
  all_failed &= !write_splat4DPalette(fp, &empty, 2);
  all_failed &= !write_splat4DPalette(fp, &palette, 0);
  fclose(fp);
  return all_failed;
}

static bool test_read_palette_rejects_invalid_inputs(void) {
  Splat4DPalette palette;
  FILE *fp = tmpfile();
  if (!fp)
    return false;
  bool fail_fp = !read_splat4DPalette(NULL, &palette, 2);
  bool fail_palette = !read_splat4DPalette(fp, NULL, 2);
  bool fail_count = !read_splat4DPalette(fp, &palette, 0);
  fclose(fp);
  return fail_fp && fail_palette && fail_count;
}

static bool test_read_palette_fails_on_short_file(void) {
  FILE *fp = tmpfile();
  if (!fp)
    return false;
  // Write less data than required for two splats.
  Splat4D one = create_splat4D(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  fwrite(&one, sizeof(Splat4D), 1, fp);
  rewind(fp);
  Splat4DPalette palette = {.palette = NULL};
  bool ok = !read_splat4DPalette(fp, &palette, 2) && palette.palette == NULL;
  fclose(fp);
  return ok;
}

static bool test_write_index_rejects_nulls(void) {
  uint64_t indices[4];
  make_indices(indices);
  Splat4DIndex index = create_splat4DIndex(indices);
  bool all_failed = true;
  all_failed &= !write_splat4DIndex(NULL, &index, 4);
  FILE *fp = tmpfile();
  if (!fp)
    return false;
  all_failed &= !write_splat4DIndex(fp, NULL, 4);
  Splat4DIndex empty = create_splat4DIndex(NULL);
  all_failed &= !write_splat4DIndex(fp, &empty, 4);
  all_failed &= !write_splat4DIndex(fp, &index, 0);
  fclose(fp);
  return all_failed;
}

static bool test_read_index_rejects_invalid_inputs(void) {
  Splat4DIndex index;
  FILE *fp = tmpfile();
  if (!fp)
    return false;
  bool fail_fp = !read_splat4DIndex(NULL, &index, 4);
  bool fail_index = !read_splat4DIndex(fp, NULL, 4);
  bool fail_count = !read_splat4DIndex(fp, &index, 0);
  fclose(fp);
  return fail_fp && fail_index && fail_count;
}

static bool test_read_index_fails_on_short_file(void) {
  FILE *fp = tmpfile();
  if (!fp)
    return false;
  uint64_t value = 42;
  fwrite(&value, sizeof(uint64_t), 1, fp);
  rewind(fp);
  Splat4DIndex index = {.index = NULL};
  bool ok = !read_splat4DIndex(fp, &index, 4) && index.index == NULL;
  fclose(fp);
  return ok;
}

static bool test_write_footer_rejects_nulls(void) {
  Splat4DHeader header = make_header();
  Splat4DFooter footer = create_splat4DFooter(&header);
  FILE *fp = tmpfile();
  if (!fp)
    return false;
  bool ok = !write_splat4DFooter(NULL, &footer) && !write_splat4DFooter(fp, NULL);
  fclose(fp);
  return ok;
}

static bool test_read_footer_rejects_nulls(void) {
  Splat4DFooter footer;
  FILE *fp = tmpfile();
  if (!fp)
    return false;
  bool ok = !read_splat4DFooter(NULL, &footer) && !read_splat4DFooter(fp, NULL);
  fclose(fp);
  return ok;
}

static bool test_read_footer_fails_on_short_file(void) {
  FILE *fp = tmpfile();
  if (!fp)
    return false;

  Splat4DFooter footer = create_splat4DFooter(&(Splat4DHeader){0});
  fwrite(&footer, sizeof(uint8_t), sizeof(Splat4DFooter) - 1, fp);
  rewind(fp);

  Splat4DFooter out = {0};
  bool failed = !read_splat4DFooter(fp, &out);
  fclose(fp);
  return failed;
}

static bool test_write_video_rejects_nulls(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DVideo video = create_splat4DVideo(make_header(), palette, indices);
  FILE *fp = tmpfile();
  if (!fp)
    return false;
  bool ok = !write_splat4DVideo(NULL, &video) && !write_splat4DVideo(fp, NULL);
  fclose(fp);
  return ok;
}

static bool test_read_video_rejects_nulls(void) {
  Splat4DVideo video;
  FILE *fp = tmpfile();
  if (!fp)
    return false;
  bool ok = !read_splat4DVideo(NULL, &video) && !read_splat4DVideo(fp, NULL);
  fclose(fp);
  return ok;
}

static bool test_read_video_fails_on_truncated_index(void) {
  Splat4D palette_data[2];
  uint64_t indices[4];
  make_palette(palette_data);
  make_indices(indices);
  Splat4DVideo video = create_splat4DVideo(make_header(), palette_data, indices);

  FILE *fp = tmpfile();
  if (!fp)
    return false;

  fwrite(&video.header, sizeof(Splat4DHeader), 1, fp);
  fwrite(video.palette.palette, sizeof(Splat4D), video.header.pSize, fp);
  // Intentionally omit index/footers.
  rewind(fp);

  Splat4DVideo loaded;
  loaded.palette.palette = (Splat4D *)0x1;
  loaded.index.index = (uint64_t *)0x1;
  bool ok = !read_splat4DVideo(fp, &loaded) && loaded.palette.palette == NULL &&
            loaded.index.index == NULL;
  fclose(fp);
  return ok;
}

static bool test_read_video_fails_on_crc_mismatch(void) {
  Splat4D palette_data[2];
  uint64_t indices[4];
  make_palette(palette_data);
  make_indices(indices);
  Splat4DVideo video = create_splat4DVideo(make_header(), palette_data, indices);

  FILE *fp = tmpfile();
  if (!fp)
    return false;

  write_splat4DVideo(fp, &video);

  // Corrupt checksum at end of file.
  fseek(fp, -(long)sizeof(Splat4DFooter), SEEK_END);
  uint32_t bad = video.footer.checksum ^ 0xFFFFFFFFu;
  fwrite(&bad, sizeof(uint32_t), 1, fp);
  fflush(fp);
  rewind(fp);

  Splat4DVideo loaded;
  bool ok = !read_splat4DVideo(fp, &loaded);
  fclose(fp);
  if (ok)
    free_splat4DVideo(&loaded);
  return ok;
}

static bool test_read_video_rejects_big_endian_flag(void) {
  Splat4D palette_data[2];
  uint64_t indices[4];
  make_palette(palette_data);
  make_indices(indices);
  Splat4DVideo video = create_splat4DVideo(make_header(), palette_data, indices);
  video.header.flags |= SPLAT_FLAG_ENDIAN_BIG;

  FILE *fp = tmpfile();
  if (!fp)
    return false;

  if (!write_splat4DVideo(fp, &video)) {
    fclose(fp);
    return false;
  }

  rewind(fp);
  Splat4DVideo loaded;
  bool ok = !read_splat4DVideo(fp, &loaded);
  fclose(fp);
  return ok;
}

static bool test_read_video_fails_on_invalid_footer_marker(void) {
  Splat4D palette_data[2];
  uint64_t indices[4];
  make_palette(palette_data);
  make_indices(indices);
  Splat4DVideo video = create_splat4DVideo(make_header(), palette_data, indices);

  FILE *fp = tmpfile();
  if (!fp)
    return false;

  if (!write_splat4DVideo(fp, &video)) {
    fclose(fp);
    return false;
  }

  // Corrupt the footer terminator.
  fseek(fp, -(long)sizeof(Splat4DFooter), SEEK_END);
  fseek(fp, (long)offsetof(Splat4DFooter, end), SEEK_CUR);
  uint32_t invalid_end = 0;
  fwrite(&invalid_end, sizeof(uint32_t), 1, fp);
  fflush(fp);
  rewind(fp);

  Splat4DVideo loaded;
  memset(&loaded, 0, sizeof loaded);
  bool failed = !read_splat4DVideo(fp, &loaded);
  fclose(fp);
  free_splat4DVideo(&loaded);
  return failed;
}

static bool test_idxoffset_sanity_mismatch(void) {
  Splat4DHeader header = make_header();
  Splat4DFooter footer = create_splat4DFooter(&header);
  footer.idxoffset += sizeof(Splat4D);
  return !sanity_check_idxoffset_file(NULL, &header, &footer) &&
         !check_idxoffset_file(NULL, &header, &footer);
}

static bool test_header_defaults_to_float32_precision(void) {
  Splat4DHeader header = create_splat4DHeader(/*width=*/2, /*height=*/2, /*depth=*/1,
                                              /*frames=*/1, /*pSize=*/2, /*flags=*/0);
  return (header.flags & SPLAT_FLAG_PRECISION_MASK) == SPLAT_FLAG_PRECISION_FLOAT32;
}

static test_case TESTS[] = {
    {"crc32_known_value", test_crc32_known_value},
    {"checksum_matches_footer", test_compute_video_checksum_matches_footer},
    {"idxoffset_helpers_agree", test_idxoffset_helpers_agree},
    {"validate_succeeds_for_valid_video", test_validate_succeeds_for_valid_video},
    {"validate_fails_for_bad_checksum", test_validate_fails_for_bad_checksum},
    {"validate_fails_for_null_video", test_validate_fails_for_null_video},
    {"validate_fails_for_bad_magic", test_validate_fails_for_bad_magic},
    {"validate_fails_for_bad_version", test_validate_fails_for_bad_version},
    {"validate_fails_for_zero_width", test_validate_fails_for_zero_width},
    {"validate_fails_for_zero_height", test_validate_fails_for_zero_height},
    {"validate_fails_for_zero_depth", test_validate_fails_for_zero_depth},
    {"validate_fails_for_zero_frames", test_validate_fails_for_zero_frames},
    {"validate_fails_for_zero_palette_size", test_validate_fails_for_zero_palette_size},
    {"validate_fails_for_bad_footer_marker", test_validate_fails_for_bad_footer_marker},
    {"validate_fails_for_big_endian_flag", test_validate_fails_for_big_endian_flag},
    {"validate_fails_for_unsupported_precision", test_validate_fails_for_unsupported_precision},
    {"validate_detects_corrupted_index", test_validate_detects_corrupted_index},
    {"write_and_read_round_trip", test_write_and_read_round_trip},
    {"write_header_rejects_null_fp", test_write_header_rejects_null_fp},
    {"write_header_rejects_null_header", test_write_header_rejects_null_header},
    {"read_header_rejects_null_fp", test_read_header_rejects_null_fp},
    {"read_header_rejects_null_header", test_read_header_rejects_null_header},
    {"write_palette_rejects_nulls", test_write_palette_rejects_nulls},
    {"read_palette_rejects_invalid_inputs", test_read_palette_rejects_invalid_inputs},
    {"read_palette_fails_on_short_file", test_read_palette_fails_on_short_file},
    {"write_index_rejects_nulls", test_write_index_rejects_nulls},
    {"read_index_rejects_invalid_inputs", test_read_index_rejects_invalid_inputs},
    {"read_index_fails_on_short_file", test_read_index_fails_on_short_file},
    {"write_footer_rejects_nulls", test_write_footer_rejects_nulls},
    {"read_footer_rejects_nulls", test_read_footer_rejects_nulls},
    {"read_footer_fails_on_short_file", test_read_footer_fails_on_short_file},
    {"write_video_rejects_nulls", test_write_video_rejects_nulls},
    {"read_video_rejects_nulls", test_read_video_rejects_nulls},
    {"read_video_fails_on_truncated_index", test_read_video_fails_on_truncated_index},
    {"read_video_fails_on_crc_mismatch", test_read_video_fails_on_crc_mismatch},
    {"read_video_rejects_big_endian_flag", test_read_video_rejects_big_endian_flag},
    {"read_video_fails_on_invalid_footer_marker", test_read_video_fails_on_invalid_footer_marker},
    {"idxoffset_sanity_mismatch", test_idxoffset_sanity_mismatch},
    {"header_defaults_to_float32_precision", test_header_defaults_to_float32_precision},
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
