
#ifndef _ART_LEB128_H_
#define _ART_LEB128_H_


#include "globals.h"


// Reads an unsigned LEB128 value, updating the given pointer to point
// just past the end of the read value. This function tolerates
// non-zero high-order bits in the fifth encoded byte.
static inline uint32_t DecodeUnsignedLeb128(const uint8_t** data)
{
    const uint8_t* ptr = *data;
    int result = *(ptr++);
    if (UNLIKELY(result > 0x7f)) {
        int cur = *(ptr++);
        result = (result & 0x7f) | ((cur & 0x7f) << 7);
        if (cur > 0x7f) {
            cur = *(ptr++);
            result |= (cur & 0x7f) << 14;
            if (cur > 0x7f) {
                cur = *(ptr++);
                result |= (cur & 0x7f) << 21;
                if (cur > 0x7f) {
                    // Note: We don't check to see if cur is out of range here,
                    // meaning we tolerate garbage in the four high-order bits.
                    cur = *(ptr++);
                    result |= cur << 28;
                }
            }
        }
    }
    *data = ptr;
    return static_cast<uint32_t>(result);
}

// Reads an unsigned LEB128 + 1 value. updating the given pointer to point
// just past the end of the read value. This function tolerates
// non-zero high-order bits in the fifth encoded byte.
// It is possible for this function to return -1.
static inline int32_t DecodeUnsignedLeb128P1(const uint8_t** data)
{
    return DecodeUnsignedLeb128(data) - 1;
}

// Reads a signed LEB128 value, updating the given pointer to point
// just past the end of the read value. This function tolerates
// non-zero high-order bits in the fifth encoded byte.
static inline int32_t DecodeSignedLeb128(const uint8_t** data)
{
    const uint8_t* ptr = *data;
    int32_t result = *(ptr++);
    if (result <= 0x7f)
        result = (result << 25) >> 25;
    else {
        int cur = *(ptr++);
        result = (result & 0x7f) | ((cur & 0x7f) << 7);
        if (cur <= 0x7f)
            result = (result << 18) >> 18;
        else {
            cur = *(ptr++);
            result |= (cur & 0x7f) << 14;
            if (cur <= 0x7f)
                result = (result << 11) >> 11;
            else {
                cur = *(ptr++);
                result |= (cur & 0x7f) << 21;
                if (cur <= 0x7f)
                    result = (result << 4) >> 4;
                else {
                    // Note: We don't check to see if cur is out of range here,
                    // meaning we tolerate garbage in the four high-order bits.
                    cur = *(ptr++);
                    result |= cur << 28;
                }
            }
        }
    }
    *data = ptr;
    return result;
}

// Returns the number of bytes needed to encode the value in unsigned LEB128.
static inline uint32_t UnsignedLeb128Size(uint32_t data)
{
    // bits_to_encode = (data != 0) ? 32 - CLZ(x) : 1  // 32 - CLZ(data | 1)
    // bytes = ceil(bits_to_encode / 7.0);             // (6 + bits_to_encode) / 7
    uint32_t x = 6 + 32 - CLZ(data | 1);
    // Division by 7 is done by (x * 37) >> 8 where 37 = ceil(256 / 7).
    // This works for 0 <= x < 256 / (7 * 37 - 256), i.e. 0 <= x <= 85.
    return (x * 37) >> 8;
}

// Returns the number of bytes needed to encode the value in unsigned LEB128.
static inline uint32_t SignedLeb128Size(int32_t data)
{
    // Like UnsignedLeb128Size(), but we need one bit beyond the highest bit that differs from sign.
    data = data ^ (data >> 31);
    uint32_t x = 1 /* we need to encode the sign bit */ + 6 + 32 - CLZ(data | 1);
    return (x * 37) >> 8;
}

static inline uint8_t* EncodeUnsignedLeb128(uint8_t* dest, uint32_t value)
{
    uint8_t out = value & 0x7f;
    value >>= 7;
    while (value != 0) {
        *dest++ = out | 0x80;
        out = value & 0x7f;
        value >>= 7;
    }
    *dest++ = out;
    return dest;
}

static inline uint8_t* EncodeSignedLeb128(uint8_t* dest, int32_t value)
{
    uint32_t extra_bits = static_cast<uint32_t>(value ^ (value >> 31)) >> 6;
    uint8_t out = value & 0x7f;
    while (extra_bits != 0u) {
        *dest++ = out | 0x80;
        value >>= 7;
        out = value & 0x7f;
        extra_bits >>= 7;
    }
    *dest++ = out;
    return dest;
}

#endif
