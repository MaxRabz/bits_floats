#include "assignment.h"

#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned checks = 0;
static unsigned failures = 0;
static const char *current_test = "";

#define CHECK(condition)                                                      \
    do {                                                                      \
        checks++;                                                             \
        if (!(condition)) {                                                   \
            failures++;                                                       \
            fprintf(                                                          \
                stderr,                                                       \
                "FAIL [%s] %s:%d: %s\n",                                      \
                current_test,                                                 \
                __FILE__,                                                     \
                __LINE__,                                                     \
                #condition                                                    \
            );                                                                \
        }                                                                     \
    } while (0)

#define CHECK_U32(actual, expected)                                            \
    do {                                                                      \
        uint32_t actual_value = (actual);                                      \
        uint32_t expected_value = (expected);                                  \
        checks++;                                                             \
        if (actual_value != expected_value) {                                  \
            failures++;                                                       \
            fprintf(                                                          \
                stderr,                                                       \
                "FAIL [%s] %s:%d: got 0x%08" PRIX32                           \
                ", expected 0x%08" PRIX32 "\n",                               \
                current_test,                                                 \
                __FILE__,                                                     \
                __LINE__,                                                     \
                actual_value,                                                 \
                expected_value                                                \
            );                                                                \
        }                                                                     \
    } while (0)

#define CHECK_I32(actual, expected)                                            \
    do {                                                                      \
        int32_t actual_value = (actual);                                       \
        int32_t expected_value = (expected);                                   \
        checks++;                                                             \
        if (actual_value != expected_value) {                                  \
            failures++;                                                       \
            fprintf(                                                          \
                stderr,                                                       \
                "FAIL [%s] %s:%d: got %" PRId32                               \
                ", expected %" PRId32 "\n",                                   \
                current_test,                                                 \
                __FILE__,                                                     \
                __LINE__,                                                     \
                actual_value,                                                 \
                expected_value                                                \
            );                                                                \
        }                                                                     \
    } while (0)

static uint32_t rng_state = UINT32_C(0xC02113A5);

static uint32_t next_random_u32(void) {
    uint32_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng_state = x;
    return x;
}

static unsigned oracle_popcount(uint32_t value) {
    unsigned count = 0;
    for (unsigned i = 0; i < 32; i++) {
        count += (unsigned)((value >> i) & UINT32_C(1));
    }
    return count;
}

static uint32_t oracle_mask(unsigned low, unsigned high) {
    uint64_t width = (uint64_t)high - (uint64_t)low + UINT64_C(1);
    uint64_t low_bits =
        width == 32 ? UINT64_C(0xFFFFFFFF) : ((UINT64_C(1) << width) - 1);
    return (uint32_t)(low_bits << low);
}

static int32_t oracle_decode(uint32_t bits, unsigned width) {
    uint64_t modulus = UINT64_C(1) << width;
    uint64_t mask = modulus - UINT64_C(1);
    uint64_t value = (uint64_t)bits & mask;
    uint64_t sign_bit = UINT64_C(1) << (width - 1);

    int64_t result =
        (value & sign_bit) == 0
            ? (int64_t)value
            : (int64_t)value - (int64_t)modulus;

    return (int32_t)result;
}

static float bits_to_float(uint32_t bits) {
    float value;
    memcpy(&value, &bits, sizeof value);
    return value;
}

static uint32_t float_to_bits(float value) {
    uint32_t bits;
    memcpy(&bits, &value, sizeof bits);
    return bits;
}

static void test_problem_1(void) {
    current_test = "problem 1: symmetric difference";

    CHECK(
        bitset_symmetric_difference_count(
            UINT32_C(0x69),
            UINT32_C(0x55)
        ) == 4
    );
    CHECK(
        bitset_symmetric_difference_count(
            UINT32_C(0),
            UINT32_C(0)
        ) == 0
    );
    CHECK(
        bitset_symmetric_difference_count(
            UINT32_MAX,
            UINT32_C(0)
        ) == 32
    );
    CHECK(
        bitset_symmetric_difference_count(
            UINT32_MAX,
            UINT32_MAX
        ) == 0
    );

    for (unsigned i = 0; i < 10000; i++) {
        uint32_t a = next_random_u32();
        uint32_t b = next_random_u32();
        unsigned expected = oracle_popcount(a ^ b);
        CHECK(bitset_symmetric_difference_count(a, b) == expected);
    }
}

static void test_problem_2(void) {
    current_test = "problem 2: masks and splicing";

    CHECK_U32(bit_range_mask(0, 0), UINT32_C(0x00000001));
    CHECK_U32(bit_range_mask(2, 5), UINT32_C(0x0000003C));
    CHECK_U32(bit_range_mask(31, 31), UINT32_C(0x80000000));
    CHECK_U32(bit_range_mask(0, 31), UINT32_MAX);
    CHECK_U32(bit_range_mask(16, 31), UINT32_C(0xFFFF0000));

    for (unsigned low = 0; low < 32; low++) {
        for (unsigned high = low; high < 32; high++) {
            CHECK_U32(bit_range_mask(low, high), oracle_mask(low, high));
        }
    }

    CHECK_U32(
        splice_bits(UINT32_C(0xA1), UINT32_C(0xE), 2, 5),
        UINT32_C(0xB9)
    );
    CHECK_U32(
        splice_bits(UINT32_C(0xAAAAAAAA), UINT32_C(0x55555555), 0, 31),
        UINT32_C(0x55555555)
    );
    CHECK_U32(
        splice_bits(UINT32_MAX, UINT32_C(0), 31, 31),
        UINT32_C(0x7FFFFFFF)
    );

    for (unsigned i = 0; i < 10000; i++) {
        unsigned low = next_random_u32() % 32;
        unsigned high = low + next_random_u32() % (32 - low);
        unsigned width = high - low + 1;
        uint32_t destination = next_random_u32();
        uint32_t source = next_random_u32();
        uint32_t mask = oracle_mask(low, high);

        uint64_t source_low_mask =
            width == 32
                ? UINT64_C(0xFFFFFFFF)
                : ((UINT64_C(1) << width) - UINT64_C(1));

        uint32_t expected =
            (destination & ~mask) |
            ((uint32_t)(((uint64_t)source & source_low_mask) << low) & mask);

        CHECK_U32(
            splice_bits(destination, source, low, high),
            expected
        );
    }
}

static void test_problem_3(void) {
    current_test = "problem 3: two's complement decoding";

    CHECK_I32(decode_twos_complement(UINT32_C(0x7F), 8), 127);
    CHECK_I32(decode_twos_complement(UINT32_C(0x80), 8), -128);
    CHECK_I32(decode_twos_complement(UINT32_C(0xEB), 8), -21);
    CHECK_I32(decode_twos_complement(UINT32_C(0xFFFFFFFD), 3), -3);
    CHECK_I32(decode_twos_complement(UINT32_C(0), 1), 0);
    CHECK_I32(decode_twos_complement(UINT32_C(1), 1), -1);
    CHECK_I32(
        decode_twos_complement(UINT32_C(0x80000000), 32),
        INT32_MIN
    );
    CHECK_I32(
        decode_twos_complement(UINT32_C(0x7FFFFFFF), 32),
        INT32_MAX
    );
    CHECK_I32(
        decode_twos_complement(UINT32_C(0xFFFFFFFF), 32),
        -1
    );

    for (unsigned width = 1; width <= 12; width++) {
        uint32_t limit = UINT32_C(1) << width;
        for (uint32_t bits = 0; bits < limit; bits++) {
            CHECK_I32(
                decode_twos_complement(bits, width),
                oracle_decode(bits, width)
            );

            CHECK_I32(
                decode_twos_complement(bits | UINT32_C(0xFFFFF000), width),
                oracle_decode(bits, width)
            );
        }
    }

    for (unsigned i = 0; i < 10000; i++) {
        uint32_t bits = next_random_u32();
        unsigned width = 13 + next_random_u32() % 20;
        CHECK_I32(
            decode_twos_complement(bits, width),
            oracle_decode(bits, width)
        );
    }
}

static void test_problem_4(void) {
    current_test = "problem 4: fixed-width signed addition";

    for (unsigned width = 2; width <= 8; width++) {
        uint32_t limit = UINT32_C(1) << width;
        int64_t minimum = -(INT64_C(1) << (width - 1));
        int64_t maximum = (INT64_C(1) << (width - 1)) - 1;

        for (uint32_t a = 0; a < limit; a++) {
            for (uint32_t b = 0; b < limit; b++) {
                uint32_t sum_bits = UINT32_C(0xDEADBEEF);
                bool actual_overflow =
                    add_signed_width(a, b, width, &sum_bits);

                int64_t decoded_a = oracle_decode(a, width);
                int64_t decoded_b = oracle_decode(b, width);
                int64_t mathematical_sum = decoded_a + decoded_b;
                bool expected_overflow =
                    mathematical_sum < minimum || mathematical_sum > maximum;
                uint32_t expected_sum = (a + b) & (limit - 1);

                CHECK_U32(sum_bits, expected_sum);
                CHECK(actual_overflow == expected_overflow);
            }
        }
    }

    const struct {
        uint32_t a;
        uint32_t b;
        unsigned width;
    } targeted[] = {
        {UINT32_C(0x7FFFFFFF), UINT32_C(1), 32},
        {UINT32_C(0x80000000), UINT32_C(0xFFFFFFFF), 32},
        {UINT32_C(0xFFFFFFFF), UINT32_C(1), 32},
        {UINT32_C(0x40000000), UINT32_C(0x40000000), 32},
        {UINT32_C(0x80000000), UINT32_C(0x80000000), 32},
    };

    for (size_t i = 0; i < sizeof targeted / sizeof targeted[0]; i++) {
        uint32_t sum_bits = 0;
        bool actual = add_signed_width(
            targeted[i].a,
            targeted[i].b,
            targeted[i].width,
            &sum_bits
        );

        int64_t a = oracle_decode(targeted[i].a, 32);
        int64_t b = oracle_decode(targeted[i].b, 32);
        int64_t sum = a + b;
        bool expected = sum < INT32_MIN || sum > INT32_MAX;

        CHECK_U32(sum_bits, targeted[i].a + targeted[i].b);
        CHECK(actual == expected);
    }

    for (unsigned i = 0; i < 10000; i++) {
        unsigned width = 9 + next_random_u32() % 24;
        uint32_t a = next_random_u32();
        uint32_t b = next_random_u32();
        uint32_t mask =
            width == 32
                ? UINT32_MAX
                : ((UINT32_C(1) << width) - UINT32_C(1));
        int64_t minimum = -(INT64_C(1) << (width - 1));
        int64_t maximum = (INT64_C(1) << (width - 1)) - 1;
        int64_t decoded_a = oracle_decode(a, width);
        int64_t decoded_b = oracle_decode(b, width);
        int64_t mathematical_sum = decoded_a + decoded_b;
        bool expected_overflow =
            mathematical_sum < minimum || mathematical_sum > maximum;
        uint32_t expected_sum = ((a & mask) + (b & mask)) & mask;
        uint32_t actual_sum = 0;
        bool actual_overflow =
            add_signed_width(a, b, width, &actual_sum);

        CHECK_U32(actual_sum, expected_sum);
        CHECK(actual_overflow == expected_overflow);
    }
}

static void test_problem_5(void) {
    current_test = "problem 5: mathematical mixed comparison";

    CHECK(compare_signed_unsigned_math(-1, UINT32_C(0)) == -1);
    CHECK(compare_signed_unsigned_math(INT32_MIN, UINT32_MAX) == -1);
    CHECK(compare_signed_unsigned_math(0, UINT32_C(0)) == 0);
    CHECK(compare_signed_unsigned_math(42, UINT32_C(42)) == 0);
    CHECK(compare_signed_unsigned_math(42, UINT32_C(10)) == 1);
    CHECK(compare_signed_unsigned_math(10, UINT32_C(42)) == -1);
    CHECK(
        compare_signed_unsigned_math(INT32_MAX, UINT32_MAX) == -1
    );

    for (unsigned i = 0; i < 20000; i++) {
        int32_t signed_value = (int32_t)next_random_u32();
        uint32_t unsigned_value = next_random_u32();

        int64_t s = signed_value;
        uint64_t u = unsigned_value;
        int expected;

        if (s < 0) {
            expected = -1;
        } else if ((uint64_t)s < u) {
            expected = -1;
        } else if ((uint64_t)s > u) {
            expected = 1;
        } else {
            expected = 0;
        }

        CHECK(
            compare_signed_unsigned_math(signed_value, unsigned_value)
                == expected
        );
    }
}

static void test_problem_6(void) {
    current_test = "problem 6: multiply by 24";

    CHECK_U32(multiply_by_24(UINT32_C(0)), UINT32_C(0));
    CHECK_U32(multiply_by_24(UINT32_C(1)), UINT32_C(24));
    CHECK_U32(multiply_by_24(UINT32_C(10)), UINT32_C(240));
    CHECK_U32(
        multiply_by_24(UINT32_MAX),
        UINT32_MAX * UINT32_C(24)
    );
    CHECK_U32(
        multiply_by_24(UINT32_C(0x80000000)),
        UINT32_C(0x80000000) * UINT32_C(24)
    );

    for (unsigned i = 0; i < 20000; i++) {
        uint32_t x = next_random_u32();
        CHECK_U32(multiply_by_24(x), x * UINT32_C(24));
    }
}

static void check_fp32_info(
    uint32_t bits,
    bool negative,
    fp32_kind_t kind,
    uint8_t exponent_bits,
    uint32_t fraction_bits,
    int unbiased_exponent
) {
    fp32_info_t info = fp32_decode(bits);

    CHECK(info.negative == negative);
    CHECK(info.kind == kind);
    CHECK(info.exponent_bits == exponent_bits);
    CHECK_U32(info.fraction_bits, fraction_bits);
    CHECK(info.unbiased_exponent == unbiased_exponent);
}

static void test_problem_7(void) {
    current_test = "problem 7: IEEE-754 decoding";

    CHECK(sizeof(float) == 4);
    CHECK(FLT_RADIX == 2);
    CHECK(FLT_MANT_DIG == 24);
    CHECK(FLT_MAX_EXP == 128);

    check_fp32_info(
        UINT32_C(0x00000000),
        false,
        FP32_ZERO,
        0,
        0,
        -126
    );
    check_fp32_info(
        UINT32_C(0x80000000),
        true,
        FP32_ZERO,
        0,
        0,
        -126
    );
    check_fp32_info(
        UINT32_C(0x00000001),
        false,
        FP32_SUBNORMAL,
        0,
        1,
        -126
    );
    check_fp32_info(
        UINT32_C(0x3F800000),
        false,
        FP32_NORMAL,
        127,
        0,
        0
    );
    check_fp32_info(
        UINT32_C(0xC0B80000),
        true,
        FP32_NORMAL,
        129,
        UINT32_C(0x380000),
        2
    );
    check_fp32_info(
        UINT32_C(0x7F800000),
        false,
        FP32_INFINITY,
        255,
        0,
        0
    );
    check_fp32_info(
        UINT32_C(0xFF800000),
        true,
        FP32_INFINITY,
        255,
        0,
        0
    );
    check_fp32_info(
        UINT32_C(0x7FC12345),
        false,
        FP32_NAN,
        255,
        UINT32_C(0x412345),
        0
    );

    CHECK(fp32_exact_value(UINT32_C(0x40B80000)) == 5.75);
    CHECK(fp32_exact_value(UINT32_C(0xBEC80000)) == -0.390625);
    CHECK(fp32_exact_value(UINT32_C(0x3F800000)) == 1.0);
    CHECK(
        fp32_exact_value(UINT32_C(0x3DCCCCCD))
            == (double)bits_to_float(UINT32_C(0x3DCCCCCD))
    );
    CHECK(
        fp32_exact_value(UINT32_C(0x00000001))
            == ldexp(1.0, -149)
    );
    CHECK(
        fp32_exact_value(UINT32_C(0x007FFFFF))
            == (double)bits_to_float(UINT32_C(0x007FFFFF))
    );
    CHECK(
        fp32_exact_value(UINT32_C(0x7F7FFFFF))
            == (double)bits_to_float(UINT32_C(0x7F7FFFFF))
    );

    double positive_zero = fp32_exact_value(UINT32_C(0x00000000));
    double negative_zero = fp32_exact_value(UINT32_C(0x80000000));
    CHECK(positive_zero == 0.0);
    CHECK(!signbit(positive_zero));
    CHECK(negative_zero == 0.0);
    CHECK(signbit(negative_zero));
    CHECK(isinf(fp32_exact_value(UINT32_C(0x7F800000))));
    CHECK(fp32_exact_value(UINT32_C(0x7F800000)) > 0.0);
    CHECK(isinf(fp32_exact_value(UINT32_C(0xFF800000))));
    CHECK(fp32_exact_value(UINT32_C(0xFF800000)) < 0.0);
    CHECK(isnan(fp32_exact_value(UINT32_C(0x7FC00001))));

    for (unsigned i = 0; i < 20000; i++) {
        uint32_t bits = next_random_u32();
        uint32_t exponent = (bits >> 23) & UINT32_C(0xFF);
        uint32_t fraction = bits & UINT32_C(0x7FFFFF);

        if (exponent == UINT32_C(0xFF) && fraction != 0) {
            CHECK(isnan(fp32_exact_value(bits)));
        } else {
            double expected = (double)bits_to_float(bits);
            double actual = fp32_exact_value(bits);

            if (expected == 0.0) {
                CHECK(actual == 0.0);
                CHECK(signbit(actual) == signbit(expected));
            } else {
                CHECK(actual == expected);
            }
        }
    }
}

static void test_problem_8(void) {
    current_test = "problem 8: next representable float";

    CHECK_U32(
        fp32_next_up_bits(UINT32_C(0x00000000)),
        UINT32_C(0x00000001)
    );
    CHECK_U32(
        fp32_next_up_bits(UINT32_C(0x80000000)),
        UINT32_C(0x00000001)
    );
    CHECK_U32(
        fp32_next_up_bits(UINT32_C(0x3F800000)),
        UINT32_C(0x3F800001)
    );
    CHECK_U32(
        fp32_next_up_bits(UINT32_C(0xBF800000)),
        UINT32_C(0xBF7FFFFF)
    );
    CHECK_U32(
        fp32_next_up_bits(UINT32_C(0x7F7FFFFF)),
        UINT32_C(0x7F800000)
    );
    CHECK_U32(
        fp32_next_up_bits(UINT32_C(0x7F800000)),
        UINT32_C(0x7F800000)
    );
    CHECK_U32(
        fp32_next_up_bits(UINT32_C(0xFF800000)),
        UINT32_C(0xFF7FFFFF)
    );
    CHECK_U32(
        fp32_next_up_bits(UINT32_C(0x80000001)),
        UINT32_C(0x80000000)
    );
    CHECK_U32(
        fp32_next_up_bits(UINT32_C(0x7FC12345)),
        UINT32_C(0x7FC12345)
    );

    for (unsigned i = 0; i < 30000; i++) {
        uint32_t bits = next_random_u32();
        uint32_t exponent = bits & UINT32_C(0x7F800000);
        uint32_t fraction = bits & UINT32_C(0x007FFFFF);

        if (exponent == UINT32_C(0x7F800000) && fraction != 0) {
            CHECK_U32(fp32_next_up_bits(bits), bits);
            continue;
        }

        float value = bits_to_float(bits);
        float expected_value = nextafterf(value, INFINITY);
        uint32_t expected_bits = float_to_bits(expected_value);

        CHECK_U32(fp32_next_up_bits(bits), expected_bits);
    }
}

static void test_problem_9(void) {
    current_test = "problem 9: float-to-int truncation";

    int32_t out = INT32_C(123456);

    CHECK(fp32_truncate_to_int(UINT32_C(0x407F5C29), &out));
    CHECK_I32(out, 3);

    CHECK(fp32_truncate_to_int(UINT32_C(0xC07F5C29), &out));
    CHECK_I32(out, -3);

    CHECK(fp32_truncate_to_int(UINT32_C(0x3F7FFFFF), &out));
    CHECK_I32(out, 0);

    CHECK(fp32_truncate_to_int(UINT32_C(0xBF7FFFFF), &out));
    CHECK_I32(out, 0);

    CHECK(fp32_truncate_to_int(UINT32_C(0x00000001), &out));
    CHECK_I32(out, 0);

    CHECK(fp32_truncate_to_int(UINT32_C(0x4EFFFFFF), &out));
    CHECK_I32(out, INT32_C(2147483520));

    CHECK(!fp32_truncate_to_int(UINT32_C(0x4F000000), &out));

    CHECK(fp32_truncate_to_int(UINT32_C(0xCF000000), &out));
    CHECK_I32(out, INT32_MIN);

    CHECK(!fp32_truncate_to_int(UINT32_C(0xCF000001), &out));
    CHECK(!fp32_truncate_to_int(UINT32_C(0x7F800000), &out));
    CHECK(!fp32_truncate_to_int(UINT32_C(0xFF800000), &out));
    CHECK(!fp32_truncate_to_int(UINT32_C(0x7FC12345), &out));

    for (unsigned i = 0; i < 50000; i++) {
        uint32_t bits = next_random_u32();
        float value = bits_to_float(bits);
        double as_double = (double)value;
        bool expected_valid = false;
        int32_t expected = 0;

        if (isfinite(as_double)) {
            double truncated = trunc(as_double);
            if (truncated >= (double)INT32_MIN &&
                truncated <= (double)INT32_MAX) {
                expected_valid = true;
                expected = (int32_t)truncated;
            }
        }

        int32_t actual = INT32_C(0x12345678);
        bool actual_valid = fp32_truncate_to_int(bits, &actual);

        CHECK(actual_valid == expected_valid);
        if (expected_valid && actual_valid) {
            CHECK_I32(actual, expected);
        }
    }
}

static bool selected(const char *requested, const char *number) {
    return strcmp(requested, "all") == 0 || strcmp(requested, number) == 0;
}

int main(int argc, char **argv) {
    const char *requested = argc >= 2 ? argv[1] : "all";

    if (selected(requested, "1")) {
        test_problem_1();
    }
    if (selected(requested, "2")) {
        test_problem_2();
    }
    if (selected(requested, "3")) {
        test_problem_3();
    }
    if (selected(requested, "4")) {
        test_problem_4();
    }
    if (selected(requested, "5")) {
        test_problem_5();
    }
    if (selected(requested, "6")) {
        test_problem_6();
    }
    if (selected(requested, "7")) {
        test_problem_7();
    }
    if (selected(requested, "8")) {
        test_problem_8();
    }
    if (selected(requested, "9")) {
        test_problem_9();
    }

    if (strcmp(requested, "all") != 0 &&
        strcmp(requested, "1") != 0 &&
        strcmp(requested, "2") != 0 &&
        strcmp(requested, "3") != 0 &&
        strcmp(requested, "4") != 0 &&
        strcmp(requested, "5") != 0 &&
        strcmp(requested, "6") != 0 &&
        strcmp(requested, "7") != 0 &&
        strcmp(requested, "8") != 0 &&
        strcmp(requested, "9") != 0) {
        fprintf(stderr, "Unknown test selection: %s\n", requested);
        fprintf(stderr, "Use all or a number from 1 through 9.\n");
        return 2;
    }

    if (failures == 0) {
        printf("PASS: %u checks\n", checks);
        return 0;
    }

    fprintf(
        stderr,
        "FAILED: %u of %u checks failed\n",
        failures,
        checks
    );
    return 1;
}
