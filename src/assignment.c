#include "assignment.h"

#include <math.h>
#include <stddef.h>

unsigned bitset_symmetric_difference_count(uint32_t a, uint32_t b) {
    uint32_t c = a^b;
    int count = 0;
    while(c) {
        count += c & 1;
        c >>= 1;
    }
    return count;
}

uint32_t bit_range_mask(unsigned low, unsigned high) {
    int width = high - low + 1;
    if (width == 32) {
        return ~0U;
    }
    uint32_t mask = 1U << width;
    mask--;
    mask <<= low;
    return mask;
}

uint32_t splice_bits(
    uint32_t destination,
    uint32_t source,
    unsigned low,
    unsigned high
) {
    int width = high - low + 1;
    if (width == 32) {
        return source;
    }
    uint32_t clear = ~0U;
    clear <<= width;
    clear <<= low;
    clear = clear | ((1U << low) -1 );
    uint32_t result = destination & clear;
    uint32_t shift = 0U | ((1U << width) -1 );
    shift = shift & source;
    shift <<= low;
    result = result | shift;
    return result;
}

int32_t decode_twos_complement(uint32_t bits, unsigned width) {
    uint32_t bitsToKeep = 0U;
    if (width != 32) {
        bitsToKeep = bits & ((1U << width) - 1);
    } else {
        bitsToKeep = bits;
    }
    int32_t result = 0;
    int32_t value = 1;
    for (unsigned i = 0; i < width; i++) {
        if (i == width-1) {
            value *= -1;
        }
        if (bitsToKeep & 1) {
            result += value;
        }
        value *=2;
        bitsToKeep >>= 1;
    }
    return result;
}

bool add_signed_width(
    uint32_t a_bits,
    uint32_t b_bits,
    unsigned width,
    uint32_t *sum_bits
) {
    uint32_t result = 0;
    uint32_t a_bits_sign = 0;
    uint32_t b_bits_sign = 0;
    uint32_t result_bits_sign = 0;
    if (width == 32) {
        result = a_bits + b_bits;
        *sum_bits = result;
        a_bits_sign = a_bits & (1U << (width-1));
        b_bits_sign = b_bits & (1U << (width-1));
        result_bits_sign = result & (1U << (width-1));
    } else {
        uint32_t a_bits_width = splice_bits(0U, a_bits, 0, width-1);
        uint32_t b_bits_width = splice_bits(0U, b_bits, 0, width-1);
        result = a_bits_width + b_bits_width;
        result = splice_bits(0U, result, 0, width-1);
        result_bits_sign = result & (1U << (width-1));
        *sum_bits = result;
        a_bits_sign = a_bits_width & (1U << (width-1));
        b_bits_sign = b_bits_width & (1U << (width-1));
    }
    bool valid = false;
    if (a_bits_sign == b_bits_sign && a_bits_sign != result_bits_sign) valid = true;
    return valid;
}

int compare_signed_unsigned_math(
    int32_t signed_value,
    uint32_t unsigned_value
) {
    if (signed_value < 0) {
        return -1;
    } else {
        uint32_t unisgned_signed = (uint32_t) signed_value;
        if (unsigned_value == unisgned_signed) return 0;
        if (unsigned_value > unisgned_signed) return -1;
        else return 1;
    }
}

uint32_t multiply_by_24(uint32_t x) {
    uint32_t int_32_x = x << 5;
    uint32_t int_8_x = x << 3;

    return int_32_x - int_8_x;
}

fp32_info_t fp32_decode(uint32_t bits) {
    (void)bits;

    fp32_info_t info = {
        .negative = false,
        .kind = FP32_ZERO,
        .exponent_bits = 0,
        .fraction_bits = 0,
        .unbiased_exponent = -126,
    };
    return info;
}

double fp32_exact_value(uint32_t bits) {
    (void)bits;
    return 0.0;
}

uint32_t fp32_next_up_bits(uint32_t bits) {
    (void)bits;
    return 0;
}

bool fp32_truncate_to_int(uint32_t bits, int32_t *out) {
    (void)bits;

    if (out != NULL) {
        *out = 0;
    }
    return false;
}
