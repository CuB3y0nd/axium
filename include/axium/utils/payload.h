#ifndef AXIUM_PAYLOAD_H
#define AXIUM_PAYLOAD_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Patches a buffer by replacing occurrences of a marker with new data.
 *
 * This function performs an in-place replacement. If the replacement is shorter
 * than the marker, the remaining marker bytes are zero-filled.
 *
 * @param buf The buffer to patch (restrict).
 * @param buf_size Size of the buffer.
 * @param marker The data to search for (restrict).
 * @param marker_size Size of the marker.
 * @param replacement The data to write in place of the marker (restrict).
 * @param replacement_size Size of the replacement data.
 */
void patch(uint8_t *restrict buf, size_t buf_size, const void *restrict marker,
           size_t marker_size, const void *restrict replacement,
           size_t replacement_size) __attribute__((nonnull(1, 3, 5)))
__attribute__((leaf));

#endif // AXIUM_PAYLOAD_H
