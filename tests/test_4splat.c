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
  // Axis-aligned so the three distinct spatial sigmas in make_palette() survive
  // a round trip (the zero-flag default, isotropic, keeps only one).
  return create_splat4DHeader(/*width=*/2, /*height=*/2, /*depth=*/1,
                              /*frames=*/1, /*pSize=*/2,
                              /*flags=*/SPLAT_FLAG_PRECISION_FLOAT32 |
                                  (SPLAT_SHAPE_AXIS_ALIGNED << SPLAT_FLAG_SPLAT_SHAPE_SHIFT));
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

static bool test_create_splat4D(void) {
  float mu_x = 1.0f, sigma_x = 0.1f;
  float mu_y = 2.0f, sigma_y = 0.2f;
  float mu_z = 3.0f, sigma_z = 0.3f;
  float mu_t = 4.0f, sigma_t = 0.4f;
  float r = 0.5f, g = 0.6f, b = 0.7f, alpha = 0.8f;

  Splat4D splat =
      create_splat4D(mu_x, sigma_x, mu_y, sigma_y, mu_z, sigma_z, mu_t, sigma_t, r, g, b, alpha);

  return splat.mu_x == mu_x && splat.sigma_x == sigma_x && splat.mu_y == mu_y &&
         splat.sigma_y == sigma_y && splat.mu_z == mu_z && splat.sigma_z == sigma_z &&
         splat.mu_t == mu_t && splat.sigma_t == sigma_t && splat.r == r && splat.g == g &&
         splat.b == b && splat.alpha == alpha;
}

static bool test_create_splat4D_zero_values(void) {
  Splat4D splat =
      create_splat4D(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

  return splat.mu_x == 0.0f && splat.sigma_x == 0.0f && splat.mu_y == 0.0f &&
         splat.sigma_y == 0.0f && splat.mu_z == 0.0f && splat.sigma_z == 0.0f &&
         splat.mu_t == 0.0f && splat.sigma_t == 0.0f && splat.r == 0.0f && splat.g == 0.0f &&
         splat.b == 0.0f && splat.alpha == 0.0f;
}

static bool test_crc32_known_value(void) {
  const char *input = "123456789";
  uint32_t actual = splat_crc32(input, strlen(input));
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
  uint64_t forward = compute_idxoffset_forward(&header);
  uint64_t reverse = compute_idxoffset_reverse(&header);
  uint64_t expected = (uint64_t)sizeof(Splat4DHeader) +
                      (uint64_t)header.pSize * (uint64_t)palette_entry_disk_bytes(header.flags);
  return forward == reverse && forward == expected;
}

static bool test_idxoffset_forward_handles_large_palette(void) {
  // A palette large enough that the index offset (header + palette bytes) exceeds
  // 4 GiB must not be truncated when reported as a 64-bit offset.
  Splat4DHeader header = create_splat4DHeader(
      /*width=*/1, /*height=*/1, /*depth=*/1, /*frames=*/1,
      /*pSize=*/100000000u,
      /*flags=*/SPLAT_FLAG_PRECISION_FLOAT32 |
          (SPLAT_SHAPE_AXIS_ALIGNED << SPLAT_FLAG_SPLAT_SHAPE_SHIFT));

  uint64_t expected = (uint64_t)sizeof(Splat4DHeader) +
                      (uint64_t)header.pSize * (uint64_t)palette_entry_disk_bytes(header.flags);
  uint64_t forward = compute_idxoffset_forward(&header);

  return expected > UINT32_MAX && forward == expected;
}

static bool test_idxoffset_reverse_handles_large_files(void) {
  // Use a header whose index payload pushes the total file size beyond 4 GiB while the
  // palette remains small enough that the true index offset still fits into 32 bits.
  Splat4DHeader header =
      create_splat4DHeader(/*width=*/1, /*height=*/1, /*depth=*/1, /*frames=*/600000000,
                           /*pSize=*/1, /*flags=*/SPLAT_FLAG_PRECISION_FLOAT32);

  uint64_t index_bytes = header_total_indices(&header) * sizeof(uint64_t);
  uint64_t forward = compute_idxoffset_forward(&header);
  uint64_t reverse = compute_idxoffset_reverse(&header);
  uint64_t expected = (uint64_t)sizeof(Splat4DHeader) +
                      (uint64_t)header.pSize * (uint64_t)palette_entry_disk_bytes(header.flags);

  return index_bytes > UINT32_MAX && expected == forward && reverse == forward;
}

static bool test_check_index_values_accepts_valid(void) {
  uint64_t indices[4] = {0, 1, 0, 1};
  uint64_t bad_pos = 12345;
  // pSize 2, 1-byte width: all entries valid and representable.
  return check_index_values(indices, 4, /*pSize=*/2, /*idx_width=*/1, &bad_pos) == SPLAT_INDEX_OK;
}

static bool test_check_index_values_rejects_out_of_range(void) {
  uint64_t indices[4] = {0, 1, 5, 1};
  uint64_t bad_pos = 0;
  SplatIndexCheck r = check_index_values(indices, 4, /*pSize=*/2, /*idx_width=*/8, &bad_pos);
  return r == SPLAT_INDEX_OUT_OF_RANGE && bad_pos == 2;
}

static bool test_check_index_values_detects_too_wide(void) {
  // Valid palette references (< pSize) that nevertheless overflow a 1-byte
  // index width must be rejected rather than silently truncated on write.
  uint64_t indices[3] = {10, 300, 20};
  uint64_t bad_pos = 0;
  SplatIndexCheck r = check_index_values(indices, 3, /*pSize=*/1000, /*idx_width=*/1, &bad_pos);
  return r == SPLAT_INDEX_TOO_WIDE && bad_pos == 1;
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

static bool test_validate_fails_for_unsupported_precision(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DVideo video = create_splat4DVideo(make_header(), palette, indices);
  video.header.flags &= ~SPLAT_FLAG_PRECISION_MASK;
  video.header.flags |= SPLAT_FLAG_PRECISION_FLOAT128;
  video.footer.checksum = compute_video_checksum(&video);
  return !validate_splat4DVideo(&video);
}

static bool test_read_video_rejects_big_endian_flag(void) {
  Splat4D palette_data[2];
  uint64_t indices[4];
  make_palette(palette_data);
  make_indices(indices);
  Splat4DVideo video = create_splat4DVideo(make_header(), palette_data, indices);
  video.header.flags |= SPLAT_FLAG_ENDIAN_BIG;
  video.footer.checksum = compute_video_checksum(&video);

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

static bool test_header_defaults_to_float32_precision(void) {
  Splat4DHeader header = create_splat4DHeader(/*width=*/2, /*height=*/2, /*depth=*/1,
                                              /*frames=*/1, /*pSize=*/2, /*flags=*/0);
  return (header.flags & SPLAT_FLAG_PRECISION_MASK) == SPLAT_FLAG_PRECISION_FLOAT32;
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
  bool footer_match = original.footer.idxoffset == loaded.footer.idxoffset &&
                      original.footer.checksum == loaded.footer.checksum &&
                      original.footer.end == loaded.footer.end;

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

static bool test_read_header_fails_on_short_file(void) {
  FILE *fp = tmpfile();
  if (!fp)
    return false;

  Splat4DHeader header = make_header();
  // Write slightly fewer bytes than required
  fwrite(&header, sizeof(uint8_t), sizeof(Splat4DHeader) - 1, fp);
  rewind(fp);

  Splat4DHeader out;
  bool failed = !read_splat4DHeader(fp, &out);
  fclose(fp);
  return failed;
}

static bool test_write_palette_rejects_nulls(void) {
  Splat4D palette_data[2];
  make_palette(palette_data);
  Splat4DPalette palette = create_splat4DPalette(palette_data);
  bool all_failed = true;
  all_failed &= !write_splat4DPalette(NULL, &palette, 2, SPLAT_FLAG_PRECISION_FLOAT32);
  FILE *fp = tmpfile();
  if (!fp)
    return false;
  all_failed &= !write_splat4DPalette(fp, NULL, 2, SPLAT_FLAG_PRECISION_FLOAT32);
  Splat4DPalette empty = create_splat4DPalette(NULL);
  all_failed &= !write_splat4DPalette(fp, &empty, 2, SPLAT_FLAG_PRECISION_FLOAT32);
  all_failed &= !write_splat4DPalette(fp, &palette, 0, SPLAT_FLAG_PRECISION_FLOAT32);
  fclose(fp);
  return all_failed;
}

static bool test_read_palette_rejects_invalid_inputs(void) {
  Splat4DPalette palette;
  FILE *fp = tmpfile();
  if (!fp)
    return false;
  bool fail_fp = !read_splat4DPalette(NULL, &palette, 2, SPLAT_FLAG_PRECISION_FLOAT32);
  bool fail_palette = !read_splat4DPalette(fp, NULL, 2, SPLAT_FLAG_PRECISION_FLOAT32);
  bool fail_count = !read_splat4DPalette(fp, &palette, 0, SPLAT_FLAG_PRECISION_FLOAT32);
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
  bool ok = !read_splat4DPalette(fp, &palette, 2, SPLAT_FLAG_PRECISION_FLOAT32) &&
            palette.palette == NULL;
  fclose(fp);
  return ok;
}

static bool test_write_index_rejects_nulls(void) {
  uint64_t indices[4];
  make_indices(indices);
  Splat4DIndex index = create_splat4DIndex(indices);
  bool all_failed = true;
  all_failed &= !write_splat4DIndex(NULL, &index, 4, SPLAT_FLAG_PRECISION_FLOAT32);
  FILE *fp = tmpfile();
  if (!fp)
    return false;
  all_failed &= !write_splat4DIndex(fp, NULL, 4, SPLAT_FLAG_PRECISION_FLOAT32);
  Splat4DIndex empty = create_splat4DIndex(NULL);
  all_failed &= !write_splat4DIndex(fp, &empty, 4, SPLAT_FLAG_PRECISION_FLOAT32);
  all_failed &= !write_splat4DIndex(fp, &index, 0, SPLAT_FLAG_PRECISION_FLOAT32);
  fclose(fp);
  return all_failed;
}

static bool test_read_index_rejects_invalid_inputs(void) {
  Splat4DIndex index;
  FILE *fp = tmpfile();
  if (!fp)
    return false;
  bool fail_fp = !read_splat4DIndex(NULL, &index, 4, SPLAT_FLAG_PRECISION_FLOAT32);
  bool fail_index = !read_splat4DIndex(fp, NULL, 4, SPLAT_FLAG_PRECISION_FLOAT32);
  bool fail_count = !read_splat4DIndex(fp, &index, 0, SPLAT_FLAG_PRECISION_FLOAT32);
  fclose(fp);
  return fail_fp && fail_index && fail_count;
}

static bool test_read_index_fails_on_short_file(void) {
  FILE *fp = tmpfile();
  if (!fp)
    return false;
  uint8_t partial = 0x2A;
  fwrite(&partial, sizeof(uint8_t), 1, fp);
  rewind(fp);
  Splat4DIndex index = {.index = NULL};
  bool ok = !read_splat4DIndex(fp, &index, 4, SPLAT_FLAG_PRECISION_FLOAT32) && index.index == NULL;
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
  uint8_t buf[SPLAT_FOOTER_DISK_BYTES];
  serialize_footer(&footer, buf);
  fwrite(buf, sizeof(uint8_t), SPLAT_FOOTER_DISK_BYTES - 1, fp); // one byte short
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

  write_splat4DHeader(fp, &video.header);
  write_splat4DPalette(fp, &video.palette, video.header.pSize, video.header.flags);
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

  // Corrupt the stored checksum (footer bytes 8..11).
  fseek(fp, -(long)SPLAT_FOOTER_DISK_BYTES + 8, SEEK_END);
  uint8_t bad[4];
  store_u32le(bad, video.footer.checksum ^ 0xFFFFFFFFu);
  fwrite(bad, sizeof bad, 1, fp);
  fflush(fp);
  rewind(fp);

  Splat4DVideo loaded;
  bool ok = !read_splat4DVideo(fp, &loaded);
  fclose(fp);
  if (ok)
    free_splat4DVideo(&loaded);
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

  // Corrupt the footer terminator (footer bytes 12..15).
  fseek(fp, -(long)SPLAT_FOOTER_DISK_BYTES + 12, SEEK_END);
  uint8_t invalid_end[4] = {0, 0, 0, 0};
  fwrite(invalid_end, sizeof invalid_end, 1, fp);
  fflush(fp);
  rewind(fp);

  Splat4DVideo loaded;
  memset(&loaded, 0, sizeof loaded);
  bool failed = !read_splat4DVideo(fp, &loaded);
  fclose(fp);
  free_splat4DVideo(&loaded);
  return failed;
}

// Build an on-disk video whose stored checksum is valid for the (possibly
// mutated) header, so that read validation must reject it for a reason other
// than a CRC mismatch. Returns false on any I/O failure.
static bool write_video_with_header(FILE *fp, Splat4DHeader header) {
  Splat4D palette_data[2];
  uint64_t indices[4];
  make_palette(palette_data);
  make_indices(indices);
  // create_splat4DVideo computes the checksum over the header as given, so a
  // mutated magic/version is still covered by a matching CRC.
  Splat4DVideo video = create_splat4DVideo(header, palette_data, indices);
  return write_splat4DVideo(fp, &video);
}

static bool test_read_video_rejects_bad_magic(void) {
  Splat4DHeader header = make_header();
  header.magic = 0xDEADBEEFu;

  FILE *fp = tmpfile();
  if (!fp)
    return false;
  if (!write_video_with_header(fp, header)) {
    fclose(fp);
    return false;
  }
  rewind(fp);

  Splat4DVideo loaded;
  memset(&loaded, 0, sizeof loaded);
  bool failed = !read_splat4DVideo(fp, &loaded);
  fclose(fp);
  free_splat4DVideo(&loaded);
  return failed;
}

static bool test_read_video_rejects_bad_version(void) {
  Splat4DHeader header = make_header();
  header.version[0] = 2;

  FILE *fp = tmpfile();
  if (!fp)
    return false;
  if (!write_video_with_header(fp, header)) {
    fclose(fp);
    return false;
  }
  rewind(fp);

  Splat4DVideo loaded;
  memset(&loaded, 0, sizeof loaded);
  bool failed = !read_splat4DVideo(fp, &loaded);
  fclose(fp);
  free_splat4DVideo(&loaded);
  return failed;
}

static bool test_header_total_indices_checked(void) {
  uint64_t total = 0;
  Splat4DHeader h = make_header();

  // Happy path
  if (!header_total_indices_checked(&h, &total) || total != 4)
    return false;

  // Null pointers
  if (header_total_indices_checked(NULL, &total))
    return false;
  if (header_total_indices_checked(&h, NULL))
    return false;

  // Zero dimension
  Splat4DHeader h_zero = h;
  h_zero.width = 0;
  if (header_total_indices_checked(&h_zero, &total))
    return false;

  h_zero = h;
  h_zero.height = 0;
  if (header_total_indices_checked(&h_zero, &total))
    return false;

  // Overflow dimensions
  // 0xFFFFFFFF * 0xFFFFFFFF fits in uint64_t, but multiplying by 2 overflows it
  Splat4DHeader h_overflow = h;
  h_overflow.width = 0xFFFFFFFF;
  h_overflow.height = 0xFFFFFFFF;
  h_overflow.depth = 2;
  h_overflow.frames = 1;
  if (header_total_indices_checked(&h_overflow, &total))
    return false;

  return true;
}

static bool test_idxoffset_sanity_mismatch(void) {
  Splat4DHeader header = make_header();
  Splat4DFooter footer = create_splat4DFooter(&header);
  footer.idxoffset += sizeof(Splat4D);
  return !sanity_check_idxoffset_file(NULL, &header, &footer) &&
         !check_idxoffset_file(NULL, &header, &footer);
}

typedef struct {
  size_t total_bytes;
  size_t call_count;
} MockStreamCtx;

static bool mock_stream_consumer(const uint8_t *chunk, size_t n, void *ctx) {
  (void)chunk;
  MockStreamCtx *state = ctx;
  state->total_bytes += n;
  state->call_count++;
  return true;
}

static bool mock_stream_consumer_fail(const uint8_t *chunk, size_t n, void *ctx) {
  (void)chunk;
  (void)n;
  MockStreamCtx *state = ctx;
  state->call_count++;
  return false;
}

static bool test_stream_splat4DVideo_success(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DVideo video = create_splat4DVideo(make_header(), palette, indices);

  MockStreamCtx ctx = {0};
  // Use a chunk size that will trigger multiple calls.
  bool ok = stream_splat4DVideo(&video, 16, mock_stream_consumer, &ctx);

  size_t expected_bytes =
      sizeof(Splat4DHeader) + video.header.pSize * palette_entry_disk_bytes(video.header.flags) +
      header_total_indices(&video.header) * 1; // get_index_width_bytes returns 1

  return ok && ctx.total_bytes == expected_bytes && ctx.call_count > 1;
}

static bool test_stream_splat4DVideo_rejects_null(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DVideo video = create_splat4DVideo(make_header(), palette, indices);
  MockStreamCtx ctx = {0};

  bool fail_video = !stream_splat4DVideo(NULL, 1024, mock_stream_consumer, &ctx);
  bool fail_func = !stream_splat4DVideo(&video, 1024, NULL, &ctx);

  return fail_video && fail_func && ctx.call_count == 0;
}

static bool test_stream_splat4DVideo_stops_on_consumer_error(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DVideo video = create_splat4DVideo(make_header(), palette, indices);

  MockStreamCtx ctx = {0};
  bool ok = stream_splat4DVideo(&video, 16, mock_stream_consumer_fail, &ctx);

  // Should fail and call_count should be exactly 1, since the first call returns false and stops
  // streaming
  return !ok && ctx.call_count == 1;
}

// Round-trip a 2x2x1x1 / 2-entry video whose header carries `flags`, reporting
// whether the header and index survived write -> read unchanged.
static bool round_trip_with_flags(uint32_t flags, bool *headers_match, bool *index_match) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DHeader header = create_splat4DHeader(/*width=*/2, /*height=*/2, /*depth=*/1,
                                              /*frames=*/1, /*pSize=*/2, flags);
  Splat4DVideo original = create_splat4DVideo(header, palette, indices);

  FILE *fp = tmpfile();
  if (!fp)
    return false;
  if (!write_splat4DVideo(fp, &original)) {
    fclose(fp);
    return false;
  }
  rewind(fp);
  Splat4DVideo loaded;
  bool ok = read_splat4DVideo(fp, &loaded);
  fclose(fp);
  if (!ok)
    return false;

  *headers_match = memcmp(&original.header, &loaded.header, sizeof(Splat4DHeader)) == 0;
  *index_match = memcmp(original.index.index, loaded.index.index,
                        header_total_indices(&original.header) * sizeof(uint64_t)) == 0;
  free_splat4DVideo(&loaded);
  return true;
}

static bool test_round_trip_index_width(void) {
  const uint32_t widths[] = {SPLAT_INDEX_WIDTH_8, SPLAT_INDEX_WIDTH_16, SPLAT_INDEX_WIDTH_32,
                             SPLAT_INDEX_WIDTH_64};
  for (size_t i = 0; i < ARRAY_SIZE(widths); ++i) {
    uint32_t flags = SPLAT_FLAG_PRECISION_FLOAT32 | (widths[i] << SPLAT_FLAG_INDEX_WIDTH_SHIFT);
    bool headers_match = false, index_match = false;
    if (!round_trip_with_flags(flags, &headers_match, &index_match))
      return false;
    if (!headers_match || !index_match)
      return false;
  }
  return true;
}

static bool test_round_trip_preserves_descriptive_flags(void) {
  uint32_t flags = SPLAT_FLAG_PRECISION_FLOAT32 | SPLAT_FLAG_SORTED |
                   (SPLAT_SHAPE_AXIS_ALIGNED << SPLAT_FLAG_SPLAT_SHAPE_SHIFT) |
                   (SPLAT_COLOR_OKLAB << SPLAT_FLAG_COLOR_SPACE_SHIFT) |
                   (SPLAT_INTERP_LANCZOS << SPLAT_FLAG_INTERP_SHIFT) |
                   (0xABu << SPLAT_FLAG_METADATA_SHIFT);
  bool headers_match = false, index_match = false;
  if (!round_trip_with_flags(flags, &headers_match, &index_match))
    return false;
  return headers_match && index_match;
}

static bool test_read_video_rejects_reserved_splat_shape(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DVideo video = create_splat4DVideo(make_header(), palette, indices);
  video.header.flags |= (SPLAT_SHAPE_RESERVED << SPLAT_FLAG_SPLAT_SHAPE_SHIFT);
  video.footer.checksum = compute_video_checksum(&video);

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

static bool test_read_video_rejects_encryption(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DVideo video = create_splat4DVideo(make_header(), palette, indices);
  video.header.flags |= (1u << SPLAT_FLAG_ENCRYPTION_SHIFT);
  video.footer.checksum = compute_video_checksum(&video);

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

static bool test_half_conversion_exact_values(void) {
  // Values exactly representable in binary16 must survive the round trip.
  const float exact[] = {0.0f, 1.0f, -1.0f, 0.5f, -2.0f, 2048.0f, 0.0009765625f, 65504.0f};
  for (size_t i = 0; i < ARRAY_SIZE(exact); ++i) {
    if (half_to_float(float_to_half(exact[i])) != exact[i])
      return false;
  }
  // Idempotence: narrowing an already-half value again yields the same bits.
  uint16_t a = float_to_half(0.6f);
  uint16_t b = float_to_half(half_to_float(a));
  return a == b;
}

static bool test_round_trip_float64_palette(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DHeader header = create_splat4DHeader(
      2, 2, 1, 1, 2,
      SPLAT_FLAG_PRECISION_FLOAT64 | (SPLAT_SHAPE_AXIS_ALIGNED << SPLAT_FLAG_SPLAT_SHAPE_SHIFT));
  Splat4DVideo original = create_splat4DVideo(header, palette, indices);

  FILE *fp = tmpfile();
  if (!fp)
    return false;
  if (!write_splat4DVideo(fp, &original)) {
    fclose(fp);
    return false;
  }
  rewind(fp);
  Splat4DVideo loaded;
  bool ok = read_splat4DVideo(fp, &loaded);
  fclose(fp);
  if (!ok)
    return false;

  // Widening float32 -> float64 -> float32 is exact, so the palette is identical.
  bool palette_match =
      memcmp(original.palette.palette, loaded.palette.palette, 2 * sizeof(Splat4D)) == 0;
  bool precision_ok =
      (loaded.header.flags & SPLAT_FLAG_PRECISION_MASK) == SPLAT_FLAG_PRECISION_FLOAT64;
  free_splat4DVideo(&loaded);
  return palette_match && precision_ok;
}

static bool test_round_trip_float16_palette(void) {
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  // float16 is selected explicitly, bypassing the float32 default in sanitize;
  // axis-aligned so all three spatial sigmas are stored.
  Splat4DHeader header =
      create_splat4DHeader(2, 2, 1, 1, 2, SPLAT_SHAPE_AXIS_ALIGNED << SPLAT_FLAG_SPLAT_SHAPE_SHIFT);
  header.flags = (header.flags & ~SPLAT_FLAG_PRECISION_MASK) | SPLAT_FLAG_PRECISION_FLOAT16;
  Splat4DVideo original = create_splat4DVideo(header, palette, indices);

  FILE *fp = tmpfile();
  if (!fp)
    return false;
  if (!write_splat4DVideo(fp, &original)) {
    fclose(fp);
    return false;
  }
  rewind(fp);
  Splat4DVideo loaded;
  bool ok = read_splat4DVideo(fp, &loaded);
  fclose(fp);
  if (!ok)
    return false;

  // float16 storage is lossy: the recovered palette equals the originals passed
  // through the narrowing conversion.
  Splat4D expected[2];
  memset(expected, 0, sizeof expected); // off-diagonal covariance stays zero
  for (int e = 0; e < 2; ++e) {
    float comps[12];
    memcpy(comps, &palette[e], sizeof comps);
    for (int i = 0; i < 12; ++i)
      comps[i] = half_to_float(float_to_half(comps[i]));
    memcpy(&expected[e], comps, sizeof comps);
  }
  bool palette_match = memcmp(expected, loaded.palette.palette, 2 * sizeof(Splat4D)) == 0;
  bool precision_ok =
      (loaded.header.flags & SPLAT_FLAG_PRECISION_MASK) == SPLAT_FLAG_PRECISION_FLOAT16;
  free_splat4DVideo(&loaded);
  return palette_match && precision_ok;
}

static bool test_rle_unit_roundtrip(void) {
  const uint8_t in[] = {5, 5, 5, 5, 7, 7, 1, 2, 2, 2};
  size_t clen = 0;
  uint8_t *comp = rle_compress(in, sizeof in, &clen);
  if (!comp)
    return false;
  uint8_t out[sizeof in];
  bool ok = rle_decompress(comp, clen, out, sizeof in) && memcmp(in, out, sizeof in) == 0;
  free(comp);
  return ok;
}

// Round-trip a 64-index / 2-entry video through every compression codec the
// current build can handle. Codecs the build lacks are skipped, so this test
// exercises None + RLE in the plain build and all linked codecs in the full one.
static bool test_round_trip_all_available_codecs(void) {
  const uint64_t total = 64;
  uint64_t *indices = malloc((size_t)total * sizeof(uint64_t));
  if (!indices)
    return false;
  for (uint64_t k = 0; k < total; k++)
    indices[k] = (k % 16 == 0) ? 1 : 0; // runs, so compression has something to do
  Splat4D palette[2];
  make_palette(palette);

  bool result = true;
  for (uint32_t codec = 0; codec < 16 && result; codec++) {
    if (!splat_compression_available(codec))
      continue;
    uint32_t flags = SPLAT_FLAG_PRECISION_FLOAT32 | (codec << SPLAT_FLAG_COMPRESSION_SHIFT);
    Splat4DHeader header = create_splat4DHeader(8, 8, 1, 1, 2, flags);
    Splat4DVideo original = create_splat4DVideo(header, palette, indices);

    FILE *fp = tmpfile();
    if (!fp) {
      result = false;
      break;
    }
    bool ok = write_splat4DVideo(fp, &original);
    if (ok) {
      rewind(fp);
      Splat4DVideo loaded;
      ok = read_splat4DVideo(fp, &loaded);
      if (ok) {
        ok = memcmp(indices, loaded.index.index, (size_t)total * sizeof(uint64_t)) == 0;
        free_splat4DVideo(&loaded);
      }
    }
    fclose(fp);
    if (!ok)
      result = false;
  }

  free(indices);
  return result;
}

static bool test_read_video_rejects_unavailable_codec(void) {
  // RAR (codec 3) has no backend in any build, so a file tagged with it must be
  // rejected on read.
  Splat4D palette[2];
  uint64_t indices[4];
  make_palette(palette);
  make_indices(indices);
  Splat4DVideo video = create_splat4DVideo(make_header(), palette, indices);
  video.header.flags |= (SPLAT_COMPRESSION_RAR << SPLAT_FLAG_COMPRESSION_SHIFT);
  video.footer.checksum = compute_video_checksum(&video);

  FILE *fp = tmpfile();
  if (!fp)
    return false;
  // Write the (uncompressed) body directly so the tagged-but-unhandled codec is
  // exercised purely on the read path.
  bool wrote = write_splat4DHeader(fp, &video.header) &&
               write_splat4DPalette(fp, &video.palette, video.header.pSize, video.header.flags) &&
               write_splat4DIndex(fp, &video.index, header_total_indices(&video.header),
                                  video.header.flags) &&
               write_splat4DFooter(fp, &video.footer);
  if (!wrote) {
    fclose(fp);
    return false;
  }
  rewind(fp);
  Splat4DVideo loaded;
  bool ok = !read_splat4DVideo(fp, &loaded);
  fclose(fp);
  return ok;
}

#ifdef SPLAT_WITH_LCMS2
static float abs_diff(float a, float b) {
  float d = a - b;
  return d < 0 ? -d : d;
}

static bool test_color_convert_round_trip(void) {
  Splat4D pal[3];
  pal[0] = create_splat4D(0, 0, 0, 0, 0, 0, 0, 0, 0.5f, 0.4f, 0.3f, 1.0f);
  pal[1] = create_splat4D(0, 0, 0, 0, 0, 0, 0, 0, 0.2f, 0.6f, 0.8f, 1.0f);
  pal[2] = create_splat4D(0, 0, 0, 0, 0, 0, 0, 0, 0.9f, 0.1f, 0.5f, 1.0f);
  Splat4D orig[3];
  memcpy(orig, pal, sizeof orig);

  if (!splat_convert_palette_colors(pal, 3, SPLAT_COLOR_SRGB, SPLAT_COLOR_REC2020))
    return false;
  bool changed = memcmp(orig, pal, sizeof orig) != 0;
  if (!splat_convert_palette_colors(pal, 3, SPLAT_COLOR_REC2020, SPLAT_COLOR_SRGB))
    return false;

  float maxerr = 0.0f;
  for (int i = 0; i < 3; ++i) {
    float e = abs_diff(pal[i].r, orig[i].r);
    if (e > maxerr)
      maxerr = e;
    e = abs_diff(pal[i].g, orig[i].g);
    if (e > maxerr)
      maxerr = e;
    e = abs_diff(pal[i].b, orig[i].b);
    if (e > maxerr)
      maxerr = e;
  }
  return changed && maxerr < 1e-3f;
}

static bool test_color_convert_identity(void) {
  Splat4D pal[1] = {create_splat4D(0, 0, 0, 0, 0, 0, 0, 0, 0.3f, 0.4f, 0.5f, 1.0f)};
  Splat4D orig = pal[0];
  if (!splat_convert_palette_colors(pal, 1, SPLAT_COLOR_SRGB, SPLAT_COLOR_SRGB))
    return false;
  return memcmp(&orig, &pal[0], sizeof orig) == 0;
}

static bool test_color_convert_rejects_unsupported(void) {
  // ACES AP0 has no profile in the backend and must be refused.
  Splat4D pal[1] = {create_splat4D(0, 0, 0, 0, 0, 0, 0, 0, 0.3f, 0.4f, 0.5f, 1.0f)};
  return !splat_convert_palette_colors(pal, 1, SPLAT_COLOR_SRGB, SPLAT_COLOR_ACES_AP0);
}

static bool test_oklab_round_trip(void) {
  Splat4D pal[3];
  pal[0] = create_splat4D(0, 0, 0, 0, 0, 0, 0, 0, 0.5f, 0.4f, 0.3f, 1.0f);
  pal[1] = create_splat4D(0, 0, 0, 0, 0, 0, 0, 0, 0.2f, 0.6f, 0.8f, 1.0f);
  pal[2] = create_splat4D(0, 0, 0, 0, 0, 0, 0, 0, 0.9f, 0.1f, 0.5f, 1.0f);
  Splat4D orig[3];
  memcpy(orig, pal, sizeof orig);

  if (!splat_convert_palette_colors(pal, 3, SPLAT_COLOR_SRGB, SPLAT_COLOR_OKLAB))
    return false;
  bool changed = memcmp(orig, pal, sizeof orig) != 0;
  if (!splat_convert_palette_colors(pal, 3, SPLAT_COLOR_OKLAB, SPLAT_COLOR_SRGB))
    return false;

  float maxerr = 0.0f;
  for (int i = 0; i < 3; ++i) {
    float e = abs_diff(pal[i].r, orig[i].r);
    if (e > maxerr)
      maxerr = e;
    e = abs_diff(pal[i].g, orig[i].g);
    if (e > maxerr)
      maxerr = e;
    e = abs_diff(pal[i].b, orig[i].b);
    if (e > maxerr)
      maxerr = e;
  }
  return changed && maxerr < 1e-4f;
}

// OKLab pivots through sRGB, so OKLab -> an lcms-modeled space works too.
static bool test_oklab_pivots_to_lcms_space(void) {
  Splat4D pal[1] = {create_splat4D(0, 0, 0, 0, 0, 0, 0, 0, 0.4f, 0.5f, 0.6f, 1.0f)};
  return splat_convert_palette_colors(pal, 1, SPLAT_COLOR_OKLAB, SPLAT_COLOR_REC2020);
}
#endif // SPLAT_WITH_LCMS2

static bool test_image_round_trip(void) {
  // 2x2: red, green, red, blue -> 3 distinct colors
  uint8_t rgb[12] = {255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255};
  Splat4DVideo v;
  if (!image_to_video(rgb, 2, 2, &v))
    return false;

  bool ok = v.header.pSize == 3 && v.header.width == 2 && v.header.height == 2 &&
            v.header.depth == 1 && v.header.frames == 1;
  uint8_t *out = NULL;
  uint32_t w = 0, h = 0;
  if (ok)
    ok = video_to_image(&v, &out, &w, &h);
  if (ok)
    ok = w == 2 && h == 2 && memcmp(rgb, out, sizeof rgb) == 0;
  free(out);
  free_splat4DVideo(&v);
  return ok;
}

static bool test_image_through_file(void) {
  uint8_t rgb[12] = {10, 20, 30, 40, 50, 60, 10, 20, 30, 200, 100, 50};
  Splat4DVideo v;
  if (!image_to_video(rgb, 2, 2, &v))
    return false;

  FILE *fp = tmpfile();
  if (!fp) {
    free_splat4DVideo(&v);
    return false;
  }
  bool ok = write_splat4DVideo(fp, &v);
  free_splat4DVideo(&v);
  if (!ok) {
    fclose(fp);
    return false;
  }
  rewind(fp);
  Splat4DVideo loaded;
  ok = read_splat4DVideo(fp, &loaded);
  fclose(fp);
  if (!ok)
    return false;

  uint8_t *out = NULL;
  uint32_t w = 0, h = 0;
  ok = video_to_image(&loaded, &out, &w, &h) && w == 2 && h == 2 &&
       memcmp(rgb, out, sizeof rgb) == 0;
  free(out);
  free_splat4DVideo(&loaded);
  return ok;
}

static bool test_image_decode_rejects_non_2d(void) {
  uint8_t rgb[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
  Splat4DVideo v;
  if (!image_to_video(rgb, 2, 2, &v))
    return false;
  v.header.depth = 2; // pretend the video is volumetric
  uint8_t *out = NULL;
  bool rejected = !video_to_image(&v, &out, NULL, NULL);
  free(out);
  free_splat4DVideo(&v);
  return rejected;
}

static bool test_palette_entry_disk_bytes_by_shape(void) {
  uint32_t f32 = SPLAT_FLAG_PRECISION_FLOAT32;
  return palette_entry_disk_bytes(f32 | (SPLAT_SHAPE_ISOTROPIC << SPLAT_FLAG_SPLAT_SHAPE_SHIFT)) ==
             40 &&
         palette_entry_disk_bytes(
             f32 | (SPLAT_SHAPE_AXIS_ALIGNED << SPLAT_FLAG_SPLAT_SHAPE_SHIFT)) == 48 &&
         palette_entry_disk_bytes(
             f32 | (SPLAT_SHAPE_FULL_COVARIANCE << SPLAT_FLAG_SPLAT_SHAPE_SHIFT)) == 60;
}

// Round-trip a single splat through a file at the given shape.
static bool round_trip_shape(uint32_t shape, Splat4D in, Splat4D *out) {
  Splat4D pal[1] = {in};
  uint64_t idx[1] = {0};
  Splat4DHeader h = create_splat4DHeader(
      1, 1, 1, 1, 1, SPLAT_FLAG_PRECISION_FLOAT32 | (shape << SPLAT_FLAG_SPLAT_SHAPE_SHIFT));
  Splat4DVideo v = create_splat4DVideo(h, pal, idx); // borrows the stack arrays
  FILE *fp = tmpfile();
  if (!fp)
    return false;
  bool ok = write_splat4DVideo(fp, &v);
  if (ok) {
    rewind(fp);
    Splat4DVideo loaded;
    ok = read_splat4DVideo(fp, &loaded);
    if (ok) {
      *out = loaded.palette.palette[0];
      free_splat4DVideo(&loaded);
    }
  }
  fclose(fp);
  return ok;
}

static bool test_shape_isotropic_collapses_sigmas(void) {
  Splat4D in = create_splat4D(1, 7, 2, 8, 3, 9, 4, 5, 0.1f, 0.2f, 0.3f, 1.0f);
  Splat4D out;
  if (!round_trip_shape(SPLAT_SHAPE_ISOTROPIC, in, &out))
    return false;
  // One shared sigma (sigma_x = 7) is stored and expands to all three axes.
  return out.sigma_x == 7.0f && out.sigma_y == 7.0f && out.sigma_z == 7.0f && out.mu_x == 1.0f &&
         out.mu_y == 2.0f && out.mu_z == 3.0f && out.mu_t == 4.0f && out.sigma_t == 5.0f &&
         out.r == 0.1f && out.alpha == 1.0f && out.sigma_xy == 0.0f && out.sigma_xz == 0.0f &&
         out.sigma_yz == 0.0f;
}

static bool test_shape_axis_aligned_round_trip(void) {
  Splat4D in = create_splat4D(1, 7, 2, 8, 3, 9, 4, 5, 0.1f, 0.2f, 0.3f, 1.0f);
  Splat4D out;
  if (!round_trip_shape(SPLAT_SHAPE_AXIS_ALIGNED, in, &out))
    return false;
  return out.sigma_x == 7.0f && out.sigma_y == 8.0f && out.sigma_z == 9.0f &&
         out.sigma_xy == 0.0f && out.sigma_xz == 0.0f && out.sigma_yz == 0.0f && out.mu_z == 3.0f &&
         out.b == 0.3f;
}

static bool test_shape_full_covariance_round_trip(void) {
  Splat4D in = create_splat4D(1, 7, 2, 8, 3, 9, 4, 5, 0.1f, 0.2f, 0.3f, 1.0f);
  in.sigma_xy = 0.25f;
  in.sigma_xz = 0.5f;
  in.sigma_yz = 0.75f;
  Splat4D out;
  if (!round_trip_shape(SPLAT_SHAPE_FULL_COVARIANCE, in, &out))
    return false;
  return out.sigma_x == 7.0f && out.sigma_y == 8.0f && out.sigma_z == 9.0f &&
         out.sigma_xy == 0.25f && out.sigma_xz == 0.5f && out.sigma_yz == 0.75f;
}

static bool test_video_round_trip_shared_palette(void) {
  // Two 2x1 frames; green is shared, so the global palette has 3 colors, not 4.
  uint8_t f0[6] = {255, 0, 0, 0, 255, 0}; // red, green
  uint8_t f1[6] = {0, 255, 0, 0, 0, 255}; // green, blue
  const uint8_t *frames[2] = {f0, f1};
  Splat4DVideo v;
  if (!frames_to_video(frames, 2, 2, 1, &v))
    return false;

  bool ok = v.header.pSize == 3 && v.header.frames == 2 && v.header.width == 2 &&
            v.header.height == 1 && v.header.depth == 1;
  uint8_t **rec = NULL;
  uint32_t nf = 0, w = 0, h = 0;
  if (ok)
    ok = video_to_frames(&v, &rec, &nf, &w, &h);
  if (ok)
    ok = nf == 2 && w == 2 && h == 1 && memcmp(rec[0], f0, 6) == 0 && memcmp(rec[1], f1, 6) == 0;
  if (rec) {
    for (uint32_t t = 0; t < nf; ++t)
      free(rec[t]);
    free(rec);
  }
  free_splat4DVideo(&v);
  return ok;
}

static bool test_video_through_file(void) {
  uint8_t f0[6] = {10, 20, 30, 40, 50, 60};
  uint8_t f1[6] = {40, 50, 60, 70, 80, 90};
  const uint8_t *frames[2] = {f0, f1};
  Splat4DVideo v;
  if (!frames_to_video(frames, 2, 2, 1, &v))
    return false;

  FILE *fp = tmpfile();
  if (!fp) {
    free_splat4DVideo(&v);
    return false;
  }
  bool ok = write_splat4DVideo(fp, &v);
  free_splat4DVideo(&v);
  if (!ok) {
    fclose(fp);
    return false;
  }
  rewind(fp);
  Splat4DVideo loaded;
  ok = read_splat4DVideo(fp, &loaded);
  fclose(fp);
  if (!ok)
    return false;

  uint8_t **rec = NULL;
  uint32_t nf = 0, w = 0, h = 0;
  ok = video_to_frames(&loaded, &rec, &nf, &w, &h) && nf == 2 && memcmp(rec[0], f0, 6) == 0 &&
       memcmp(rec[1], f1, 6) == 0;
  if (rec) {
    for (uint32_t t = 0; t < nf; ++t)
      free(rec[t]);
    free(rec);
  }
  free_splat4DVideo(&loaded);
  return ok;
}

// image_to_video is exactly the single-frame collapse of the video codec.
static bool test_image_is_single_frame_video(void) {
  uint8_t rgb[6] = {1, 2, 3, 200, 100, 50};
  Splat4DVideo vi, vv;
  if (!image_to_video(rgb, 2, 1, &vi))
    return false;
  const uint8_t *frames[1] = {rgb};
  if (!frames_to_video(frames, 1, 2, 1, &vv)) {
    free_splat4DVideo(&vi);
    return false;
  }
  bool ok = vi.header.frames == 1 && vv.header.frames == 1 && vi.header.pSize == vv.header.pSize &&
            vi.footer.checksum == vv.footer.checksum;
  free_splat4DVideo(&vi);
  free_splat4DVideo(&vv);
  return ok;
}

// Frozen conformance vector: the exact v0.2 on-disk bytes for a fixed 2x1x1x1,
// 2-entry axis-aligned float32 video. All float values are exactly representable
// in binary32, so these bytes are platform-independent. Any change to the
// serialization (field order, sizes, markers, endianness) breaks this test.
static const uint8_t GOLDEN_4SPL[] = {
    0x34, 0x53, 0x50, 0x4C, 0x01, 0x01, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, // magic,ver,width
    0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, // height,depth,frames
    0x02, 0x00, 0x00, 0x00, 0x04, 0x04, 0x00, 0x00,                         // pSize, flags=0x0404
    0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0xA0, 0x40, // mu_x,mu_y,mu_z
    0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0xC0, 0x40, // sigma_x,y,z
    0x00, 0x00, 0xE0, 0x40, 0x00, 0x00, 0x00, 0x41,                         // mu_t, sigma_t
    0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x80, 0x3E, 0x00, 0x00, 0x40, 0x3F, // r,g,b
    0x00, 0x00, 0x80, 0x3F,                                                 // alpha (entry 0 end)
    0x00, 0x00, 0x10, 0x41, 0x00, 0x00, 0x30, 0x41, 0x00, 0x00, 0x50, 0x41, // entry 1 mu_x,y,z
    0x00, 0x00, 0x20, 0x41, 0x00, 0x00, 0x40, 0x41, 0x00, 0x00, 0x60, 0x41, // sigma_x,y,z
    0x00, 0x00, 0x70, 0x41, 0x00, 0x00, 0x80, 0x41,                         // mu_t, sigma_t
    0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0xC0, 0x3E, 0x00, 0x00, 0x20, 0x3F, // r,g,b
    0x00, 0x00, 0x60, 0x3F,                                                 // alpha (entry 1 end)
    0x00, 0x01,                                                             // index: 0, 1
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                         // footer idxoffset=128
    0x0F, 0x84, 0xC3, 0xAD,                                                 // checksum
    0x4C, 0x50, 0x53, 0x34,                                                 // "LPS4"
};

static bool test_golden_conformance_vector(void) {
  Splat4D pal[2];
  pal[0] = create_splat4D(1, 2, 3, 4, 5, 6, 7, 8, 0.5f, 0.25f, 0.75f, 1.0f);
  pal[1] = create_splat4D(9, 10, 11, 12, 13, 14, 15, 16, 0.125f, 0.375f, 0.625f, 0.875f);
  uint64_t idx[2] = {0, 1};
  Splat4DHeader h = create_splat4DHeader(
      2, 1, 1, 1, 2,
      SPLAT_FLAG_PRECISION_FLOAT32 | (SPLAT_SHAPE_AXIS_ALIGNED << SPLAT_FLAG_SPLAT_SHAPE_SHIFT));
  Splat4DVideo v = create_splat4DVideo(h, pal, idx);

  FILE *fp = tmpfile();
  if (!fp)
    return false;
  bool ok = write_splat4DVideo(fp, &v);
  if (ok) {
    fflush(fp);
    fseek(fp, 0, SEEK_END);
    long n = ftell(fp);
    rewind(fp);
    uint8_t buf[sizeof GOLDEN_4SPL];
    ok = n == (long)sizeof GOLDEN_4SPL && fread(buf, 1, sizeof buf, fp) == sizeof buf &&
         memcmp(buf, GOLDEN_4SPL, sizeof buf) == 0;
  }
  fclose(fp);
  return ok;
}

// The frozen vector also reads back to the exact field values it encodes.
static bool test_golden_vector_reads_back(void) {
  FILE *fp = tmpfile();
  if (!fp)
    return false;
  fwrite(GOLDEN_4SPL, 1, sizeof GOLDEN_4SPL, fp);
  rewind(fp);
  Splat4DVideo v;
  bool ok = read_splat4DVideo(fp, &v);
  fclose(fp);
  if (!ok)
    return false;
  const Splat4D *s = &v.palette.palette[0];
  ok = v.header.width == 2 && v.header.pSize == 2 && v.header.version[1] == 1 && s->mu_x == 1.0f &&
       s->mu_y == 3.0f && s->sigma_x == 2.0f && s->sigma_z == 6.0f && s->r == 0.5f &&
       s->alpha == 1.0f && v.index.index[1] == 1;
  free_splat4DVideo(&v);
  return ok;
}

static bool test_quantize_reduces_palette(void) {
  // Four distinct grays quantized down to two representatives.
  uint8_t rgb[12] = {0, 0, 0, 80, 80, 80, 160, 160, 160, 255, 255, 255};
  const uint8_t *fr[1] = {rgb};
  Splat4DVideo v;
  if (!frames_to_video_quantized(fr, 1, 4, 1, 2, &v))
    return false;
  bool ok = v.header.pSize >= 1 && v.header.pSize <= 2;
  uint8_t **rec = NULL;
  uint32_t nf = 0, w = 0, h = 0;
  if (ok)
    ok = video_to_frames(&v, &rec, &nf, &w, &h) && nf == 1 && w == 4 && h == 1;
  if (rec) {
    for (uint32_t t = 0; t < nf; ++t)
      free(rec[t]);
    free(rec);
  }
  free_splat4DVideo(&v);
  return ok;
}

static bool test_quantize_passthrough_within_budget(void) {
  // A budget >= the distinct-color count keeps the palette exact (lossless).
  uint8_t rgb[12] = {0, 0, 0, 80, 80, 80, 160, 160, 160, 255, 255, 255};
  const uint8_t *fr[1] = {rgb};
  Splat4DVideo v;
  if (!frames_to_video_quantized(fr, 1, 4, 1, 100, &v))
    return false;
  bool ok = v.header.pSize == 4;
  uint8_t **rec = NULL;
  uint32_t nf = 0, w = 0, h = 0;
  if (ok)
    ok = video_to_frames(&v, &rec, &nf, &w, &h) && nf == 1 && memcmp(rec[0], rgb, 12) == 0;
  if (rec) {
    for (uint32_t t = 0; t < nf; ++t)
      free(rec[t]);
    free(rec);
  }
  free_splat4DVideo(&v);
  return ok;
}

static bool test_volume_round_trip(void) {
  // Two 2x1 slices; distinct colors across the stack -> palette of 3.
  uint8_t s0[6] = {255, 0, 0, 0, 255, 0}; // red, green (z=0)
  uint8_t s1[6] = {0, 0, 255, 0, 0, 255}; // blue, blue (z=1)
  const uint8_t *slices[2] = {s0, s1};
  Splat4DVideo v;
  if (!stack_to_video_quantized(slices, 2, 1, 2, 1, 0, &v))
    return false;

  bool ok = v.header.depth == 2 && v.header.frames == 1 && v.header.width == 2 &&
            v.header.height == 1 && v.header.pSize == 3;
  uint8_t **rec = NULL;
  uint32_t ns = 0, w = 0, h = 0;
  if (ok)
    ok = video_to_slices(&v, &rec, &ns, &w, &h);
  if (ok)
    ok = ns == 2 && memcmp(rec[0], s0, 6) == 0 && memcmp(rec[1], s1, 6) == 0;
  if (rec) {
    for (uint32_t s = 0; s < ns; ++s)
      free(rec[s]);
    free(rec);
  }
  free_splat4DVideo(&v);
  return ok;
}

static bool test_volume_populates_mu_z(void) {
  // 1x1 stack: red in slice z=0, green in slice z=1.
  uint8_t s0[3] = {255, 0, 0};
  uint8_t s1[3] = {0, 255, 0};
  const uint8_t *slices[2] = {s0, s1};
  Splat4DVideo v;
  if (!stack_to_video_quantized(slices, 2, 1, 1, 1, 0, &v))
    return false;
  // pass 1 sees slice 0 first, so red is palette[0] (z=0), green palette[1] (z=1).
  bool ok = v.header.depth == 2 && v.header.pSize == 2 && v.palette.palette[0].mu_z == 0.0f &&
            v.palette.palette[1].mu_z == 1.0f && v.palette.palette[0].mu_t == 0.0f;
  free_splat4DVideo(&v);
  return ok;
}

static test_case TESTS[] = {
    {"header_total_indices_checked", test_header_total_indices_checked},
    {"create_splat4D", test_create_splat4D},
    {"create_splat4D_zero_values", test_create_splat4D_zero_values},
    {"crc32_known_value", test_crc32_known_value},
    {"checksum_matches_footer", test_compute_video_checksum_matches_footer},
    {"idxoffset_helpers_agree", test_idxoffset_helpers_agree},
    {"idxoffset_forward_handles_large_palette", test_idxoffset_forward_handles_large_palette},
    {"idxoffset_reverse_handles_large_files", test_idxoffset_reverse_handles_large_files},
    {"check_index_values_accepts_valid", test_check_index_values_accepts_valid},
    {"check_index_values_rejects_out_of_range", test_check_index_values_rejects_out_of_range},
    {"check_index_values_detects_too_wide", test_check_index_values_detects_too_wide},
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
    {"read_header_fails_on_short_file", test_read_header_fails_on_short_file},
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
    {"read_video_rejects_bad_magic", test_read_video_rejects_bad_magic},
    {"read_video_rejects_bad_version", test_read_video_rejects_bad_version},
    {"idxoffset_sanity_mismatch", test_idxoffset_sanity_mismatch},
    {"header_defaults_to_float32_precision", test_header_defaults_to_float32_precision},
    {"stream_splat4DVideo_success", test_stream_splat4DVideo_success},
    {"stream_splat4DVideo_rejects_null", test_stream_splat4DVideo_rejects_null},
    {"stream_splat4DVideo_stops_on_consumer_error",
     test_stream_splat4DVideo_stops_on_consumer_error},
    {"round_trip_index_width", test_round_trip_index_width},
    {"round_trip_preserves_descriptive_flags", test_round_trip_preserves_descriptive_flags},
    {"read_video_rejects_reserved_splat_shape", test_read_video_rejects_reserved_splat_shape},
    {"read_video_rejects_encryption", test_read_video_rejects_encryption},
    {"image_round_trip", test_image_round_trip},
    {"image_through_file", test_image_through_file},
    {"image_decode_rejects_non_2d", test_image_decode_rejects_non_2d},
    {"video_round_trip_shared_palette", test_video_round_trip_shared_palette},
    {"video_through_file", test_video_through_file},
    {"image_is_single_frame_video", test_image_is_single_frame_video},
    {"quantize_reduces_palette", test_quantize_reduces_palette},
    {"quantize_passthrough_within_budget", test_quantize_passthrough_within_budget},
    {"volume_round_trip", test_volume_round_trip},
    {"volume_populates_mu_z", test_volume_populates_mu_z},
    {"golden_conformance_vector", test_golden_conformance_vector},
    {"golden_vector_reads_back", test_golden_vector_reads_back},
    {"palette_entry_disk_bytes_by_shape", test_palette_entry_disk_bytes_by_shape},
    {"shape_isotropic_collapses_sigmas", test_shape_isotropic_collapses_sigmas},
    {"shape_axis_aligned_round_trip", test_shape_axis_aligned_round_trip},
    {"shape_full_covariance_round_trip", test_shape_full_covariance_round_trip},
    {"half_conversion_exact_values", test_half_conversion_exact_values},
    {"round_trip_float64_palette", test_round_trip_float64_palette},
    {"round_trip_float16_palette", test_round_trip_float16_palette},
    {"rle_unit_roundtrip", test_rle_unit_roundtrip},
    {"round_trip_all_available_codecs", test_round_trip_all_available_codecs},
    {"read_video_rejects_unavailable_codec", test_read_video_rejects_unavailable_codec},
#ifdef SPLAT_WITH_LCMS2
    {"color_convert_round_trip", test_color_convert_round_trip},
    {"oklab_round_trip", test_oklab_round_trip},
    {"oklab_pivots_to_lcms_space", test_oklab_pivots_to_lcms_space},
    {"color_convert_identity", test_color_convert_identity},
    {"color_convert_rejects_unsupported", test_color_convert_rejects_unsupported},
#endif
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
