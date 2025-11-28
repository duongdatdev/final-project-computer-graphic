/*******************************************************************************
 * THE SHIFTING MAZE - Main Entry Point
 * 
 * A 3D maze game using C++ and OpenGL (fixed-function pipeline)
 * 
 * This file only contains:
 * - OpenGL/GLUT initialization
 * - Callback registration
 * - Main loop
 * 
 * All game logic is in game.h
 * All drawing functions are in draw.h
 * Configuration is in config.h
 * 
 * ============================================================================
 * COMPUTER GRAPHICS ALGORITHMS IMPLEMENTED:
 * ============================================================================
 * 
 * 1. 3D TRANSFORMATIONS (matrix.h)
 *    - Translation Matrix
 *    - Scale Matrix
 *    - Rotation X/Y/Z Matrices
 *    - Arbitrary Axis Rotation: T = Tr(-P0)·Rx(φ)·Ry(-θ)·Rz(α)·Ry(θ)·Rx(-φ)·Tr(P0)
 *    - Matrix Multiplication: M = M1 · M2 · M3
 * 
 * 2. 3D VIEWING (camera.h)
 *    - WCS to Observer: (x0,y0,z0,1) = (x,y,z,1) · A · B · C · D
 *    - θ (theta) and φ (phi) angles
 *    - Perspective Projection: x' = D/z0 · x0, y' = D/z0 · y0
 *    - Orthographic Projection: x' = x0, y' = y0
 * 
 * 3. CURVES & SURFACES (bezier.h)
 *    - De Casteljau Algorithm
 *    - Bernstein Polynomial: B_k^n(t) = C(n,k) · (1-t)^(n-k) · t^k
 *    - Bézier Curve: P(t) = Σ B_i^n(t) · P_i
 *    - Bézier Surface: P(u,v) = Σ_i Σ_j B_i^m(u) · B_j^n(v) · P_ij
 *    - Parametric Surface: z = sin(x/10) · cos(y/10)
 *    - Sphere: x = r·cos(θ)·sin(φ), y = r·cos(φ), z = r·sin(θ)·sin(φ)
 * 
 * 4. ADVANCED TECHNIQUES (rendering.h, lighting.h)
 *    - Back-face Culling: N · V < 0
 *    - Z-buffer: glEnable(GL_DEPTH_TEST)
 *    - Lambert Shading: I = Ia·Ka + Ip·Kd·max(0, N·L)
 *    - Gouraud Shading: glShadeModel(GL_SMOOTH)
 * 
 ******************************************************************************/

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "game.h"

// ============================================================================
// GLOBAL GAME INSTANCE
// ============================================================================
Game game;

// ============================================================================
// GLUT CALLBACKS
// ============================================================================

void display() {
    game.render();
}

void reshape(int w, int h) {
    game.handleResize(w, h);
}

void keyboard(unsigned char key, int x, int y) {
    game.handleKeyDown(key);
}

void keyboardUp(unsigned char key, int x, int y) {
    game.handleKeyUp(key);
}

void mouseMotion(int x, int y) {
    game.handleMouseMove(x, y);
    
    // Warp mouse back to center
    if (game.state == STATE_PLAYING) {
        game.input.mouseWarped = true;
        glutWarpPointer(game.input.windowCenterX, game.input.windowCenterY);
    }
}

void mouseButton(int button, int state, int x, int y) {
    // Could be used for shooting or interaction
}

void update(int value) {
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    game.update(currentTime);
    
    glutPostRedisplay();
    glutTimerFunc(Config::FRAME_TIME_MS, update, 0);
}

// ============================================================================
// OPENGL INITIALIZATION
// ============================================================================

void initOpenGL() {
    // Clear color
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    
    // Enable depth testing (Z-buffer)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Enable back-face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    
    // Enable color material
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    
    // Smooth shading (Gouraud)
    glShadeModel(GL_SMOOTH);
    
    // Enable normalization
    glEnable(GL_NORMALIZE);
}

// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(Config::WINDOW_TITLE);
    
    // Initialize OpenGL
    initOpenGL();
    
    // Initialize game
    game.init();
    game.printWelcome();
    
    // Register callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutPassiveMotionFunc(mouseMotion);
    glutMouseFunc(mouseButton);
    
    // Hide cursor
    glutSetCursor(GLUT_CURSOR_NONE);
    
    // Start timer
    game.lastTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    glutTimerFunc(Config::FRAME_TIME_MS, update, 0);
    
    // Start main loop
    glutMainLoop();
    
    return 0;
}
