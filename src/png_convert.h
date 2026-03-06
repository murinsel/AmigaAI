#ifndef AMIGAAI_PNG_CONVERT_H
#define AMIGAAI_PNG_CONVERT_H

/* Convert any DataTypes-loadable picture to PNG.
 * Uses DataTypes to load the image, ReadPixelArray8 for planar→chunky,
 * and a minimal PNG encoder with stored (uncompressed) deflate.
 * Requires dt_init() to have been called (datatypes.library open).
 * Returns 0 on success, -1 on error. */
int png_convert_file(const char *input_path, const char *output_path);

/* Close graphics.library opened by png_convert_file(). */
void png_convert_cleanup(void);

#endif /* AMIGAAI_PNG_CONVERT_H */
