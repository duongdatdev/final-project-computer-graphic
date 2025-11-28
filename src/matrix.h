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
    
    Vec4 normalized() const {
        Vec4 result = *this;
        result.normalize();
        return result;
    }
    
    // Dot product (scalar product) - used for back-face culling and lighting
    float dot(const Vec4& v) const {
        return x * v.x + y * v.y + z * v.z;
    }
    
    // Cross product - used for normal calculation
    Vec4 cross(const Vec4& v) const {
        return Vec4(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
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
// | cos(a) sin(a)  0  0 |
// |-sin(a) cos(a)  0  0 |
// |   0      0     1  0 |
// |   0      0     0  1 |
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

// ============================================================================
// TRANSFORM POINT BY MATRIX - Biến đổi điểm: P' = P * M
// ============================================================================
inline Vec4 transformPoint(const Vec4& p, const Matrix4x4& M) {
    Vec4 result;
    result.x = p.x * M.m[0][0] + p.y * M.m[1][0] + p.z * M.m[2][0] + p.w * M.m[3][0];
    result.y = p.x * M.m[0][1] + p.y * M.m[1][1] + p.z * M.m[2][1] + p.w * M.m[3][1];
    result.z = p.x * M.m[0][2] + p.y * M.m[1][2] + p.z * M.m[2][2] + p.w * M.m[3][2];
    result.w = p.x * M.m[0][3] + p.y * M.m[1][3] + p.z * M.m[2][3] + p.w * M.m[3][3];
    return result;
}

// ============================================================================
// ROTATION AROUND ARBITRARY AXIS - Quay quanh trục bất kỳ
// T = Tr(-P0) * Rx(φ) * Ry(-θ) * Rz(α) * Ry(θ) * Rx(-φ) * Tr(P0)
// 
// P0 = point on axis
// direction = axis direction vector (will be normalized)
// α = rotation angle
// ============================================================================
inline Matrix4x4 createRotationArbitraryAxis(const Vec4& P0, const Vec4& direction, float alpha) {
    Vec4 d = direction.normalized();
    
    // Calculate angles θ (theta) and φ (phi) from direction vector
    // d = (dx, dy, dz)
    // Project onto XZ plane to get theta
    float dxz = sqrt(d.x * d.x + d.z * d.z);
    
    float theta = 0;
    float phi = 0;
    
    if (dxz > 0.0001f) {
        theta = atan2(d.x, d.z);  // Angle in XZ plane
    }
    
    // Phi is angle from XZ plane
    phi = atan2(d.y, dxz);
    
    // Build composite transformation:
    // T = Tr(-P0) * Rx(φ) * Ry(-θ) * Rz(α) * Ry(θ) * Rx(-φ) * Tr(P0)
    
    Matrix4x4 T1 = createTranslationMatrix(-P0.x, -P0.y, -P0.z);  // Tr(-P0)
    Matrix4x4 Rx1 = createRotationXMatrix(phi);                     // Rx(φ)
    Matrix4x4 Ry1 = createRotationYMatrix(-theta);                  // Ry(-θ)
    Matrix4x4 Rz = createRotationZMatrix(alpha);                    // Rz(α)
    Matrix4x4 Ry2 = createRotationYMatrix(theta);                   // Ry(θ)
    Matrix4x4 Rx2 = createRotationXMatrix(-phi);                    // Rx(-φ)
    Matrix4x4 T2 = createTranslationMatrix(P0.x, P0.y, P0.z);      // Tr(P0)
    
    // Combine: T1 * Rx1 * Ry1 * Rz * Ry2 * Rx2 * T2
    Matrix4x4 result = multiplyMatrix(T1, Rx1);
    result = multiplyMatrix(result, Ry1);
    result = multiplyMatrix(result, Rz);
    result = multiplyMatrix(result, Ry2);
    result = multiplyMatrix(result, Rx2);
    result = multiplyMatrix(result, T2);
    
    return result;
}

// Convert matrix to OpenGL format (column-major)
inline void matrixToOpenGL(const Matrix4x4& mat, float* glMatrix) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            glMatrix[j * 4 + i] = mat.m[i][j];
        }
    }
}

#endif // MATRIX_H
