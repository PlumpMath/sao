/*
  My math library.
  Supports function overloading if on c11.
  - Stephen Olsen
 */
#ifndef _sao_math_h
#define _sao_math_h

#include <math.h>

#define PI 3.14159265358979323846

typedef union _V2 {
    struct {float x, y;};
    float e[2];
} V2;

typedef union _V3 {
    struct {float x, y, z;};
    struct {float r, theta, phi;};
    float e[3];
} V3;

typedef union _V4 {
    struct {float x; float y; float z; float w;};
    struct {float r; float g; float b; float a;};
    float e[4];
} V4;

// 4x4 matrix stored column major
typedef union _Mat4 {
    struct {V4 col1, col2, col3, col4;};
    V4 cols[4];
    float e[16];
} Mat4;

// Macros
#define CLAMP(n, min, max) ((n<min)?(min):((n>max)?(max):(n)))

// float
static inline float
cotan(float n)
{
    return 1.0 / tan(n);
}

static inline float
to_rad(float degrees)
{
    return degrees * PI/180.0;
}

static inline float
to_deg(float radians)
{
    return radians * 180.0/PI;
}

// V2
static inline V2
v2(float x, float y)
{
    V2 result;

    result.x = x;
    result.y = y;

    return result;
}

static inline V2
add_v2(V2 a, V2 b)
{
    V2 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

static inline V2
sub_v2(V2 a, V2 b)
{
    V2 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;

    return result;
}

static inline V2
normalize_v2(V2 v)
{
    float magnitude = sqrt(v.x * v.x + v.y * v.y);
    if (magnitude != 0) {
        v.x /= magnitude;
        v.y /= magnitude;
    }

    return v;
}

static inline V2
scale_v2_n(V2 v, float n)
{
    v.x *= n;
    v.y *= n;

    return v;
}

// Generic definitions.
// @TODO, add, sub, normalize, scale
#define add(x, y) _Generic((x), \
    V2: _Generic((y), \
      V2: add_v2) \
    )(x, y)

#endif
