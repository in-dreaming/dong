#pragma once

#include <cmath>
#include <algorithm>

namespace dong::utils {

// 3D 向量
struct Vec3 {
    float x = 0.0f, y = 0.0f, z = 0.0f;

    Vec3() = default;
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vec3 operator+(const Vec3& v) const { return {x + v.x, y + v.y, z + v.z}; }
    Vec3 operator-(const Vec3& v) const { return {x - v.x, y - v.y, z - v.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vec3 operator/(float s) const { return {x / s, y / s, z / s}; }
    Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vec3& operator-=(const Vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }

    float dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    Vec3 cross(const Vec3& v) const {
        return {y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x};
    }
    float length() const { return std::sqrt(x * x + y * y + z * z); }
    float lengthSquared() const { return x * x + y * y + z * z; }
    Vec3 normalized() const {
        float len = length();
        return len > 0.0001f ? *this / len : Vec3{0, 0, 0};
    }
};

inline Vec3 operator*(float s, const Vec3& v) { return v * s; }

// 2D 向量
struct Vec2 {
    float x = 0.0f, y = 0.0f;

    Vec2() = default;
    Vec2(float x_, float y_) : x(x_), y(y_) {}

    Vec2 operator+(const Vec2& v) const { return {x + v.x, y + v.y}; }
    Vec2 operator-(const Vec2& v) const { return {x - v.x, y - v.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
};

// 4x4 矩阵 (列优先存储)
struct Mat4 {
    float m[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    static Mat4 identity() {
        return Mat4{};
    }

    static Mat4 perspective(float fov_y, float aspect, float near, float far) {
        Mat4 result{};
        float tan_half_fov = std::tan(fov_y * 0.5f);
        
        result.m[0] = 1.0f / (aspect * tan_half_fov);
        result.m[5] = 1.0f / tan_half_fov;
        result.m[10] = -(far + near) / (far - near);
        result.m[11] = -1.0f;
        result.m[14] = -(2.0f * far * near) / (far - near);
        result.m[15] = 0.0f;
        
        return result;
    }

    static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
        Vec3 f = (target - eye).normalized();
        Vec3 r = f.cross(up).normalized();
        Vec3 u = r.cross(f);

        Mat4 result{};
        result.m[0] = r.x;  result.m[4] = r.y;  result.m[8] = r.z;   result.m[12] = -r.dot(eye);
        result.m[1] = u.x;  result.m[5] = u.y;  result.m[9] = u.z;   result.m[13] = -u.dot(eye);
        result.m[2] = -f.x; result.m[6] = -f.y; result.m[10] = -f.z; result.m[14] = f.dot(eye);
        result.m[3] = 0;    result.m[7] = 0;    result.m[11] = 0;    result.m[15] = 1;
        
        return result;
    }

    static Mat4 translate(const Vec3& v) {
        Mat4 result{};
        result.m[12] = v.x;
        result.m[13] = v.y;
        result.m[14] = v.z;
        return result;
    }

    static Mat4 rotateY(float angle) {
        Mat4 result{};
        float c = std::cos(angle);
        float s = std::sin(angle);
        result.m[0] = c;  result.m[8] = s;
        result.m[2] = -s; result.m[10] = c;
        return result;
    }

    static Mat4 rotateX(float angle) {
        Mat4 result{};
        float c = std::cos(angle);
        float s = std::sin(angle);
        result.m[5] = c;  result.m[9] = -s;
        result.m[6] = s;  result.m[10] = c;
        return result;
    }

    Mat4 operator*(const Mat4& other) const {
        Mat4 result{};
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m[i + j * 4] = 0;
                for (int k = 0; k < 4; k++) {
                    result.m[i + j * 4] += m[i + k * 4] * other.m[k + j * 4];
                }
            }
        }
        return result;
    }

    Vec3 transformPoint(const Vec3& v) const {
        float w = m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15];
        return Vec3{
            (m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12]) / w,
            (m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13]) / w,
            (m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14]) / w
        };
    }

    Vec3 transformDirection(const Vec3& v) const {
        return Vec3{
            m[0] * v.x + m[4] * v.y + m[8] * v.z,
            m[1] * v.x + m[5] * v.y + m[9] * v.z,
            m[2] * v.x + m[6] * v.y + m[10] * v.z
        };
    }

    Mat4 inverse() const {
        Mat4 inv{};
        float det;

        inv.m[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + 
                   m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
        inv.m[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - 
                    m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
        inv.m[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + 
                   m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
        inv.m[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - 
                     m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
        inv.m[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - 
                    m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
        inv.m[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + 
                   m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
        inv.m[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - 
                    m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
        inv.m[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + 
                    m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
        inv.m[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + 
                   m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
        inv.m[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - 
                    m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
        inv.m[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + 
                    m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
        inv.m[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - 
                     m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
        inv.m[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - 
                    m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
        inv.m[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + 
                   m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
        inv.m[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - 
                     m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
        inv.m[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + 
                    m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

        det = m[0] * inv.m[0] + m[1] * inv.m[4] + m[2] * inv.m[8] + m[3] * inv.m[12];
        if (std::abs(det) < 0.0001f) return Mat4::identity();

        det = 1.0f / det;
        for (int i = 0; i < 16; i++) inv.m[i] *= det;
        return inv;
    }
};

// 射线
struct Ray {
    Vec3 origin;
    Vec3 direction;

    Ray() = default;
    Ray(const Vec3& o, const Vec3& d) : origin(o), direction(d.normalized()) {}

    Vec3 pointAt(float t) const { return origin + direction * t; }
};

// 平面（法向量 + 点）
struct Plane {
    Vec3 normal;
    float d = 0.0f;  // 到原点的距离

    Plane() = default;
    Plane(const Vec3& n, const Vec3& point) : normal(n.normalized()), d(-normal.dot(point)) {}
    Plane(const Vec3& n, float dist) : normal(n.normalized()), d(dist) {}

    // 射线与平面相交，返回 t 参数（负值表示不相交或在射线后方）
    float intersect(const Ray& ray) const {
        float denom = normal.dot(ray.direction);
        if (std::abs(denom) < 0.0001f) return -1.0f;  // 平行
        float t = -(normal.dot(ray.origin) + d) / denom;
        return t;
    }
};

// 3D 矩形（四个顶点定义）
struct Quad3D {
    Vec3 corners[4];  // 逆时针顺序：左下、右下、右上、左上
    Vec3 normal;

    Quad3D() = default;
    Quad3D(const Vec3& center, const Vec3& right, const Vec3& up, float width, float height) {
        Vec3 r = right.normalized() * (width * 0.5f);
        Vec3 u = up.normalized() * (height * 0.5f);
        corners[0] = center - r - u;  // 左下
        corners[1] = center + r - u;  // 右下
        corners[2] = center + r + u;  // 右上
        corners[3] = center - r + u;  // 左上
        normal = right.cross(up).normalized();
    }

    // 射线与矩形相交，返回相交点的 UV 坐标（0-1 范围），如果不相交返回 (-1, -1)
    Vec2 intersect(const Ray& ray, float* out_t = nullptr) const {
        Plane plane(normal, corners[0]);
        float t = plane.intersect(ray);
        if (t < 0.0f) return Vec2{-1, -1};

        Vec3 hit = ray.pointAt(t);
        
        // 计算局部坐标
        Vec3 edge1 = corners[1] - corners[0];  // 水平边
        Vec3 edge2 = corners[3] - corners[0];  // 垂直边
        Vec3 local = hit - corners[0];

        float u = local.dot(edge1) / edge1.lengthSquared();
        float v = local.dot(edge2) / edge2.lengthSquared();

        if (u >= 0.0f && u <= 1.0f && v >= 0.0f && v <= 1.0f) {
            if (out_t) *out_t = t;
            return Vec2{u, v};
        }
        return Vec2{-1, -1};
    }
};

// 辅助函数
inline float radians(float degrees) { return degrees * 3.14159265358979f / 180.0f; }
inline float degrees(float radians) { return radians * 180.0f / 3.14159265358979f; }
inline float clamp(float x, float min_val, float max_val) { 
    return std::min(std::max(x, min_val), max_val); 
}

} // namespace dong::utils
