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

// Draw a sphere with transformation
inline void drawSphere(float x, float y, float z, float radius) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glutSolidSphere(radius, 20, 20);
    glPopMatrix();
}

#endif // DRAW_H
