/*******************************************************************************
 * THE SHIFTING MAZE - Rendering Techniques Header
 * 
 * Implements advanced graphics techniques as described in Computer Graphics course:
 * - Back-face Culling using dot product
 * - Z-buffer (depth buffer) algorithm
 ******************************************************************************/

#ifndef RENDERING_H
#define RENDERING_H

#include "matrix.h"
#include <vector>
#include <algorithm>

// ============================================================================
// BACK-FACE CULLING - Loại bỏ mặt khuất
// 
// A face is back-facing if the dot product of:
// - Surface normal (N)
// - View direction (V = viewpoint - surface point)
// is negative: N · V < 0
// 
// This means the face is pointing away from the viewer.
// ============================================================================
class BackFaceCulling {
public:
    // Check if a face should be culled (is back-facing)
    // Returns true if the face should NOT be rendered
    static bool shouldCull(const Vec4& faceNormal, const Vec4& faceCenter, const Vec4& viewPoint) {
        // View direction: V = viewPoint - faceCenter
        Vec4 V;
        V.x = viewPoint.x - faceCenter.x;
        V.y = viewPoint.y - faceCenter.y;
        V.z = viewPoint.z - faceCenter.z;
        
        // Dot product: N · V
        float dotProduct = faceNormal.dot(V);
        
        // If dot product < 0, face is pointing away from viewer
        return dotProduct < 0;
    }
    
    // Calculate face normal from 3 vertices (counter-clockwise order)
    // N = (V1 - V0) × (V2 - V0)
    static Vec4 calculateFaceNormal(const Vec4& v0, const Vec4& v1, const Vec4& v2) {
        Vec4 edge1;
        edge1.x = v1.x - v0.x;
        edge1.y = v1.y - v0.y;
        edge1.z = v1.z - v0.z;
        
        Vec4 edge2;
        edge2.x = v2.x - v0.x;
        edge2.y = v2.y - v0.y;
        edge2.z = v2.z - v0.z;
        
        Vec4 normal = edge1.cross(edge2);
        normal.normalize();
        return normal;
    }
    
    // Calculate face center (centroid)
    static Vec4 calculateFaceCenter(const Vec4& v0, const Vec4& v1, const Vec4& v2) {
        return Vec4(
            (v0.x + v1.x + v2.x) / 3.0f,
            (v0.y + v1.y + v2.y) / 3.0f,
            (v0.z + v1.z + v2.z) / 3.0f
        );
    }
    
    static Vec4 calculateFaceCenter(const Vec4& v0, const Vec4& v1, const Vec4& v2, const Vec4& v3) {
        return Vec4(
            (v0.x + v1.x + v2.x + v3.x) / 4.0f,
            (v0.y + v1.y + v2.y + v3.y) / 4.0f,
            (v0.z + v1.z + v2.z + v3.z) / 4.0f
        );
    }
};

// ============================================================================
// Z-BUFFER (DEPTH BUFFER) ALGORITHM
// 
// For each pixel:
// 1. Initialize depth buffer with maximum depth (far plane)
// 2. For each polygon:
//    a. For each pixel covered by the polygon:
//       - Calculate depth z at that pixel
//       - If z < zBuffer[x][y]:
//         - Update frame buffer with pixel color
//         - Update zBuffer[x][y] = z
// ============================================================================
class ZBuffer {
public:
    int width, height;
    std::vector<float> buffer;
    float farValue;
    
    ZBuffer() : width(0), height(0), farValue(1000.0f) {}
    
    ZBuffer(int w, int h) : width(w), height(h), farValue(1000.0f) {
        buffer.resize(w * h, farValue);
    }
    
    // Initialize buffer to far plane value
    void clear() {
        std::fill(buffer.begin(), buffer.end(), farValue);
    }
    
    // Resize buffer
    void resize(int w, int h) {
        width = w;
        height = h;
        buffer.resize(w * h, farValue);
        clear();
    }
    
    // Get depth at pixel (x, y)
    float getDepth(int x, int y) const {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            return buffer[y * width + x];
        }
        return farValue;
    }
    
    // Set depth at pixel (x, y)
    void setDepth(int x, int y, float depth) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            buffer[y * width + x] = depth;
        }
    }
    
    // Test and set: returns true if the new depth is closer
    bool testAndSet(int x, int y, float depth) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            int index = y * width + x;
            if (depth < buffer[index]) {
                buffer[index] = depth;
                return true;
            }
        }
        return false;
    }
    
    // Interpolate depth across a triangle
    static float interpolateDepth(
        float z0, float z1, float z2,
        float u, float v, float w
    ) {
        return z0 * u + z1 * v + z2 * w;
    }
};

// ============================================================================
// RENDER POLYGON - Simple polygon structure for rendering
// (Named RenderPolygon to avoid conflict with Windows Polygon macro)
// ============================================================================
struct RenderPolygon {
    std::vector<Vec4> vertices;
    Vec4 normal;
    Vec4 center;
    float depth;  // For painter's algorithm sorting
    
    RenderPolygon() : depth(0) {}
    
    // Calculate normal and center
    void calculateProperties() {
        if (vertices.size() >= 3) {
            normal = BackFaceCulling::calculateFaceNormal(
                vertices[0], vertices[1], vertices[2]
            );
            
            center = Vec4(0, 0, 0);
            for (const auto& v : vertices) {
                center.x += v.x;
                center.y += v.y;
                center.z += v.z;
            }
            float n = (float)vertices.size();
            center.x /= n;
            center.y /= n;
            center.z /= n;
            
            depth = center.z;
        }
    }
};

// ============================================================================
// PAINTER'S ALGORITHM (alternative to Z-buffer)
// Sort polygons by depth and render back-to-front
// ============================================================================
class PaintersAlgorithm {
public:
    static void sortByDepth(std::vector<RenderPolygon>& polygons) {
        // Sort by depth (farthest first)
        std::sort(polygons.begin(), polygons.end(),
            [](const RenderPolygon& a, const RenderPolygon& b) {
                return a.depth > b.depth;
            }
        );
    }
};

#endif // RENDERING_H
