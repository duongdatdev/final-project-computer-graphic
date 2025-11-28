/*******************************************************************************
 * THE SHIFTING MAZE - Bézier Curves and Surfaces Header
 * 
 * Implements curves and surfaces exactly as described in Computer Graphics course:
 * - De Casteljau algorithm
 * - Bernstein polynomial: B_k^n(t) = C(n,k) * (1-t)^(n-k) * t^k
 * - Bézier Curve: P(t) = Σ B_i^n(t) * P_i
 * - Bézier Surface: P(u,v) = Σ_i Σ_j B_i^m(u) * B_j^n(v) * P_ij
 * - Parametric Surfaces (sin/cos terrain)
 ******************************************************************************/

#ifndef BEZIER_H
#define BEZIER_H

#include "matrix.h"
#include <vector>
#include <cmath>

// ============================================================================
// BINOMIAL COEFFICIENT - C(n, k) = n! / (k! * (n-k)!)
// ============================================================================
inline int binomial(int n, int k) {
    if (k > n) return 0;
    if (k == 0 || k == n) return 1;
    
    int result = 1;
    for (int i = 0; i < k; i++) {
        result = result * (n - i) / (i + 1);
    }
    return result;
}

// ============================================================================
// BERNSTEIN POLYNOMIAL - Đa thức Bernstein
// B_k^n(t) = C(n,k) * (1-t)^(n-k) * t^k
// ============================================================================
inline float bernstein(int n, int k, float t) {
    return binomial(n, k) * pow(1.0f - t, n - k) * pow(t, k);
}

// ============================================================================
// DE CASTELJAU ALGORITHM - Thuật toán De Casteljau
// Recursive subdivision to compute point on Bézier curve
// ============================================================================
class DeCasteljau {
public:
    // Compute point on curve using De Casteljau algorithm
    static Vec4 compute(const std::vector<Vec4>& controlPoints, float t) {
        std::vector<Vec4> points = controlPoints;
        int n = points.size();
        
        // Iteratively compute intermediate points
        for (int r = 1; r < n; r++) {
            for (int i = 0; i < n - r; i++) {
                // Linear interpolation: P_i^r = (1-t) * P_i^(r-1) + t * P_(i+1)^(r-1)
                points[i].x = (1.0f - t) * points[i].x + t * points[i + 1].x;
                points[i].y = (1.0f - t) * points[i].y + t * points[i + 1].y;
                points[i].z = (1.0f - t) * points[i].z + t * points[i + 1].z;
            }
        }
        
        return points[0];
    }
};

// ============================================================================
// BÉZIER CURVE CLASS
// P(t) = Σ B_i^n(t) * P_i, where t ∈ [0, 1]
// ============================================================================
class BezierCurve {
public:
    std::vector<Vec4> controlPoints;
    
    BezierCurve() {}
    
    BezierCurve(const std::vector<Vec4>& points) : controlPoints(points) {}
    
    // Add control point
    void addPoint(const Vec4& p) {
        controlPoints.push_back(p);
    }
    
    // Clear all points
    void clear() {
        controlPoints.clear();
    }
    
    // ========================================================================
    // COMPUTE POINT USING BERNSTEIN FORMULA
    // P(t) = Σ_{i=0}^{n} B_i^n(t) * P_i
    // ========================================================================
    Vec4 computeBernstein(float t) const {
        int n = controlPoints.size() - 1;
        Vec4 result(0, 0, 0);
        
        for (int i = 0; i <= n; i++) {
            float B = bernstein(n, i, t);
            result.x += B * controlPoints[i].x;
            result.y += B * controlPoints[i].y;
            result.z += B * controlPoints[i].z;
        }
        
        return result;
    }
    
    // ========================================================================
    // COMPUTE POINT USING DE CASTELJAU ALGORITHM
    // ========================================================================
    Vec4 computeCasteljau(float t) const {
        return DeCasteljau::compute(controlPoints, t);
    }
    
    // Default compute uses Bernstein
    Vec4 compute(float t) const {
        return computeBernstein(t);
    }
    
    // Generate points along the curve
    std::vector<Vec4> generatePoints(int numPoints) const {
        std::vector<Vec4> points;
        for (int i = 0; i <= numPoints; i++) {
            float t = (float)i / numPoints;
            points.push_back(compute(t));
        }
        return points;
    }
    
    // Compute tangent at parameter t
    Vec4 computeTangent(float t) const {
        int n = controlPoints.size() - 1;
        Vec4 tangent(0, 0, 0);
        
        // Derivative of Bézier curve
        for (int i = 0; i < n; i++) {
            float B = bernstein(n - 1, i, t);
            tangent.x += n * B * (controlPoints[i + 1].x - controlPoints[i].x);
            tangent.y += n * B * (controlPoints[i + 1].y - controlPoints[i].y);
            tangent.z += n * B * (controlPoints[i + 1].z - controlPoints[i].z);
        }
        
        return tangent;
    }
};

// ============================================================================
// BÉZIER SURFACE CLASS
// P(u,v) = Σ_i Σ_j B_i^m(u) * B_j^n(v) * P_ij
// where u, v ∈ [0, 1]
// ============================================================================
class BezierSurface {
public:
    std::vector<std::vector<Vec4>> controlPoints;  // 2D grid of control points
    int m, n;  // Degrees in u and v directions
    
    BezierSurface() : m(0), n(0) {}
    
    BezierSurface(int _m, int _n) : m(_m), n(_n) {
        controlPoints.resize(m + 1);
        for (int i = 0; i <= m; i++) {
            controlPoints[i].resize(n + 1);
        }
    }
    
    // Set control point
    void setControlPoint(int i, int j, const Vec4& p) {
        if (i >= 0 && i <= m && j >= 0 && j <= n) {
            controlPoints[i][j] = p;
        }
    }
    
    // ========================================================================
    // COMPUTE POINT ON SURFACE USING BERNSTEIN FORMULA
    // P(u,v) = Σ_{i=0}^{m} Σ_{j=0}^{n} B_i^m(u) * B_j^n(v) * P_ij
    // ========================================================================
    Vec4 compute(float u, float v) const {
        Vec4 result(0, 0, 0);
        
        for (int i = 0; i <= m; i++) {
            float Bi = bernstein(m, i, u);
            for (int j = 0; j <= n; j++) {
                float Bj = bernstein(n, j, v);
                float weight = Bi * Bj;
                result.x += weight * controlPoints[i][j].x;
                result.y += weight * controlPoints[i][j].y;
                result.z += weight * controlPoints[i][j].z;
            }
        }
        
        return result;
    }
    
    // Compute normal at (u, v) using partial derivatives
    Vec4 computeNormal(float u, float v) const {
        // Compute partial derivatives
        Vec4 dPdu(0, 0, 0);
        Vec4 dPdv(0, 0, 0);
        
        // ∂P/∂u
        for (int i = 0; i < m; i++) {
            float dBi = m * (bernstein(m - 1, i, u));
            for (int j = 0; j <= n; j++) {
                float Bj = bernstein(n, j, v);
                float weight = dBi * Bj;
                dPdu.x += weight * (controlPoints[i + 1][j].x - controlPoints[i][j].x);
                dPdu.y += weight * (controlPoints[i + 1][j].y - controlPoints[i][j].y);
                dPdu.z += weight * (controlPoints[i + 1][j].z - controlPoints[i][j].z);
            }
        }
        
        // ∂P/∂v
        for (int i = 0; i <= m; i++) {
            float Bi = bernstein(m, i, u);
            for (int j = 0; j < n; j++) {
                float dBj = n * (bernstein(n - 1, j, v));
                float weight = Bi * dBj;
                dPdv.x += weight * (controlPoints[i][j + 1].x - controlPoints[i][j].x);
                dPdv.y += weight * (controlPoints[i][j + 1].y - controlPoints[i][j].y);
                dPdv.z += weight * (controlPoints[i][j + 1].z - controlPoints[i][j].z);
            }
        }
        
        // Normal = dPdu × dPdv
        Vec4 normal = dPdu.cross(dPdv);
        normal.normalize();
        return normal;
    }
};

// ============================================================================
// PARAMETRIC SURFACE - Bề mặt tham số
// z = sin(x/10) * cos(y/10) for terrain
// ============================================================================
class ParametricSurface {
public:
    float xMin, xMax, yMin, yMax;
    int resolutionX, resolutionY;
    float amplitude;
    float frequencyX, frequencyY;
    
    ParametricSurface() {
        xMin = -10; xMax = 10;
        yMin = -10; yMax = 10;
        resolutionX = 20;
        resolutionY = 20;
        amplitude = 0.3f;
        frequencyX = 0.1f;
        frequencyY = 0.1f;
    }
    
    // ========================================================================
    // COMPUTE HEIGHT AT (x, y)
    // z = amplitude * sin(x * freqX) * cos(y * freqY)
    // ========================================================================
    float computeHeight(float x, float y) const {
        return amplitude * sin(x * frequencyX) * cos(y * frequencyY);
    }
    
    // Compute normal at (x, y) using partial derivatives
    Vec4 computeNormal(float x, float y) const {
        // f(x,y) = A * sin(x*fx) * cos(y*fy)
        // ∂f/∂x = A * fx * cos(x*fx) * cos(y*fy)
        // ∂f/∂y = -A * fy * sin(x*fx) * sin(y*fy)
        
        float dfdx = amplitude * frequencyX * cos(x * frequencyX) * cos(y * frequencyY);
        float dfdy = -amplitude * frequencyY * sin(x * frequencyX) * sin(y * frequencyY);
        
        // Surface normal = (-∂f/∂x, 1, -∂f/∂y) normalized
        Vec4 normal(-dfdx, 1.0f, -dfdy);
        normal.normalize();
        return normal;
    }
    
    // Generate vertex at grid position
    Vec4 getVertex(int i, int j) const {
        float x = xMin + (xMax - xMin) * i / resolutionX;
        float y = yMin + (yMax - yMin) * j / resolutionY;
        float z = computeHeight(x, y);
        return Vec4(x, z, y);  // Note: y is vertical in OpenGL
    }
};

// ============================================================================
// SPHERE PARAMETRIC SURFACE
// x = r * cos(θ) * sin(φ)
// y = r * sin(θ) * sin(φ)
// z = r * cos(φ)
// ============================================================================
class SphereSurface {
public:
    Vec4 center;
    float radius;
    
    SphereSurface() : center(0, 0, 0), radius(1.0f) {}
    SphereSurface(const Vec4& c, float r) : center(c), radius(r) {}
    
    // Compute point on sphere
    // θ ∈ [0, 2π], φ ∈ [0, π]
    Vec4 compute(float theta, float phi) const {
        return Vec4(
            center.x + radius * cos(theta) * sin(phi),
            center.y + radius * cos(phi),
            center.z + radius * sin(theta) * sin(phi)
        );
    }
    
    // Normal at point (always points outward from center)
    Vec4 normal(float theta, float phi) const {
        return Vec4(
            cos(theta) * sin(phi),
            cos(phi),
            sin(theta) * sin(phi)
        );
    }
};

// ============================================================================
// CYLINDER PARAMETRIC SURFACE
// x = r * cos(θ)
// y = h
// z = r * sin(θ)
// ============================================================================
class CylinderSurface {
public:
    Vec4 base;
    float radius;
    float height;
    
    CylinderSurface() : base(0, 0, 0), radius(1.0f), height(2.0f) {}
    CylinderSurface(const Vec4& b, float r, float h) : base(b), radius(r), height(h) {}
    
    // Compute point on cylinder
    // θ ∈ [0, 2π], h ∈ [0, height]
    Vec4 compute(float theta, float h) const {
        return Vec4(
            base.x + radius * cos(theta),
            base.y + h,
            base.z + radius * sin(theta)
        );
    }
    
    // Normal at point (radial direction)
    Vec4 normal(float theta) const {
        return Vec4(cos(theta), 0, sin(theta));
    }
};

#endif // BEZIER_H
