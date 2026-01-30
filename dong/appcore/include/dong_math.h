#ifndef DONG_MATH_H
#define DONG_MATH_H

#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Simple 3D Math Types and Functions
// =============================================================================

typedef struct dong_vec3_t {
    float x, y, z;
} dong_vec3_t;

typedef struct dong_vec4_t {
    float x, y, z, w;
} dong_vec4_t;

typedef struct dong_mat4_t {
    float m[16];  // Column-major order
} dong_mat4_t;

// Vector operations
static inline dong_vec3_t dong_vec3(float x, float y, float z) {
    dong_vec3_t v = {x, y, z};
    return v;
}

static inline dong_vec3_t dong_vec3_add(dong_vec3_t a, dong_vec3_t b) {
    return dong_vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline dong_vec3_t dong_vec3_sub(dong_vec3_t a, dong_vec3_t b) {
    return dong_vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline dong_vec3_t dong_vec3_scale(dong_vec3_t v, float s) {
    return dong_vec3(v.x * s, v.y * s, v.z * s);
}

static inline float dong_vec3_dot(dong_vec3_t a, dong_vec3_t b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline dong_vec3_t dong_vec3_cross(dong_vec3_t a, dong_vec3_t b) {
    return dong_vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

static inline float dong_vec3_length(dong_vec3_t v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

static inline dong_vec3_t dong_vec3_normalize(dong_vec3_t v) {
    float len = dong_vec3_length(v);
    if (len > 0.0001f) {
        return dong_vec3_scale(v, 1.0f / len);
    }
    return dong_vec3(0, 0, 0);
}

// Matrix operations
static inline dong_mat4_t dong_mat4_identity(void) {
    dong_mat4_t m = {{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    }};
    return m;
}

static inline dong_mat4_t dong_mat4_perspective(float fov_y, float aspect, float near_plane, float far_plane) {
    float tan_half_fov = tanf(fov_y * 0.5f);
    dong_mat4_t m = {{0}};
    m.m[0] = 1.0f / (aspect * tan_half_fov);
    m.m[5] = 1.0f / tan_half_fov;
    m.m[10] = -(far_plane + near_plane) / (far_plane - near_plane);
    m.m[11] = -1.0f;
    m.m[14] = -(2.0f * far_plane * near_plane) / (far_plane - near_plane);
    return m;
}

static inline dong_mat4_t dong_mat4_look_at(dong_vec3_t eye, dong_vec3_t target, dong_vec3_t up) {
    dong_vec3_t f = dong_vec3_normalize(dong_vec3_sub(target, eye));
    dong_vec3_t s = dong_vec3_normalize(dong_vec3_cross(f, up));
    dong_vec3_t u = dong_vec3_cross(s, f);

    dong_mat4_t m = dong_mat4_identity();
    m.m[0] = s.x;
    m.m[4] = s.y;
    m.m[8] = s.z;
    m.m[1] = u.x;
    m.m[5] = u.y;
    m.m[9] = u.z;
    m.m[2] = -f.x;
    m.m[6] = -f.y;
    m.m[10] = -f.z;
    m.m[12] = -dong_vec3_dot(s, eye);
    m.m[13] = -dong_vec3_dot(u, eye);
    m.m[14] = dong_vec3_dot(f, eye);
    return m;
}

static inline dong_mat4_t dong_mat4_translate(float x, float y, float z) {
    dong_mat4_t m = dong_mat4_identity();
    m.m[12] = x;
    m.m[13] = y;
    m.m[14] = z;
    return m;
}

static inline dong_mat4_t dong_mat4_rotate_y(float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    dong_mat4_t m = dong_mat4_identity();
    m.m[0] = c;
    m.m[2] = -s;
    m.m[8] = s;
    m.m[10] = c;
    return m;
}

static inline dong_mat4_t dong_mat4_scale(float sx, float sy, float sz) {
    dong_mat4_t m = dong_mat4_identity();
    m.m[0] = sx;
    m.m[5] = sy;
    m.m[10] = sz;
    return m;
}

static inline dong_mat4_t dong_mat4_multiply(dong_mat4_t a, dong_mat4_t b) {
    dong_mat4_t result = {{0}};
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            float sum = 0;
            for (int k = 0; k < 4; k++) {
                sum += a.m[k * 4 + row] * b.m[col * 4 + k];
            }
            result.m[col * 4 + row] = sum;
        }
    }
    return result;
}

static inline dong_vec4_t dong_mat4_transform(dong_mat4_t m, dong_vec4_t v) {
    dong_vec4_t result;
    result.x = m.m[0] * v.x + m.m[4] * v.y + m.m[8] * v.z + m.m[12] * v.w;
    result.y = m.m[1] * v.x + m.m[5] * v.y + m.m[9] * v.z + m.m[13] * v.w;
    result.z = m.m[2] * v.x + m.m[6] * v.y + m.m[10] * v.z + m.m[14] * v.w;
    result.w = m.m[3] * v.x + m.m[7] * v.y + m.m[11] * v.z + m.m[15] * v.w;
    return result;
}

// Constants
#ifndef DONG_PI
#define DONG_PI 3.14159265358979323846f
#endif

#ifndef DONG_DEG_TO_RAD
#define DONG_DEG_TO_RAD(deg) ((deg) * DONG_PI / 180.0f)
#endif

#ifndef DONG_RAD_TO_DEG
#define DONG_RAD_TO_DEG(rad) ((rad) * 180.0f / DONG_PI)
#endif

#ifdef __cplusplus
}
#endif

#endif // DONG_MATH_H
