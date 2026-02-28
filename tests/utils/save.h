#include <srp/srp.h>

/** Saves the contents of the framebuffer to a PNG image, ignoring the alpha
 *  value (setting everything opaque)
 *  @param[in] fb Pointer to the framebuffer
 *  @param[in] outputPath Path to the PNG image where the result will be stored.
 *                        Created if nonexistent.
 *  @returns `stbi_image_write_png` code: 0 on success, non-zero otherwise */
int saveFramebufferToImage(const SRPFramebuffer* fb, const char* outputPath);
