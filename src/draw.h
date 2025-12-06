/*******************************************************************************
 * THE SHIFTING MAZE - Drawing Functions Header
 * 
 * Contains all primitive drawing functions using OpenGL fixed-function pipeline
 ******************************************************************************/

#ifndef DRAW_H
#define DRAW_H

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "config.h"
#include "matrix.h"
#include "lighting.h"

#include <cmath>
#include <cstdlib>

// ============================================================================
// PRIMITIVE DRAWING FUNCTIONS
// ============================================================================

// Helper to transform vector by matrix
inline Vec4 transform(const Matrix4x4& m, const Vec4& v) {
    float x = m.m[0][0]*v.x + m.m[1][0]*v.y + m.m[2][0]*v.z + m.m[3][0]*v.w;
    float y = m.m[0][1]*v.x + m.m[1][1]*v.y + m.m[2][1]*v.z + m.m[3][1]*v.w;
    float z = m.m[0][2]*v.x + m.m[1][2]*v.y + m.m[2][2]*v.z + m.m[3][2]*v.w;
    float w = m.m[0][3]*v.x + m.m[1][3]*v.y + m.m[2][3]*v.z + m.m[3][3]*v.w;
    if (w != 0 && w != 1) { x/=w; y/=w; z/=w; }
    return Vec4(x, y, z);
}

// Draw a unit cube with manual lighting and back-face culling
inline void drawUnitCubeManual(const Matrix4x4& M, const Vec4& viewPos, const Light& light, const Material& material) {
    // Vertices of a unit cube centered at origin
    Vec4 v[8] = {
        Vec4(-0.5f, -0.5f, 0.5f), Vec4(0.5f, -0.5f, 0.5f), Vec4(0.5f, 0.5f, 0.5f), Vec4(-0.5f, 0.5f, 0.5f), // Front
        Vec4(-0.5f, -0.5f, -0.5f), Vec4(0.5f, -0.5f, -0.5f), Vec4(0.5f, 0.5f, -0.5f), Vec4(-0.5f, 0.5f, -0.5f) // Back
    };
    
    // Faces (indices)
    int faces[6][4] = {
        {0, 1, 2, 3}, // Front (+Z)
        {5, 4, 7, 6}, // Back (-Z)
        {3, 2, 6, 7}, // Top (+Y)
        {4, 5, 1, 0}, // Bottom (-Y)
        {1, 5, 6, 2}, // Right (+X)
        {4, 0, 3, 7}  // Left (-X)
    };

    glBegin(GL_QUADS);
    for (int i = 0; i < 6; i++) {
        // Transform vertices to world space
        Vec4 p0 = transform(M, v[faces[i][0]]);
        Vec4 p1 = transform(M, v[faces[i][1]]);
        Vec4 p2 = transform(M, v[faces[i][2]]);
        Vec4 p3 = transform(M, v[faces[i][3]]);
        
        // Back-face culling
        if (!isFaceVisible(p0, p1, p2, viewPos)) continue;
        
        // Draw face
        Vec4 p[4] = {p0, p1, p2, p3};
        
        // Transform normal (assuming uniform scale, M is fine for direction)
        // For correct normal transform with non-uniform scale, we need inverse transpose.
        // But here we assume uniform or simple scaling.
        // Actually, let's recalculate normal from world vertices to be safe and "manual"
        Vec4 normal = (p1 - p0).cross(p2 - p0);
        normal.normalize();

        for (int j = 0; j < 4; j++) {
            // Calculate lighting
            Color c = calculateLighting(p[j], normal, viewPos, light, material);
            glColor3f(c.r, c.g, c.b);
            glVertex3f(p[j].x, p[j].y, p[j].z);
        }
    }
    glEnd();
}

// Draw a sphere manually with lighting
inline void drawManualSphereManual(float radius, int slices, int stacks, const Matrix4x4& M, 
                                 const Vec4& viewPos, const Light& light, const Material& material) {
    const float PI = 3.14159265359f;
    
    for (int i = 0; i < stacks; ++i) {
        float phi1 = -PI/2 + (float)i / stacks * PI;
        float phi2 = -PI/2 + (float)(i + 1) / stacks * PI;
        
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; ++j) {
            float theta = (float)j / slices * 2 * PI;
            
            // Vertices in local space
            float x1 = radius * cos(phi1) * cos(theta);
            float y1 = radius * sin(phi1);
            float z1 = radius * cos(phi1) * sin(theta);
            
            float x2 = radius * cos(phi2) * cos(theta);
            float y2 = radius * sin(phi2);
            float z2 = radius * cos(phi2) * sin(theta);
            
            Vec4 v1(x1, y1, z1);
            Vec4 v2(x2, y2, z2);
            
            // Transform to world space
            Vec4 w1 = transform(M, v1);
            Vec4 w2 = transform(M, v2);
            
            // Normals (local) - for sphere, normal is same as position (normalized)
            Vec4 n1 = v1; n1.normalize(); n1.w = 0.0f; // Set w=0 for direction vector
            Vec4 n2 = v2; n2.normalize(); n2.w = 0.0f;
            
            // Transform normals (using M for rotation)
            // Since w=0, translation in M will be ignored by transform()
            Vec4 wn1 = transform(M, n1); wn1.normalize();
            Vec4 wn2 = transform(M, n2); wn2.normalize();
            
            // Lighting
            Color c1 = calculateLighting(w1, wn1, viewPos, light, material);
            glColor3f(c1.r, c1.g, c1.b);
            glVertex3f(w1.x, w1.y, w1.z);
            
            Color c2 = calculateLighting(w2, wn2, viewPos, light, material);
            glColor3f(c2.r, c2.g, c2.b);
            glVertex3f(w2.x, w2.y, w2.z);
        }
        glEnd();
    }
}

// Draw a cube with transformation using manual matrix multiplication 
inline void drawCube(float x, float y, float z, float scaleX, float scaleY, float scaleZ, 
                     const Vec4& viewPos, const Light& light, const Material& material, float rotY = 0) {
    // Manual Matrix Construction
    // M = T * R * S
    
    // 1. Scale
    Matrix4x4 S = createScaleMatrix(scaleX, scaleY, scaleZ);
    
    // 2. Rotate (Convert degrees to radians)
    Matrix4x4 R = createRotationYMatrix(rotY * 3.14159f / 180.0f);
    
    // 3. Translate
    Matrix4x4 T = createTranslationMatrix(x, y, z);
    
    // Combine: T * R * S
    Matrix4x4 M = T * R * S;
    
    drawUnitCubeManual(M, viewPos, light, material);
}

// Draw a sphere with transformation using manual matrix multiplication (CG.4)
inline void drawSphere(float x, float y, float z, float radius,
                       const Vec4& viewPos, const Light& light, const Material& material) {
    // Manual Translation Matrix
    Matrix4x4 T = createTranslationMatrix(x, y, z);
    
    drawManualSphereManual(radius, 20, 20, T, viewPos, light, material);
}

// ============================================================================
// CG.3 - 2D GRAPHICS ALGORITHMS
// ============================================================================

// Bresenham Line Algorithm (CG.3 1.1)
// Draws a line from (x1, y1) to (x2, y2) in 2D space (Z=0)
inline void drawLineBresenham(int x1, int y1, int x2, int y2) {
    glBegin(GL_POINTS);
    
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        glVertex2i(x1, y1);
        
        if (x1 == x2 && y1 == y2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
    
    glEnd();
}

// Midpoint Circle Algorithm (CG.3 1.2)
// Draws a circle centered at (xc, yc) with radius r
inline void drawCircleMidpoint(int xc, int yc, int r) {
    glBegin(GL_POINTS);
    
    int x = 0;
    int y = r;
    int p = 1 - r; // Initial decision parameter
    
    // Plot initial points
    glVertex2i(xc + x, yc + y);
    glVertex2i(xc - x, yc + y);
    glVertex2i(xc + x, yc - y);
    glVertex2i(xc - x, yc - y);
    glVertex2i(xc + y, yc + x);
    glVertex2i(xc - y, yc + x);
    glVertex2i(xc + y, yc - x);
    glVertex2i(xc - y, yc - x);
    
    while (x < y) {
        x++;
        if (p < 0) {
            p += 2 * x + 1;
        } else {
            y--;
            p += 2 * (x - y) + 1;
        }
        
        // Plot points in all 8 octants
        glVertex2i(xc + x, yc + y);
        glVertex2i(xc - x, yc + y);
        glVertex2i(xc + x, yc - y);
        glVertex2i(xc - x, yc - y);
        glVertex2i(xc + y, yc + x);
        glVertex2i(xc - y, yc + x);
        glVertex2i(xc + y, yc - x);
        glVertex2i(xc - y, yc - x);
    }
    
    glEnd();
}

// ============================================================================
// CG.5 - CURVES AND SURFACES
// ============================================================================

// Draw Cylinder (Ruled Surface) (CG.5 2.1)
// Parametric equation: x = r*cos(u), z = r*sin(u), y = v
inline void drawManualCylinder(float radius, float height, int slices) {
    const float PI = 3.14159265359f;
    float halfHeight = height / 2.0f;
    
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; i++) {
        float theta = (float)i / slices * 2.0f * PI;
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        
        glNormal3f(x/radius, 0, z/radius);
        glVertex3f(x, -halfHeight, z);
        glVertex3f(x, halfHeight, z);
    }
    glEnd();
    
    // Caps
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 1, 0);
    glVertex3f(0, halfHeight, 0);
    for (int i = 0; i <= slices; i++) {
        float theta = (float)i / slices * 2.0f * PI;
        glVertex3f(radius * cos(theta), halfHeight, radius * sin(theta));
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, -1, 0);
    glVertex3f(0, -halfHeight, 0);
    for (int i = 0; i <= slices; i++) {
        float theta = -(float)i / slices * 2.0f * PI;
        glVertex3f(radius * cos(theta), -halfHeight, radius * sin(theta));
    }
    glEnd();
}

// Draw Cone manually with lighting
inline void drawManualConeManual(float radius, float height, int slices, const Matrix4x4& M,
                               const Vec4& viewPos, const Light& light, const Material& material) {
    const float PI = 3.14159265359f;
    
    // Cone Side
    glBegin(GL_TRIANGLE_FAN);
    
    // Tip
    Vec4 tipLocal(0, height, 0);
    Vec4 tipWorld = transform(M, tipLocal);
    Vec4 tipNormalLocal(0, 1, 0); tipNormalLocal.w = 0;
    Vec4 tipNormalWorld = transform(M, tipNormalLocal); tipNormalWorld.normalize();
    
    Color cTip = calculateLighting(tipWorld, tipNormalWorld, viewPos, light, material);
    glColor3f(cTip.r, cTip.g, cTip.b);
    glVertex3f(tipWorld.x, tipWorld.y, tipWorld.z);
    
    for (int i = 0; i <= slices; i++) {
        float theta = (float)i / slices * 2.0f * PI;
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        
        Vec4 vLocal(x, 0, z);
        Vec4 vWorld = transform(M, vLocal);
        
        // Normal calculation
        float ny = radius / height;
        Vec4 nLocal(x/radius, ny, z/radius); nLocal.w = 0;
        Vec4 nWorld = transform(M, nLocal); nWorld.normalize();
        
        Color c = calculateLighting(vWorld, nWorld, viewPos, light, material);
        glColor3f(c.r, c.g, c.b);
        glVertex3f(vWorld.x, vWorld.y, vWorld.z);
    }
    glEnd();
    
    // Base
    glBegin(GL_TRIANGLE_FAN);
    
    // Center of base
    Vec4 baseCenterLocal(0, 0, 0);
    Vec4 baseCenterWorld = transform(M, baseCenterLocal);
    Vec4 baseNormalLocal(0, -1, 0); baseNormalLocal.w = 0;
    Vec4 baseNormalWorld = transform(M, baseNormalLocal); baseNormalWorld.normalize();
    
    Color cBase = calculateLighting(baseCenterWorld, baseNormalWorld, viewPos, light, material);
    glColor3f(cBase.r, cBase.g, cBase.b);
    glVertex3f(baseCenterWorld.x, baseCenterWorld.y, baseCenterWorld.z);
    
    for (int i = 0; i <= slices; i++) {
        float theta = -(float)i / slices * 2.0f * PI;
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        
        Vec4 vLocal(x, 0, z);
        Vec4 vWorld = transform(M, vLocal);
        
        // Normal is same as center for flat base
        Color c = calculateLighting(vWorld, baseNormalWorld, viewPos, light, material);
        glColor3f(c.r, c.g, c.b);
        glVertex3f(vWorld.x, vWorld.y, vWorld.z);
    }
    glEnd();
}

// Draw Torus (Surface of Revolution) (CG.5 2.2)
// x = (R + r*cos(v)) * cos(u)
// y = (R + r*cos(v)) * sin(u)
// z = r * sin(v)
// Note: In our system Y is up, so we swap y and z usually, or rotate.
// Let's implement standard torus lying on XZ plane.
inline void drawManualTorus(float innerRadius, float outerRadius, int nsides, int rings) {
    const float PI = 3.14159265359f;
    float ringRadius = (outerRadius - innerRadius) / 2.0f;
    float centerRadius = innerRadius + ringRadius;
    
    for (int i = 0; i < rings; i++) {
        float theta = (float)i / rings * 2.0f * PI;
        float nextTheta = (float)(i + 1) / rings * 2.0f * PI;
        
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= nsides; j++) {
            float phi = (float)j / nsides * 2.0f * PI;
            
            float cosPhi = cos(phi);
            float sinPhi = sin(phi);
            float cosTheta = cos(theta);
            float sinTheta = sin(theta);
            float cosNextTheta = cos(nextTheta);
            float sinNextTheta = sin(nextTheta);
            
            // Current ring
            float x = (centerRadius + ringRadius * cosPhi) * cosTheta;
            float z = (centerRadius + ringRadius * cosPhi) * sinTheta;
            float y = ringRadius * sinPhi;
            
            // Normal
            glNormal3f(cosPhi * cosTheta, sinPhi, cosPhi * sinTheta);
            glVertex3f(x, y, z);
            
            // Next ring
            float nx = (centerRadius + ringRadius * cosPhi) * cosNextTheta;
            float nz = (centerRadius + ringRadius * cosPhi) * sinNextTheta;
            float ny = ringRadius * sinPhi;
            
            glNormal3f(cosPhi * cosNextTheta, sinPhi, cosPhi * sinNextTheta);
            glVertex3f(nx, ny, nz);
        }
        glEnd();
    }
}

// Bezier Curve (CG.5 1.1)
// P(t) = (1-t)^3*P0 + 3(1-t)^2*t*P1 + 3(1-t)*t^2*P2 + t^3*P3
inline void drawBezierCurve(Vec4 p0, Vec4 p1, Vec4 p2, Vec4 p3, int segments) {
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= segments; i++) {
        float t = (float)i / segments;
        float u = 1.0f - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;
        
        Vec4 p = p0 * uuu + p1 * (3 * uu * t) + p2 * (3 * u * tt) + p3 * ttt;
        glVertex3f(p.x, p.y, p.z);
    }
    glEnd();
}

#endif // DRAW_H
