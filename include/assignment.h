#ifndef COM2113_ASSIGNMENT_H
#define COM2113_ASSIGNMENT_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    FP32_ZERO,
    FP32_SUBNORMAL,
    FP32_NORMAL,
    FP32_INFINITY,
    FP32_NAN
} fp32_kind_t;

typedef struct {
    bool negative;
    fp32_kind_t kind;
    uint8_t exponent_bits;
    uint32_t fraction_bits;
    int unbiased_exponent;
} fp32_info_t;

/* Problem 1 */
unsigned bitset_symmetric_difference_count(uint32_t a, uint32_t b);

/* Problem 2 */
uint32_t bit_range_mask(unsigned low, unsigned high);
uint32_t splice_bits(
    uint32_t destination,
    uint32_t source,
    unsigned low,
    unsigned high
);

/* Problem 3 */
int32_t decode_twos_complement(uint32_t bits, unsigned width);

/* Problem 4 */
bool add_signed_width(
    uint32_t a_bits,
    uint32_t b_bits,
    unsigned width,
    uint32_t *sum_bits
);

/* Problem 5 */
int compare_signed_unsigned_math(
    int32_t signed_value,
    uint32_t unsigned_value
);

/* Problem 6 */
uint32_t multiply_by_24(uint32_t x);

/* Problem 7 */
fp32_info_t fp32_decode(uint32_t bits);
double fp32_exact_value(uint32_t bits);

/* Problem 8 */
uint32_t fp32_next_up_bits(uint32_t bits);

/* Problem 9 */
bool fp32_truncate_to_int(uint32_t bits, int32_t *out);

#endif
