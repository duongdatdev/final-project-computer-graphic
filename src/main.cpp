/*******************************************************************************
 * SIMPLE MAZE - Main Entry Point
 * 
 * A simple 3D maze game using C++ and OpenGL
 * 
 * This file only contains:
 * - OpenGL/GLUT initialization
 * - Callback registration
 * - Main loop
 * 
 * All game logic is in game.h
 * All drawing functions are in draw.h
 * Configuration is in config.h
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
    if (game.state == STATE_PLAYING) {
        game.handleMouseMove(x, y);
        
        // Warp mouse back to center when it gets too close to edge
        if (game.input.needsWarp(game.windowWidth, game.windowHeight)) {
            game.input.prepareForWarp();
            glutWarpPointer(game.input.windowCenterX, game.input.windowCenterY);
        }
    }
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
    // glDisable(GL_CULL_FACE); // We implemented manual back-face culling in draw.h
    // glCullFace(GL_BACK);
    // glFrontFace(GL_CCW);
    
    // Enable lighting
    // glDisable(GL_LIGHTING); // We implemented manual lighting in lighting.h
    // glEnable(GL_LIGHT0);
    // glEnable(GL_LIGHT1);
    
    // Enable color material
    // glEnable(GL_COLOR_MATERIAL);
    // glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    
    // Smooth shading (Gouraud)
    // glShadeModel(GL_SMOOTH);
    
    // Enable normalization
    glEnable(GL_NORMALIZE);

    // Enable Fog (CG.6)
    glEnable(GL_FOG);
    GLfloat fogColor[] = {0.05f, 0.05f, 0.1f, 1.0f}; // Match clear color
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_EXP);
    glFogf(GL_FOG_DENSITY, 0.05f);
    glHint(GL_FOG_HINT, GL_DONT_CARE);
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
    
    // Hide cursor
    glutSetCursor(GLUT_CURSOR_NONE);
    
    // Start timer
    game.lastTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    glutTimerFunc(Config::FRAME_TIME_MS, update, 0);
    
    // Start main loop
    glutMainLoop();
    
    return 0;
}
