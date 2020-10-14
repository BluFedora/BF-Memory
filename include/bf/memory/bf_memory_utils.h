/******************************************************************************/
/*!
 * @file   bf_memory_utils.h
 * @author Shareef Abdoul-Raheem (http://blufedora.github.io/)
 * @brief 
 *   Contains functions useful for low level memory manipulations.
 *
 * @version 0.0.1
 * @date    2020-03-22
 * 
 * @copyright Copyright (c) 2020
 */
/******************************************************************************/
#ifndef BIFROST_MEMORY_UTILS_HPP
#define BIFROST_MEMORY_UTILS_HPP

#include <stddef.h> /* size_t                                                                   */
#include <stdint.h> /* uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t */

#if __cplusplus
extern "C" {
#endif

// clang-format off
#define bfBytes(n)     (n)
#define bfKilobytes(n) (bfBytes(n) * 1024)
#define bfMegabytes(n) (bfKilobytes(n) * 1024)
#define bfGigabytes(n) (bfMegabytes(n) * 1024)
// clang-format on

typedef unsigned char bfByte; /*!< The represents a single byte. */

/*!
 * @brief 
 *   Aligns size to required_alignment.
 *
 * @param size
 *   The potentially unaligned size of an object.
 * 
 * @param required_alignment 
 *   Must be a non zero power of two.
 *
 * @return size_t 
 *   The size of the object for the required alignment,
 */
size_t bfAlignUpSize(size_t size, size_t required_alignment);

void* bfAlignUpPointer(const void* ptr, size_t required_alignment);

/* Implements "std::align" but in C. */
void* bfStdAlign(size_t alignment, size_t size, void** ptr, size_t* space);

/*
  (Little / Big) Endian Byte Helpers
    The signed versions of these functions assume 2s compliment.
*/

uint8_t  bfBytesReadUint8LE(const bfByte* bytes);
uint16_t bfBytesReadUint16LE(const bfByte* bytes);
uint32_t bfBytesReadUint32LE(const bfByte* bytes);
uint64_t bfBytesReadUint64LE(const bfByte* bytes);
uint8_t  bfBytesReadUint8BE(const bfByte* bytes);
uint16_t bfBytesReadUint16BE(const bfByte* bytes);
uint32_t bfBytesReadUint32BE(const bfByte* bytes);
uint64_t bfBytesReadUint64BE(const bfByte* bytes);
int8_t   bfBytesReadInt8LE(const bfByte* bytes);
int16_t  bfBytesReadInt16LE(const bfByte* bytes);
int32_t  bfBytesReadInt32LE(const bfByte* bytes);
int64_t  bfBytesReadInt64LE(const bfByte* bytes);
int8_t   bfBytesReadInt8BE(const bfByte* bytes);
int16_t  bfBytesReadInt16BE(const bfByte* bytes);
int32_t  bfBytesReadInt32BE(const bfByte* bytes);
int64_t  bfBytesReadInt64BE(const bfByte* bytes);

void bfBytesWriteUint8LE(bfByte* bytes, uint8_t value);
void bfBytesWriteUint16LE(bfByte* bytes, uint16_t value);
void bfBytesWriteUint32LE(bfByte* bytes, uint32_t value);
void bfBytesWriteUint64LE(bfByte* bytes, uint64_t value);
void bfBytesWriteUint8BE(bfByte* bytes, uint8_t value);
void bfBytesWriteUint16BE(bfByte* bytes, uint16_t value);
void bfBytesWriteUint32BE(bfByte* bytes, uint32_t value);
void bfBytesWriteUint64BE(bfByte* bytes, uint64_t value);
void bfBytesWriteInt8LE(bfByte* bytes, int8_t value);
void bfBytesWriteInt16LE(bfByte* bytes, int16_t value);
void bfBytesWriteInt32LE(bfByte* bytes, int32_t value);
void bfBytesWriteInt64LE(bfByte* bytes, int64_t value);
void bfBytesWriteInt8BE(bfByte* bytes, int8_t value);
void bfBytesWriteInt16BE(bfByte* bytes, int16_t value);
void bfBytesWriteInt32BE(bfByte* bytes, int32_t value);
void bfBytesWriteInt64BE(bfByte* bytes, int64_t value);

#if __cplusplus
}
#endif

#endif /* BIFROST_MEMORY_UTILS_HPP */
