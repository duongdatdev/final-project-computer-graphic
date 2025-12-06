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

// Draw a unit cube centered at origin (1x1x1)
inline void drawUnitCube() {
    glBegin(GL_QUADS);
    
    // Front face (+Z)
    glNormal3f(0, 0, 1);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    
    // Back face (-Z)
    glNormal3f(0, 0, -1);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    
    // Top face (+Y)
    glNormal3f(0, 1, 0);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    
    // Bottom face (-Y)
    glNormal3f(0, -1, 0);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    
    // Right face (+X)
    glNormal3f(1, 0, 0);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    
    // Left face (-X)
    glNormal3f(-1, 0, 0);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    
    glEnd();
}

// Draw a sphere manually using parametric equation (CG.5)
// x = r * cos(phi) * cos(theta)
// y = r * sin(phi)
// z = r * cos(phi) * sin(theta)
inline void drawManualSphere(float radius, int slices, int stacks) {
    const float PI = 3.14159265359f;
    
    for (int i = 0; i < stacks; ++i) {
        float phi1 = -PI/2 + (float)i / stacks * PI;
        float phi2 = -PI/2 + (float)(i + 1) / stacks * PI;
        
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; ++j) {
            float theta = (float)j / slices * 2 * PI;
            
            float x1 = radius * cos(phi1) * cos(theta);
            float y1 = radius * sin(phi1);
            float z1 = radius * cos(phi1) * sin(theta);
            
            float x2 = radius * cos(phi2) * cos(theta);
            float y2 = radius * sin(phi2);
            float z2 = radius * cos(phi2) * sin(theta);
            
            // Normal vector is same as position vector (normalized) for sphere at origin
            glNormal3f(x1/radius, y1/radius, z1/radius);
            glVertex3f(x1, y1, z1);
            
            glNormal3f(x2/radius, y2/radius, z2/radius);
            glVertex3f(x2, y2, z2);
        }
        glEnd();
    }
}

// Draw a cube with transformation using manual matrix multiplication 
inline void drawCube(float x, float y, float z, float scaleX, float scaleY, float scaleZ, float rotY = 0) {
    glPushMatrix();
    
    // Manual Matrix Construction
    // M = T * R * S
    
    // 1. Scale
    Matrix4x4 S = createScaleMatrix(scaleX, scaleY, scaleZ);
    
    // 2. Rotate (Convert degrees to radians)
    Matrix4x4 R = createRotationYMatrix(rotY * 3.14159f / 180.0f);
    
    // 3. Translate
    Matrix4x4 T = createTranslationMatrix(x, y, z);
    
    // Combine: T * R * S
    // Note: In OpenGL column-major, this order applies S first, then R, then T to the vertex.
    Matrix4x4 M = T * R * S;
    
    // Apply to current matrix
    glMultMatrixf(M.ptr());
    
    drawUnitCube();
    glPopMatrix();
}

// Draw a sphere with transformation using manual matrix multiplication (CG.4)
inline void drawSphere(float x, float y, float z, float radius) {
    glPushMatrix();
    
    // Manual Translation Matrix
    Matrix4x4 T = createTranslationMatrix(x, y, z);
    glMultMatrixf(T.ptr());
    
    // Manual Sphere Drawing 
    drawManualSphere(radius, 20, 20);
    
    glPopMatrix();
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

// Draw Cone (Ruled Surface) (CG.5 2.1)
inline void drawManualCone(float radius, float height, int slices) {
    const float PI = 3.14159265359f;
    
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 1, 0); // Approximate normal at tip
    glVertex3f(0, height, 0); // Tip
    
    for (int i = 0; i <= slices; i++) {
        float theta = (float)i / slices * 2.0f * PI;
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        
        // Normal calculation for side
        // Slope vector is (x, -height, z)
        // Tangent is (-sin, 0, cos)
        // Normal is cross product... simplified:
        float ny = radius / height; // Approximate
        glNormal3f(x/radius, ny, z/radius);
        
        glVertex3f(x, 0, z);
    }
    glEnd();
    
    // Base
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, -1, 0);
    glVertex3f(0, 0, 0);
    for (int i = 0; i <= slices; i++) {
        float theta = -(float)i / slices * 2.0f * PI;
        glVertex3f(radius * cos(theta), 0, radius * sin(theta));
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
