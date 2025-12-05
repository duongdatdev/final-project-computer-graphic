/*******************************************************************************
 * THE SHIFTING MAZE - Matrix Operations Header
 * 
 * Implements 3D transformations exactly as described in Computer Graphics course:
 * - Translation matrix
 * - Scale matrix  
 * - Rotation matrices (X, Y, Z axis)
 * - Rotation around arbitrary axis: T = Tr(-P0) * Rx(φ) * Ry(-θ) * Rz(α) * Ry(θ) * Rx(-φ) * Tr(P0)
 * - Matrix multiplication: M = M1 * M2 * M3
 ******************************************************************************/

#ifndef MATRIX_H
#define MATRIX_H

#include <cmath>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 4x4 Matrix structure for homogeneous coordinates
struct Matrix4x4 {
    float m[4][4];
    
    Matrix4x4() {
        memset(m, 0, sizeof(m));
    }
    
    // Initialize as identity matrix
    void setIdentity() {
        memset(m, 0, sizeof(m));
        m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
    }
};

// 3D Vector with homogeneous coordinate
struct Vec4 {
    float x, y, z, w;
    
    Vec4() : x(0), y(0), z(0), w(1) {}
    Vec4(float _x, float _y, float _z) : x(_x), y(_y), z(_z), w(1) {}
    Vec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
    
    // Vector operations
    Vec4 operator+(const Vec4& v) const { return Vec4(x + v.x, y + v.y, z + v.z); }
    Vec4 operator-(const Vec4& v) const { return Vec4(x - v.x, y - v.y, z - v.z); }
    Vec4 operator*(float s) const { return Vec4(x * s, y * s, z * s); }
    
    float length() const { return sqrt(x*x + y*y + z*z); }
    
    void normalize() {
        float len = length();
        if (len > 0.0001f) {
            x /= len; y /= len; z /= len;
        }
    }
};

// ============================================================================
// TRANSLATION MATRIX - Tịnh tiến
// | 1  0  0  0 |
// | 0  1  0  0 |
// | 0  0  1  0 |
// | tx ty tz 1 |
// ============================================================================
inline Matrix4x4 createTranslationMatrix(float tx, float ty, float tz) {
    Matrix4x4 mat;
    mat.setIdentity();
    mat.m[3][0] = tx;
    mat.m[3][1] = ty;
    mat.m[3][2] = tz;
    return mat;
}

// ============================================================================
// SCALE MATRIX - Tỷ lệ
// | sx 0  0  0 |
// | 0  sy 0  0 |
// | 0  0  sz 0 |
// | 0  0  0  1 |
// ============================================================================
inline Matrix4x4 createScaleMatrix(float sx, float sy, float sz) {
    Matrix4x4 mat;
    mat.setIdentity();
    mat.m[0][0] = sx;
    mat.m[1][1] = sy;
    mat.m[2][2] = sz;
    return mat;
}

// ============================================================================
// ROTATION MATRIX AROUND X AXIS - Quay quanh trục X
// | 1    0       0    0 |
// | 0  cos(a) sin(a)  0 |
// | 0 -sin(a) cos(a)  0 |
// | 0    0       0    1 |
// ============================================================================
inline Matrix4x4 createRotationXMatrix(float angle) {
    Matrix4x4 mat;
    mat.setIdentity();
    float c = cos(angle);
    float s = sin(angle);
    mat.m[1][1] = c;  mat.m[1][2] = s;
    mat.m[2][1] = -s; mat.m[2][2] = c;
    return mat;
}

// ============================================================================
// ROTATION MATRIX AROUND Y AXIS - Quay quanh trục Y
// | cos(a)  0 -sin(a) 0 |
// |   0     1    0    0 |
// | sin(a)  0  cos(a) 0 |
// |   0     0    0    1 |
// ============================================================================
inline Matrix4x4 createRotationYMatrix(float angle) {
    Matrix4x4 mat;
    mat.setIdentity();
    float c = cos(angle);
    float s = sin(angle);
    mat.m[0][0] = c;  mat.m[0][2] = -s;
    mat.m[2][0] = s;  mat.m[2][2] = c;
    return mat;
}

// ============================================================================
// ROTATION MATRIX AROUND Z AXIS - Quay quanh trục Z
// | cos(a)  sin(a) 0  0 |
// | -sin(a) cos(a) 0  0 |
// |   0       0    1  0 |
// |   0       0    0  1 |
// ============================================================================
inline Matrix4x4 createRotationZMatrix(float angle) {
    Matrix4x4 mat;
    mat.setIdentity();
    float c = cos(angle);
    float s = sin(angle);
    mat.m[0][0] = c;  mat.m[0][1] = s;
    mat.m[1][0] = -s; mat.m[1][1] = c;
    return mat;
}

// ============================================================================
// ROTATION MATRIX AROUND ARBITRARY AXIS - Quay quanh trục bất kỳ
// Axis (x,y,z) must be normalized
// ============================================================================
inline Matrix4x4 createRotationArbitraryMatrix(float angle, float x, float y, float z) {
    Matrix4x4 mat;
    mat.setIdentity();
    
    // Normalize vector if needed
    float len = sqrt(x*x + y*y + z*z);
    if (len > 0.0001f) {
        x /= len; y /= len; z /= len;
    }
    
    float c = cos(angle);
    float s = sin(angle);
    float t = 1.0f - c;
    
    mat.m[0][0] = c + x*x*t;
    mat.m[0][1] = y*x*t + z*s;
    mat.m[0][2] = z*x*t - y*s;
    
    mat.m[1][0] = x*y*t - z*s;
    mat.m[1][1] = c + y*y*t;
    mat.m[1][2] = z*y*t + x*s;
    
    mat.m[2][0] = x*z*t + y*s;
    mat.m[2][1] = y*z*t - x*s;
    mat.m[2][2] = c + z*z*t;
    
    return mat;
}

// ============================================================================
// MATRIX MULTIPLICATION - Kết hợp ma trận: M = M1 * M2
// ============================================================================
inline Matrix4x4 multiplyMatrix(const Matrix4x4& A, const Matrix4x4& B) {
    Matrix4x4 result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.m[i][j] = 0;
            for (int k = 0; k < 4; k++) {
                result.m[i][j] += A.m[i][k] * B.m[k][j];
            }
        }
    }
    return result;
}

#endif // MATRIX_H