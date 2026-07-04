// Shared Porffor 2c runtime types (included by generated/porffor/*.c)
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

#ifndef NAN
#define NAN (0.0 / 0.0)
#endif
#ifndef INFINITY
#define INFINITY (1.0 / 0.0)
#endif

struct ReturnValue {
    f64 value;
    i32 type;
};
