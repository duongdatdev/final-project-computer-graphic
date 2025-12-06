/*******************************************************************************
 * THE SHIFTING MAZE - Matrix Operations Header
 * 
 * Implements 3D transformations manually for Computer Graphics course.
 * NOTE: This matrix implementation uses a column-major memory layout convention
 * compatible with OpenGL's glLoadMatrixf.
 * 
 * Access: m[col][row]
 ******************************************************************************/

#ifndef MATRIX_H
#define MATRIX_H

#include <cmath>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
    
    // Dot product
    float dot(const Vec4& v) const {
        return x * v.x + y * v.y + z * v.z;
    }
    
    // Cross product
    Vec4 cross(const Vec4& v) const {
        return Vec4(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }
};

// 4x4 Matrix structure
// Stored as m[col][row] to match OpenGL's column-major order
struct Matrix4x4 {
    float m[4][4];
    
    Matrix4x4() {
        setIdentity();
    }
    
    // Initialize as identity matrix
    void setIdentity() {
        memset(m, 0, sizeof(m));
        m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
    }
    
    // Access for glLoadMatrixf
    const float* ptr() const {
        return &m[0][0];
    }

    // Matrix multiplication: Result = This * Other
    // Note: OpenGL uses column-major order.
    // If we want to apply T then R then S, we usually do M = T * R * S in math notation (if v is column vector)
    // But in OpenGL memory layout (column-major), if we store them as such, the multiplication logic needs to be correct.
    // Standard matrix multiplication C[i][j] = Sum(A[i][k] * B[k][j]) works if we treat them as standard matrices.
    // Let's stick to standard row/col multiplication logic.
    Matrix4x4 operator*(const Matrix4x4& other) const {
        Matrix4x4 result;
        // Initialize to 0
        memset(result.m, 0, sizeof(result.m));
        
        for (int col = 0; col < 4; col++) {
            for (int row = 0; row < 4; row++) {
                for (int k = 0; k < 4; k++) {
                    // m[col][row]
                    result.m[col][row] += m[k][row] * other.m[col][k];
                }
            }
        }
        return result;
    }
};

// ============================================================================
// TRANSLATION MATRIX
// | 1  0  0  tx |
// | 0  1  0  ty |
// | 0  0  1  tz |
// | 0  0  0  1  |
// ============================================================================
inline Matrix4x4 createTranslationMatrix(float tx, float ty, float tz) {
    Matrix4x4 mat;
    // Col 3 is translation vector
    mat.m[3][0] = tx;
    mat.m[3][1] = ty;
    mat.m[3][2] = tz;
    return mat;
}

// ============================================================================
// SCALE MATRIX
// | sx 0  0  0 |
// | 0  sy 0  0 |
// | 0  0  sz 0 |
// | 0  0  0  1 |
// ============================================================================
inline Matrix4x4 createScaleMatrix(float sx, float sy, float sz) {
    Matrix4x4 mat;
    mat.m[0][0] = sx;
    mat.m[1][1] = sy;
    mat.m[2][2] = sz;
    return mat;
}

// ============================================================================
// ROTATION MATRIX AROUND Y AXIS
// | cos  0  sin  0 |
// | 0    1  0    0 |
// | -sin 0  cos  0 |
// | 0    0  0    1 |
// ============================================================================
inline Matrix4x4 createRotationYMatrix(float angle) {
    Matrix4x4 mat;
    float c = cos(angle);
    float s = sin(angle);
    
    // Col 0
    mat.m[0][0] = c;
    mat.m[0][2] = -s;
    
    // Col 2
    mat.m[2][0] = s;
    mat.m[2][2] = c;
    
    return mat;
}

// ============================================================================
// ROTATION MATRIX AROUND X AXIS
// | 1  0    0    0 |
// | 0  cos -sin  0 |
// | 0  sin  cos  0 |
// | 0  0    0    1 |
// ============================================================================
inline Matrix4x4 createRotationXMatrix(float angle) {
    Matrix4x4 mat;
    float c = cos(angle);
    float s = sin(angle);
    
    // Col 1
    mat.m[1][1] = c;
    mat.m[1][2] = s;
    
    // Col 2
    mat.m[2][1] = -s;
    mat.m[2][2] = c;
    
    return mat;
}

// ============================================================================
// ROTATION MATRIX AROUND Z AXIS
// | cos -sin 0  0 |
// | sin  cos 0  0 |
// | 0    0   1  0 |
// | 0    0   0  1 |
// ============================================================================
inline Matrix4x4 createRotationZMatrix(float angle) {
    Matrix4x4 mat;
    float c = cos(angle);
    float s = sin(angle);
    
    // Col 0
    mat.m[0][0] = c;
    mat.m[0][1] = s;
    
    // Col 1
    mat.m[1][0] = -s;
    mat.m[1][1] = c;
    
    return mat;
}

// ============================================================================
// ROTATION MATRIX AROUND ARBITRARY AXIS
// Uses Rodrigues' rotation formula concept adapted for matrix
// ============================================================================
inline Matrix4x4 createRotationAxisMatrix(float angle, float uX, float uY, float uZ) {
    Matrix4x4 mat;
    float c = cos(angle);
    float s = sin(angle);
    float one_c = 1.0f - c;
    
    // Normalize axis
    float len = sqrt(uX*uX + uY*uY + uZ*uZ);
    if (len > 0.0001f) {
        uX /= len; uY /= len; uZ /= len;
    }
    
    float x = uX, y = uY, z = uZ;
    
    // Col 0
    mat.m[0][0] = x*x*one_c + c;
    mat.m[0][1] = y*x*one_c + z*s;
    mat.m[0][2] = x*z*one_c - y*s;
    
    // Col 1
    mat.m[1][0] = x*y*one_c - z*s;
    mat.m[1][1] = y*y*one_c + c;
    mat.m[1][2] = y*z*one_c + x*s;
    
    // Col 2
    mat.m[2][0] = x*z*one_c + y*s;
    mat.m[2][1] = y*z*one_c - x*s;
    mat.m[2][2] = z*z*one_c + c;
    
    return mat;
}

// ============================================================================
// VIEW MATRIX (Replaces gluLookAt)
// Constructs a view matrix from eye position, target, and up vector
// ============================================================================
inline Matrix4x4 createLookAtMatrix(const Vec4& eye, const Vec4& center, const Vec4& up) {
    Vec4 f = center - eye;
    f.normalize();
    
    Vec4 u = up;
    u.normalize();
    
    Vec4 s = f.cross(u);
    s.normalize();
    
    u = s.cross(f);
    
    Matrix4x4 mat;
    
    // Rotation part (transposed basis)
    // Col 0
    mat.m[0][0] = s.x;
    mat.m[0][1] = u.x;
    mat.m[0][2] = -f.x;
    
    // Col 1
    mat.m[1][0] = s.y;
    mat.m[1][1] = u.y;
    mat.m[1][2] = -f.y;
    
    // Col 2
    mat.m[2][0] = s.z;
    mat.m[2][1] = u.z;
    mat.m[2][2] = -f.z;
    
    // Translation part (Col 3)
    // -dot(s, eye), -dot(u, eye), dot(f, eye)
    mat.m[3][0] = -s.dot(eye);
    mat.m[3][1] = -u.dot(eye);
    mat.m[3][2] = f.dot(eye);
    mat.m[3][3] = 1.0f;
    
    return mat;
}

// ============================================================================
// PERSPECTIVE PROJECTION MATRIX (Replaces gluPerspective)
// ============================================================================
inline Matrix4x4 createPerspectiveMatrix(float fovY, float aspect, float zNear, float zFar) {
    Matrix4x4 mat;
    memset(mat.m, 0, sizeof(mat.m)); // Clear all to 0
    
    float f = 1.0f / tan(fovY * M_PI / 360.0f); // cot(fov/2)
    
    mat.m[0][0] = f / aspect;
    mat.m[1][1] = f;
    mat.m[2][2] = (zFar + zNear) / (zNear - zFar);
    mat.m[2][3] = -1.0f;
    mat.m[3][2] = (2.0f * zFar * zNear) / (zNear - zFar);
    mat.m[3][3] = 0.0f;
    
    return mat;
}

#endif 