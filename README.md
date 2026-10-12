# COM2113 Programming Assignment: Bits, Integers, and Floating Point

This assignment contains nine independent C exercises. It is not one cohesive application. Each problem targets a specific idea from the Bits/Integers and Floating-Point lectures.

## The required method is part of the assignment

A function does not earn full credit merely because it returns the expected values. For the bit-manipulation problems, you must operate directly on integer bit patterns using masks, shifts, and bitwise operators. For the floating-point problems, you must decode the IEEE-754 fields yourself.

The tests can check behavior, but no finite test suite can detect every shortcut. Your source may therefore be inspected. A solution that converts a number into a string, uses a wider type to avoid the intended boundary case, or reinterprets raw bits as a `float` may pass some tests while still violating the assignment.

The same reasoning will appear on the exam, where these shortcuts will not help. Treat every **Required approach** and **Forbidden shortcuts** section as part of the specification.

## Learning goals

By the end of the assignment, you should be able to:

- treat a bit string as a compact set;
- build masks and move bit fields safely;
- interpret a fixed-width two's-complement value;
- distinguish carry-out from signed overflow;
- avoid signed/unsigned comparison traps;
- replace multiplication by a constant with shifts and subtraction;
- decode IEEE-754 binary32 fields;
- reason about normal values, subnormal values, infinity, NaN, and signed zero;
- find the next representable `float`; and
- reproduce truncation toward zero without performing a floating-point cast.

## Files you may edit

Edit only:

```text
src/assignment.c
```

Do not change the declarations in `include/assignment.h`. The tests compile against those exact declarations.

## Build and test

Run all tests:

```bash
make test
```

Run one problem:

```bash
make test-1
make test-2
make test-3
make test-4
make test-5
make test-6
make test-7
make test-8
make test-9
```

Other useful commands:

```bash
make clean
make test-asan
make valgrind
```

## Assumptions

- A byte has 8 bits.
- `uint32_t` and `int32_t` exist and have exactly 32 bits.
- `float` uses IEEE-754 binary32: 1 sign bit, 8 exponent bits, and 23 fraction bits.
- The course environment is GCC or Clang on x86-64 Ubuntu.
- Unless stated otherwise, arguments satisfy the listed preconditions.
- Bit positions are numbered from the right, starting at 0.

When an example shows only 8 bits, assume the omitted upper 24 bits are zero unless the example says otherwise.

---

# Problem 1 — Bitsets and symmetric difference

Implement:

```c
unsigned bitset_symmetric_difference_count(uint32_t a, uint32_t b);
```

## What the inputs mean

Interpret each 32-bit value as a set of possible elements `0` through `31`.

- Bit `i` equal to `1` means element `i` is present.
- Bit `i` equal to `0` means element `i` is absent.

Return the number of elements that appear in **exactly one** input set. An element that appears in both sets is not counted. An element that appears in neither set is also not counted.

This operation is called the **symmetric difference** of the two sets.

## Example 1: overlapping sets

```text
a = 01101001
b = 01010101
```

Read the bits from right to left as elements `0` through `7`:

```text
a contains {0, 3, 5, 6}
b contains {0, 2, 4, 6}
```

Elements `0` and `6` appear in both sets, so they are not counted. The elements that appear in exactly one set are:

```text
{2, 3, 4, 5}
```

## Example 2: identical sets

```text
a = 10100110
b = 10100110
```

Every present element occurs in both sets. 

There are no elements in exactly one set, so return `0`.

## Example 3: disjoint sets

```text
a = 00001111
b = 11110000
```

The first set contains elements `0` through `3`. The second contains elements `4` through `7`. They have no overlap.


All eight shown elements occur in exactly one set, so return `8`.

## Example 4: a high bit still counts

```text
a = 0x80000000
b = 0x00000000
```

Only bit `31` differs. That means element `31` appears in exactly one set, so return `1`.

## Required approach

- Count the `1` bits using bit operations such as `&`, shifts, and subtraction.
- Operate directly on the integer bit pattern.

## Forbidden shortcuts

- No binary strings.
- No array with one entry per bit.
- No repeated `/ 2` or `% 2` to extract digits.
- No `__builtin_popcount` or related builtin/library operation.

Test:

```bash
make test-1
```

---

# Problem 2 — Masks and bit-field splicing

Implement both functions:

```c
uint32_t bit_range_mask(unsigned low, unsigned high);

uint32_t splice_bits(
    uint32_t destination,
    uint32_t source,
    unsigned low,
    unsigned high
);
```

Precondition for both functions:

```text
0 <= low <= high < 32
```

## Part A: `bit_range_mask`

Return a 32-bit value whose inclusive range of bit positions `[low, high]` is `1`. Every bit outside that range must be `0`.

The range is inclusive, so its width is:

```text
high - low + 1
```

### Example 1: a four-bit range

```text
bit_range_mask(2, 5)
```

Positions `2`, `3`, `4`, and `5` must be `1`:

```text
bit position: 7 6 5 4 3 2 1 0
result:       0 0 1 1 1 1 0 0
```

Therefore the low byte is:

```text
00111100
```

and the full 32-bit result is:

```text
00000000 00000000 00000000 00111100
```

### Example 2: one selected bit

```text
bit_range_mask(7, 7)
```

The range contains only position `7`, so the result is:

```text
00000000 00000000 00000000 10000000
```

### Example 3: the complete 32-bit range

```text
bit_range_mask(0, 31)
```

Every position is inside the range, so return:

```text
11111111 11111111 11111111 11111111
```

This edge case matters because shifting a 32-bit value by 32 is undefined behavior in C.

## Part B: `splice_bits`

Copy the low `high - low + 1` bits of `source` into positions `[low, high]` of `destination`.

- Bits of `destination` inside the range are replaced.
- Bits of `destination` outside the range remain exactly as they were.
- Only the required low bits of `source` are used. Higher source bits are ignored.

### Example 1: insert four bits into the middle

```text
destination = 10100001
source      = 00001110
low = 2
high = 5
```

The target range has width `5 - 2 + 1 = 4`, so use the low four source bits:

```text
source low four bits = 1110
```

Positions `5` through `2` of the destination are replaced:

```text
position:             7 6 | 5 4 3 2 | 1 0
original destination: 1 0 | 1 0 0 0 | 0 1
inserted source bits:     | 1 1 1 0 |
result:               1 0 | 1 1 1 0 | 0 1
```

Return:

```text
10111001
```

### Example 2: source bits can clear destination bits

```text
destination = 11111111
source      = 00000000
low = 3
high = 5
```

The range has width 3, and the low three source bits are `000`. Replace destination positions `5`, `4`, and `3` with zero:

```text
position: 7 6 | 5 4 3 | 2 1 0
before:   1 1 | 1 1 1 | 1 1 1
after:    1 1 | 0 0 0 | 1 1 1
```

Return:

```text
11000111
```

### Example 3: higher source bits are ignored

```text
destination = 00000000
source      = 11110101
low = 1
high = 3
```

The range width is 3, so use only the low three source bits:

```text
source low three bits = 101
```

Move those bits into positions `3` through `1`:

```text
result = 00001010
```

The leading `11110` from `source` does not participate.

### Example 4: replace the entire word

```text
splice_bits(destination, source, 0, 31)
```

The range includes every bit, so every destination bit is replaced by the corresponding source bit. The result is exactly `source`.

## Required approach

- Build masks using shifts and bitwise operators.
- Clear the destination range before inserting the source field.
- Preserve every bit outside `[low, high]`.

## Forbidden shortcuts

- No strings or per-bit arrays.
- No loop that copies one bit position at a time.
- No `pow`, floating-point arithmetic, or precomputed mask table.

Test:

```bash
make test-2
```

---

# Problem 3 — Decode a fixed-width two's-complement value

Implement:

```c
int32_t decode_twos_complement(uint32_t bits, unsigned width);
```

Precondition:

```text
1 <= width <= 32
```

## What the function must do

Interpret only the low `width` bits of `bits` as a signed two's-complement integer. Ignore every higher bit.

For a `width`-bit value:

- the highest included bit, position `width - 1`, is the sign bit;
- sign bit `0` means the value is nonnegative;
- sign bit `1` means the value is negative.

Return the mathematical value as an `int32_t`.

## Example 1: positive 8-bit value

```text
decode_twos_complement(0b01111111, 8)
```

The included bits are:

```text
01111111
```

The sign bit is `0`, so read the remaining bits as an ordinary binary number:

```text
64 + 32 + 16 + 8 + 4 + 2 + 1 = 127
```

Return `127`.

## Example 2: the most-negative 8-bit value

```text
decode_twos_complement(0b10000000, 8)
```

The sign bit is `1`, so the value is negative. In 8-bit two's complement, the sign position has weight `-128`, and all other bits are zero:

```text
-128 + 0 = -128
```

Return `-128`.

This is a special boundary value: positive `128` cannot be represented in signed 8-bit two's complement.

## Example 3: a general negative value

```text
decode_twos_complement(0b11101011, 8)
```

One way to reason about it is to invert and add 1 to find the magnitude:

```text
bits:              11101011
invert:            00010100
add 1:             00010101
unsigned magnitude = 21
```

Because the original sign bit was `1`, return `-21`.

You can also use signed bit weights:

```text
-128 + 64 + 32 + 8 + 2 + 1 = -21
```

## Example 4: higher bits are ignored

```text
decode_twos_complement(0xFFFFFFFD, 3)
```

Only the low three bits matter:

```text
0xFFFFFFFD ends in ...101
```

As a 3-bit signed value, `101` has weights:

```text
-4 + 0 + 1 = -3
```

Return `-3`. The many `1` bits above position 2 must not change the answer.

## Example 5: width 1

```text
decode_twos_complement(0, 1) -> 0
decode_twos_complement(1, 1) -> -1
```

A 1-bit signed two's-complement system has only two possible values:

```text
0 -> 0
1 -> -1
```

## Required approach

- Mask away bits above the requested width.
- Examine the sign bit for that width.
- Use two's-complement bit reasoning to obtain the signed value.
- Handle `width == 32` without undefined shifts or signed overflow.

## Forbidden shortcuts

- No strings or digit arrays.
- No `int64_t`, `uint64_t`, `long long`, or `unsigned long long`.
- No cast to a narrower signed type such as `int8_t` or `int16_t`.
- No floating-point arithmetic.

Test:

```bash
make test-3
```

---

# Problem 4 — Fixed-width addition and signed overflow

Implement:

```c
bool add_signed_width(
    uint32_t a_bits,
    uint32_t b_bits,
    unsigned width,
    uint32_t *sum_bits
);
```

Preconditions:

```text
1 <= width <= 32
sum_bits != NULL
```

## What the function must do

Interpret the low `width` bits of both inputs as signed two's-complement values.

The function has two outputs:

1. Store the wrapped low-`width`-bit sum in `*sum_bits`.
2. Return `true` exactly when the exact signed mathematical sum is outside the representable range for that width.

For example, an 8-bit signed value can represent only `-128` through `127`.

Do not confuse **carry-out** with **signed overflow**. They answer different questions.

## Example 1: largest valid positive result

In 8 bits:

```text
100 + 27 = 127
```

Bit patterns:

```text
  01100100   (100)
+ 00011011   (27)
= 01111111   (127)
```

The result remains positive and is representable. Store:

```text
*sum_bits = 0b01111111
```

Return `false` because there is no signed overflow.

## Example 2: positive overflow

In 8 bits:

```text
100 + 28 = 128
```

But signed 8-bit values stop at `127`:

```text
  01100100   (100)
+ 00011100   (28)
= 10000000   (wrapped low 8 bits)
```

The stored bits `10000000` represent `-128`, not positive `128`.

- Both inputs were positive.
- The wrapped result appears negative.

Store `0b10000000` and return `true`.

## Example 3: negative overflow

In 8 bits:

```text
-100 + -29 = -129
```

The exact result is below the minimum `-128`, so it cannot be represented. The wrapped low 8 bits are:

```text
-100 -> 10011100
 -29 -> 11100011
sum  -> 01111111
```

The stored bits represent positive `127`.

- Both inputs were negative.
- The wrapped result appears positive.

Store `0b01111111` and return `true`.

## Example 4: carry-out without signed overflow

In 8 bits:

```text
-39 + 92 = 53
```

Bit patterns:

```text
  11011001   (-39)
+ 01011100   (92)
=1 00110101
```

There is a carry beyond the low 8 bits, but the retained bits are:

```text
00110101 = 53
```

The operands have different signs, so signed overflow is impossible here. Store `0b00110101` and return `false`.

This example is why the carry bit alone is not a signed-overflow test.

## Example 5: a 4-bit boundary

Signed 4-bit range:

```text
-8 through 7
```

For `7 + 1`:

```text
  0111
+ 0001
= 1000
```

`1000` represents `-8`, so store `0b1000` and return `true`.

## Required approach

- Add the low-`width` patterns and mask the result back to the requested width.
- Determine overflow from the operand sign bits and result sign bit.
- Use the rule: two same-sign operands overflow exactly when the wrapped result has the opposite sign.

## Forbidden shortcuts

- No wider integer type to calculate the exact mathematical sum.
- No min/max comparison performed in `int64_t` or similar.
- No floating-point arithmetic.
- Do not use carry-out as the signed-overflow answer.

Test:

```bash
make test-4
```

---

# Problem 5 — Compare signed and unsigned values mathematically

Implement:

```c
int compare_signed_unsigned_math(int32_t signed_value, uint32_t unsigned_value);
```

Return:

```text
-1  if signed_value is mathematically smaller
 0  if both represent the same mathematical integer
+1  if signed_value is mathematically larger
```

## Important distinction

Compare the mathematical values a human intends. Do **not** blindly rely on C's mixed signed/unsigned comparison.

When C compares a 32-bit signed integer with a 32-bit unsigned integer, it may convert the signed value to unsigned first. A negative number then becomes a very large positive unsigned number.

## Example 1: negative versus zero

```text
compare_signed_unsigned_math(-1, 0) -> -1
```

Mathematically:

```text
-1 < 0
```

So return `-1`.

A direct C expression such as `signed_value < unsigned_value` is dangerous here because converting `-1` to `uint32_t` produces `4294967295`.

## Example 2: equal values

```text
compare_signed_unsigned_math(0, 0) -> 0
```

Both represent the same mathematical integer, so return `0`.

## Example 3: signed value is larger

```text
compare_signed_unsigned_math(42, 10) -> 1
```

The signed input is nonnegative, so its mathematical value survives conversion to unsigned:

```text
42 > 10
```

Return `1`.

## Example 4: unsigned value exceeds every positive `int32_t`

```text
compare_signed_unsigned_math(INT32_MAX, UINT32_MAX) -> -1
```

Their mathematical values are:

```text
INT32_MAX  = 2147483647
UINT32_MAX = 4294967295
```

Therefore the signed value is smaller, so return `-1`.

## Example 5: same bit pattern, different mathematical values

```text
signed_value   = -1          // bit pattern 0xFFFFFFFF
unsigned_value = UINT32_MAX  // bit pattern 0xFFFFFFFF
```

The bit patterns match, but the interpretations do not:

```text
signed mathematical value   = -1
unsigned mathematical value = 4294967295
```

Return `-1`, not `0`.

## Required approach

- Handle negative `signed_value` separately.
- Convert it to `uint32_t` only after proving it is nonnegative.
- Then compare two values of the same unsigned type.

## Forbidden shortcuts

- No `int64_t`, `uint64_t`, `long long`, or `unsigned long long` common representation.
- No floating-point common representation.

Test:

```bash
make test-5
```

---

# Problem 6 — Multiply by 24 using shifts and subtraction

Implement:

```c
uint32_t multiply_by_24(uint32_t x);
```

Return `24 * x` modulo `2^32`.

Because both input and output are unsigned 32-bit values, wraparound is defined.

## Required algebraic observation

Express 24 as a difference of powers of two:

```text
24 = 32 - 8
```

Therefore:

```text
24x = 32x - 8x
```

A left shift by 5 multiplies an unsigned value by 32 modulo `2^32`. A left shift by 3 multiplies it by 8 modulo `2^32`.

## Example 1: ordinary small value

```text
multiply_by_24(5) -> 120
```

Walkthrough:

```text
32 * 5 = 160
 8 * 5 = 40
160 - 40 = 120
```

Return `120`.

## Example 2: zero

```text
multiply_by_24(0) -> 0
```

Both shifted terms are zero, so their difference is zero.

## Example 3: one

```text
multiply_by_24(1) -> 24
```

Walkthrough:

```text
32 * 1 - 8 * 1 = 32 - 8 = 24
```

## Example 4: defined unsigned wraparound

```text
multiply_by_24(0x10000000) -> 0x80000000
```

The exact mathematical product is:

```text
0x10000000 * 24 = 0x180000000
```

A `uint32_t` keeps only the low 32 bits:

```text
0x180000000 mod 2^32 = 0x80000000
```

That wrapped result is required and is not an error.

## Operator restriction

Inside `multiply_by_24`, you may use only:

- `<<`
- `-`
- parentheses
- integer constants
- parameter `x`

You may not use:

- `*`, `+`, `/`, or `%`;
- loops;
- function calls;
- helper functions; or
- macros that hide a forbidden operation.

The source checker examines this function as well as its output.

Test:

```bash
make test-6
```

---

# Problem 7 — Decode IEEE-754 binary32

Implement both functions:

```c
fp32_info_t fp32_decode(uint32_t bits);
double fp32_exact_value(uint32_t bits);
```

## Binary32 layout

```text
31            30 ........ 23 22 ...................... 0
+---------------+-----------+---------------------------+
| sign (1 bit)  | exp (8)   | fraction (23)             |
+---------------+-----------+---------------------------+
```

`fp32_decode` fills:

```c
typedef struct {
    bool negative;
    fp32_kind_t kind;
    uint8_t exponent_bits;
    uint32_t fraction_bits;
    int unbiased_exponent;
} fp32_info_t;
```

Classification rules:

```text
exp = 0,   fraction = 0  -> zero
exp = 0,   fraction != 0 -> subnormal
exp 1..254               -> normal
exp = 255, fraction = 0  -> infinity
exp = 255, fraction != 0 -> NaN
```

For `unbiased_exponent`:

```text
normal         -> exponent_bits - 127
zero/subnormal -> -126
infinity/NaN   -> 0
```

For a normal value:

```text
value = (-1)^sign * (1.fraction) * 2^(exponent_bits - 127)
```

For a subnormal value there is no implicit leading `1`:

```text
value = (-1)^sign * (0.fraction) * 2^-126
```

`fp32_exact_value` returns the represented value as a `double`. Every finite binary32 value is exactly representable in binary64.

## Example 1: positive 1.0

```text
bits = 0x3F800000
```

Fields:

```text
sign bit      = 0
exponent bits = 127
fraction bits = 0
```

Classification:

- exponent is neither 0 nor 255, so this is normal;
- unbiased exponent is `127 - 127 = 0`;
- significand is `1.0` because the implicit leading 1 is followed by all-zero fraction bits.

Value:

```text
(+1) * 1.0 * 2^0 = 1.0
```

`fp32_decode` reports `FP32_NORMAL`, and `fp32_exact_value` returns `1.0`.

## Example 2: negative 2.5

```text
bits = 0xC0200000
```

Fields:

```text
sign bit      = 1
exponent bits = 128
fraction bits = 0x200000
```

The top fraction bit represents `1/4`, so the significand is:

```text
1 + 1/4 = 1.25
```

The unbiased exponent is:

```text
128 - 127 = 1
```

Value:

```text
(-1) * 1.25 * 2^1 = -2.5
```

## Example 3: smallest positive subnormal

```text
bits = 0x00000001
```

Fields:

```text
sign bit      = 0
exponent bits = 0
fraction bits = 1
```

Because exponent is zero and fraction is nonzero, this is subnormal. There is no hidden leading `1`.

The single lowest fraction bit contributes:

```text
2^-23 * 2^-126 = 2^-149
```

So `fp32_exact_value` returns exactly `2^-149` as a `double`.

## Example 4: negative zero

```text
bits = 0x80000000
```

Fields:

```text
sign bit      = 1
exponent bits = 0
fraction bits = 0
```

This is zero, but the sign bit is set. The decode result must report `negative == true`, and the numeric function must return **negative zero**, not positive zero.

Negative zero compares equal to positive zero, but its sign is still observable with functions such as `signbit`.

## Example 5: positive infinity

```text
bits = 0x7F800000
```

Fields:

```text
sign bit      = 0
exponent bits = 255
fraction bits = 0
```

Exponent 255 with a zero fraction means infinity. Return positive infinity from `fp32_exact_value`.

## Example 6: NaN

```text
bits = 0x7FC00001
```

Exponent 255 with a nonzero fraction means NaN. The exact payload does not need to be preserved by the returned `double`; the result simply must satisfy `isnan`.

## Required approach

- Extract sign, exponent, and fraction using masks and shifts.
- Classify from the exponent and fraction fields.
- Construct the mathematical value from the decoded fields.
- Preserve signed zero.
- Return the appropriate infinity or NaN.

## Forbidden shortcuts

- No `memcpy` into a `float`.
- No union reinterpretation.
- No pointer casting or pointer punning.
- No library classification of a reinterpreted `float`.

Test:

```bash
make test-7
```

---

# Problem 8 — Next representable float toward positive infinity

Implement:

```c
uint32_t fp32_next_up_bits(uint32_t bits);
```

Return the raw binary32 encoding of the smallest representable value that is **strictly greater** than the input's numeric value.

Special rules:

- NaN returns unchanged.
- Positive infinity returns unchanged.
- Positive zero and negative zero both move to the smallest positive subnormal.
- Negative infinity moves to the largest finite negative value.

## Why raw ordering helps

Among nonnegative finite binary32 values, increasing the raw unsigned bit pattern by 1 moves to the next larger representable value.

Among negative finite values, the raw ordering runs in the opposite numeric direction. To move upward toward positive infinity—toward a value that is less negative—the raw pattern moves downward by 1.

Zero, NaN, and infinity require explicit handling.

## Example 1: positive zero

```text
input  = 0x00000000   (+0.0)
output = 0x00000001
```

`0x00000001` is the smallest positive subnormal, equal to `2^-149`. There is no representable positive value between zero and it.

## Example 2: negative zero

```text
input  = 0x80000000   (-0.0)
output = 0x00000001
```

Although negative zero has a different encoding, it has the same numeric value as positive zero. The smallest representable value strictly greater than zero is again the smallest positive subnormal.

## Example 3: positive one

```text
input  = 0x3F800000   (1.0)
output = 0x3F800001
```

For positive normal values, the next greater encoding is one raw step higher. `0x3F800001` represents:

```text
1 + 2^-23
```

## Example 4: negative one

```text
input  = 0xBF800000   (-1.0)
output = 0xBF7FFFFF
```

The desired result must be slightly greater than `-1.0`, meaning slightly less negative. For negative values, that means decreasing the raw pattern by 1.

`0xBF7FFFFF` is approximately `-0.99999994`, which is indeed greater than `-1.0`.

## Example 5: largest finite positive value

```text
input  = 0x7F7FFFFF
output = 0x7F800000
```

The next raw positive pattern is positive infinity, so return the infinity encoding.

## Example 6: negative infinity

```text
input  = 0xFF800000
output = 0xFF7FFFFF
```

The smallest value greater than negative infinity is the most negative finite binary32 value, encoded by `0xFF7FFFFF`.

## Example 7: NaN and positive infinity

```text
fp32_next_up_bits(0x7FC01234) -> 0x7FC01234
fp32_next_up_bits(0x7F800000) -> 0x7F800000
```

Both return unchanged according to the assignment's special rules.

## Required approach

- Work directly on the 32-bit encoding.
- Detect NaN, infinity, and both zeros with masks/comparisons.
- Use the opposite raw-step directions for positive and negative finite values.

## Forbidden shortcuts

- No reinterpretation as `float`.
- No `nextafterf`, `nextafter`, or equivalent routine.
- No floating-point addition or subtraction.
- No `FLT_EPSILON`; adjacent-float spacing is not constant.

Test:

```bash
make test-8
```

---

# Problem 9 — Reproduce a float-to-int cast from raw bits

Implement:

```c
bool fp32_truncate_to_int(uint32_t bits, int32_t *out);
```

Precondition:

```text
out != NULL
```

Interpret `bits` as IEEE-754 binary32.

If truncation toward zero produces a value representable by `int32_t`, write it to `*out` and return `true`.

Return `false` for:

- NaN;
- positive or negative infinity; or
- a finite value whose truncated integer lies outside `INT32_MIN` through `INT32_MAX`.

Truncation toward zero means discarding the fractional portion:

```text
 3.99 ->  3
-3.99 -> -3
```

It does not mean floor for negative numbers.

## Example 1: positive value with a fraction

```text
bits = 0x415A0000   // 13.625f
```

Binary form of the value:

```text
13.625 = 1101.101₂ = 1.101101₂ * 2^3
```

The unbiased exponent `3` places the binary point after four whole-number bits:

```text
1101 . 101
```

Discard the fractional `.101` portion. Store `13` and return `true`.

## Example 2: negative value with a fraction

```text
bits = 0xC15A0000   // -13.625f
```

First determine the positive magnitude exactly as above:

```text
magnitude before truncation = 13.625
integer magnitude           = 13
```

Then apply the sign:

```text
result = -13
```

Store `-13` and return `true`. Do not round down to `-14`.

## Example 3: magnitude below one

```text
bits = 0x3F400000   // 0.75f
```

The binary value is `0.11₂`. There are no whole-number `1` bits to keep, so truncation produces `0`.

For the negative encoding of `-0.75f`, truncation also produces `0` because the conversion moves toward zero.

## Example 4: exactly `INT32_MAX`

`2147483647` is not exactly representable as binary32, so use the largest binary32 value below `2^31`:

```text
bits = 0x4EFFFFFF
value = 2147483520.0
```

This value has no fractional component and fits within `int32_t`. Store `2147483520` and return `true`.

## Example 5: positive `2^31` is invalid

```text
bits = 0x4F000000   // 2147483648.0 = 2^31
```

`INT32_MAX` is only `2147483647`, so the truncated value is too large. Return `false` and do not rely on a C float-to-int cast, whose out-of-range behavior is not what this problem asks you to reproduce.

## Example 6: negative `2^31` is valid

```text
bits = 0xCF000000   // -2147483648.0 = -2^31
```

This is exactly `INT32_MIN`, so store:

```text
-2147483648
```

and return `true`.

This asymmetry is important: `int32_t` has one more negative value than positive value.

## Example 7: NaN and infinity

```text
0x7FC00000 -> false   // NaN
0x7F800000 -> false   // +infinity
0xFF800000 -> false   // -infinity
```

None represents a finite integer result.

## Required approach

- Extract sign, exponent, and fraction using masks and shifts.
- Determine the binary-point location from the unbiased exponent.
- Shift the significand to discard fractional binary places.
- Apply the sign after determining the integer magnitude.
- Explicitly handle values below 1, NaN, infinity, `INT32_MIN`, and out-of-range values.

## Forbidden shortcuts

- No reinterpretation as a `float` using `memcpy`, unions, or pointer casts.
- No cast from `float` or `double` to integer.
- No `trunc`, `floor`, `ceil`, `round`, or similar routine.
- No `int64_t`, `uint64_t`, `long long`, or `unsigned long long` boundary shortcut.

Test:

```bash
make test-9
```

---

# Problem 10 — Write your own question
Pretend YOU are the instructor for this class now, and you are responsible for creating problems for the exam. Create a problem that is fully within the scope of what we have learned in class so far along with a detailed solution / walkthrough. **Post your question and solution on Piazza when you are done.**

Note: this will be graded on completion, so it is up to you how hard you want to try. However, as an incentive for you to be creative and to actually review the questions that your peers share on Piazza, I will pick up to one student-written problem to go on the exam (possibly with some modifications). The author of the question will get 1 extra credit point on the exam.

---

# Submission checklist

Before submitting:

```bash
make clean
make test
make test-asan
```

Also verify:

- no compiler warnings;
- no changes to the public header or tests;
- every problem uses the required method;
- no strings or per-bit arrays used to decode bit patterns;
- no forbidden wider-integer shortcut;
- no forbidden reinterpretation of raw bits as a `float`;
- no undefined shift such as `1u << 32`;
- no signed overflow used as a shortcut; and
- no confusion between logical operators (`&&`, `||`) and bitwise operators (`&`, `|`).

A passing test suite establishes correctness for the tested inputs. It does not by itself prove that the intended technique was used.
