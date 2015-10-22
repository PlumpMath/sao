/*
  My math library.
  Supports function overloading if on c11.
  - Stephen Olsen
 */
#ifndef _sao_math_h
#define _sao_math_h

#include <math.h>

#define PI 3.14159265358979323846

typedef union {
    struct {float x, y;};
    float e[2];
} V2;

typedef union {
    struct {float x, y, z;};
    struct {float r, theta, phi;};
    struct {V2 xy; float _z;};
    float e[3];
} V3;

typedef union {
    struct {float x; float y; float z; float w;};
    struct {float r; float g; float b; float a;};
    struct {V3 xyz; float _w;};
    struct {V3 rgb; float _a;};
    struct {V2 xy; V2 zw;};
    float e[4];
} V4;

// 4x4 matrix stored column major
typedef union {
    struct {V4 col1, col2, col3, col4;};
    V4 cols[4];
    float e[16];
} Mat4;

// Macros
#define CLAMP(n, min, max) ((n<min)?(min):((n>max)?(max):(n)))
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

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
scale_v2(V2 v, float n)
{
    v.x *= n;
    v.y *= n;

    return v;
}

// V3
static inline V3
v3(float x, float y, float z) {
    V3 result;

    result.x = x;
    result.y = y;
    result.z = z;

    return result;
}

static inline V3
add_v3(V3 a, V3 b)
{
    V3 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    
    return result;
}

static inline V3
sub_v3(V3 a, V3 b)
{
    V3 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    
    return result;
}

static inline V3
normalize_v3(V3 v)
{
    float magnitude = sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
    if (magnitude != 0) {
        v.x = v.x / magnitude;
        v.y = v.y / magnitude;
        v.z = v.z / magnitude;
    }
    return v;
}

static inline V3
scale_v3(V3 v, float n)
{
    V3 result;

    result.x = n*v.x;
    result.y = n*v.y;
    result.z = n*v.z;
    
    return result;
}

static inline float
dot(V3 a, V3 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

static inline V3
cross(V3 a, V3 b)
{
    V3 result;

    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;

    return result;
}

// V4
inline V4
v4(float x, float y, float z, float w) {
    V4 result;

    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;

    return result;
}

inline V4
v4_from_v3(V3 v, float w)
{
    V4 result;
    
    result.x = v.x;
    result.y = v.y;
    result.z = v.z;
    result.w = w;

    return result;
}

static inline V4
add_v4(V4 a, V4 b)
{
    V4 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;
    
    return result;
}

static inline V4
sub_v4(V4 a, V4 b)
{
    V4 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    result.w = a.w - b.w;
    
    return result;
}

static inline V4
normalize_v4(V4 v)
{
    float magnitude = sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z) + (v.w * v.w));
    if (magnitude != 0) {
        v.x = v.x / magnitude;
        v.y = v.y / magnitude;
        v.z = v.z / magnitude;
        v.w = v.w / magnitude;
    }
    return v;
}

static inline V4
scale_v4(V4 v, float n)
{
    V4 result;

    result.x = n*v.x;
    result.y = n*v.y;
    result.z = n*v.z;
    result.w = n*v.w;
    
    return result;
}

// Mat4

static inline Mat4
mat4(float a11, float a21, float a31, float a41,
     float a12, float a22, float a32, float a42,
     float a13, float a23, float a33, float a43,
     float a14, float a24, float a34, float a44)
{
    Mat4 result;

    result.e[0] = a11;
    result.e[1] = a12;
    result.e[2] = a13;
    result.e[3] = a14;

    result.e[4] = a21;
    result.e[5] = a22;
    result.e[6] = a23;
    result.e[7] = a24;
    
    result.e[8] = a31;
    result.e[9] = a32;
    result.e[10] = a33;
    result.e[11] = a34;
    
    result.e[12] = a41;
    result.e[13] = a42;
    result.e[14] = a43;
    result.e[15] = a44;

    return result;
}

static const Mat4 IDENTITY_MATRIX = (Mat4){
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};


// This is tedious and hard to read compared to operator overloading :(
// @TODO: Is there a varadic argument way to do this so I can say.
// mul(A, B, C, D) which means A * B * C * D
static inline Mat4
mul_mat4(Mat4 a, Mat4 b)
{
    Mat4 result = IDENTITY_MATRIX;

    for (int row=0; row<4; row++) {
        for (int col=0; col<4; col++) {
            result.e[row*4 + col] =
                (a.e[row*4 + 0] * b.e[col + 0]) +
                (a.e[row*4 + 1] * b.e[col + 4]) +
                (a.e[row*4 + 2] * b.e[col + 8]) +
                (a.e[row*4 + 3] * b.e[col + 12]);
        }
    }

    return result;
}

static inline Mat4
perspective(float field_of_view,
            float display_ratio,
            float near_clip,
            float far_clip)
{
    float y_scale = cotan(to_rad(field_of_view / 2.0));
    float x_scale = y_scale / display_ratio;
    float frustrum_length = far_clip - near_clip;
    float z_scale = -(far_clip + near_clip)/(frustrum_length);

    Mat4 result = mat4(
        x_scale, 0,       0,       0,
        0,       y_scale, 0,       0,
        0,       0,       z_scale,-((2 * near_clip * far_clip)/frustrum_length),
        0,       0,      -1,       0
    );
    
    return result;
}

static inline Mat4
look_at(V3 eye,
        V3 center,
        V3 up)
{
    V3 f = normalize_v3(sub_v3(center, eye));
    V3 s = normalize_v3(cross(f, up));
    V3 u = normalize_v3(cross(s, f));

    Mat4 result = IDENTITY_MATRIX;

    result.e[0*4+0] = s.x;
    result.e[1*4+0] = s.y;
    result.e[2*4+0] = s.z;
    result.e[0*4+1] = u.x;
    result.e[1*4+1] = u.y;
    result.e[2*4+1] = u.z;
    result.e[0*4+2] =-f.x;
    result.e[1*4+2] =-f.y;
    result.e[2*4+2] =-f.z;
    result.e[3*4+0] =-dot(s, eye);
    result.e[3*4+1] =-dot(u, eye);
    result.e[3*4+2] = dot(f, eye);
    
    return result;
}

// Generic definitions.
#define add(x, y) _Generic((x),                 \
                           V2: add_v2,          \
                           V3: add_v3,          \
                           V4: add_v4)(x, y)    \

#define sub(x, y) _Generic((x),                 \
                           V2: sub_v2,          \
                           V3: sub_v3,          \
                           V4: sub_v4)(x, y)    \

#define normalize(x) _Generic((x),                      \
                              V2: normalize_v2,         \
                              V3: normalize_v3,         \
                              V4: normalize_v4)(x)      \

#define scale(x, y) _Generic((x),                       \
                             V2: scale_v2,              \
                             V3: scale_v3,              \
                             V4: scale_v4)(x, y)        \

#endif
