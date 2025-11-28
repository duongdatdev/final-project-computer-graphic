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

// Draw a cube with transformation
inline void drawCube(float x, float y, float z, float scaleX, float scaleY, float scaleZ, float rotY = 0) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(rotY, 0, 1, 0);
    glScalef(scaleX, scaleY, scaleZ);
    drawUnitCube();
    glPopMatrix();
}

// ============================================================================
// SPHERE DRAWING - Using Parametric Surface Formula
// x = r * cos(θ) * sin(φ)
// y = r * cos(φ)  
// z = r * sin(θ) * sin(φ)
// ============================================================================
inline void drawSphere(float radius, int slices, int stacks) {
    for (int i = 0; i < stacks; i++) {
        float phi1 = M_PI * i / stacks;
        float phi2 = M_PI * (i + 1) / stacks;
        
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; j++) {
            float theta = 2.0f * M_PI * j / slices;
            
            // Vertex 1
            float x1 = radius * cos(theta) * sin(phi1);
            float y1 = radius * cos(phi1);
            float z1 = radius * sin(theta) * sin(phi1);
            glNormal3f(x1 / radius, y1 / radius, z1 / radius);
            glVertex3f(x1, y1, z1);
            
            // Vertex 2
            float x2 = radius * cos(theta) * sin(phi2);
            float y2 = radius * cos(phi2);
            float z2 = radius * sin(theta) * sin(phi2);
            glNormal3f(x2 / radius, y2 / radius, z2 / radius);
            glVertex3f(x2, y2, z2);
        }
        glEnd();
    }
}

// Draw sphere at position
inline void drawSphereAt(float x, float y, float z, float radius, int slices = 16, int stacks = 8) {
    glPushMatrix();
    glTranslatef(x, y, z);
    drawSphere(radius, slices, stacks);
    glPopMatrix();
}

// ============================================================================
// CYLINDER DRAWING - Using Parametric Surface
// x = r * cos(θ)
// y = h
// z = r * sin(θ)
// ============================================================================
inline void drawCylinder(float radius, float height, int slices) {
    float halfHeight = height / 2.0f;
    
    // Side surface
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; i++) {
        float theta = 2.0f * M_PI * i / slices;
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        
        glNormal3f(cos(theta), 0, sin(theta));
        glVertex3f(x, -halfHeight, z);
        glVertex3f(x, halfHeight, z);
    }
    glEnd();
    
    // Top cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 1, 0);
    glVertex3f(0, halfHeight, 0);
    for (int i = 0; i <= slices; i++) {
        float theta = 2.0f * M_PI * i / slices;
        glVertex3f(radius * cos(theta), halfHeight, radius * sin(theta));
    }
    glEnd();
    
    // Bottom cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, -1, 0);
    glVertex3f(0, -halfHeight, 0);
    for (int i = slices; i >= 0; i--) {
        float theta = 2.0f * M_PI * i / slices;
        glVertex3f(radius * cos(theta), -halfHeight, radius * sin(theta));
    }
    glEnd();
}

// ============================================================================
// TEXT DRAWING
// ============================================================================
inline void drawText(float x, float y, const char* text, void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(font, *text);
        text++;
    }
}

inline void drawText3D(float x, float y, float z, const char* text, void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos3f(x, y, z);
    while (*text) {
        glutBitmapCharacter(font, *text);
        text++;
    }
}

// ============================================================================
// 2D SHAPE DRAWING (for HUD)
// ============================================================================
inline void drawQuad2D(float x, float y, float width, float height) {
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

inline void drawCircle2D(float x, float y, float radius, int segments = 16) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        glVertex2f(x + cos(angle) * radius, y + sin(angle) * radius);
    }
    glEnd();
}

// ============================================================================
// GRID/FLOOR DRAWING
// ============================================================================
inline void drawGrid(float size, int divisions, float y = 0) {
    float step = size / divisions;
    float half = size / 2;
    
    glBegin(GL_LINES);
    for (int i = 0; i <= divisions; i++) {
        float pos = -half + i * step;
        // X-axis lines
        glVertex3f(-half, y, pos);
        glVertex3f(half, y, pos);
        // Z-axis lines
        glVertex3f(pos, y, -half);
        glVertex3f(pos, y, half);
    }
    glEnd();
}

#endif // DRAW_H
