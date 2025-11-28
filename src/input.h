/*******************************************************************************
 * THE SHIFTING MAZE - Input Handler Header
 * 
 * Handles keyboard and mouse input
 ******************************************************************************/

#ifndef INPUT_H
#define INPUT_H

#include "config.h"

// ============================================================================
// INPUT MANAGER CLASS
// ============================================================================
class InputManager {
public:
    // Keyboard state
    bool keys[256];
    bool specialKeys[256];
    
    // Mouse state
    int mouseX, mouseY;
    int lastMouseX, lastMouseY;
    int mouseDeltaX, mouseDeltaY;
    bool mouseWarped;
    bool mouseCaptured;
    
    // Window center for mouse capture
    int windowCenterX, windowCenterY;
    
    InputManager() {
        reset();
    }
    
    void reset() {
        for (int i = 0; i < 256; i++) {
            keys[i] = false;
            specialKeys[i] = false;
        }
        mouseX = mouseY = 0;
        lastMouseX = lastMouseY = 0;
        mouseDeltaX = mouseDeltaY = 0;
        mouseWarped = false;
        mouseCaptured = true;
        windowCenterX = Config::WINDOW_WIDTH / 2;
        windowCenterY = Config::WINDOW_HEIGHT / 2;
    }
    
    void setWindowCenter(int x, int y) {
        windowCenterX = x;
        windowCenterY = y;
    }
    
    // ========================================================================
    // KEYBOARD HANDLING
    // ========================================================================
    void keyDown(unsigned char key) {
        keys[key] = true;
    }
    
    void keyUp(unsigned char key) {
        keys[key] = false;
    }
    
    void specialKeyDown(int key) {
        if (key >= 0 && key < 256) {
            specialKeys[key] = true;
        }
    }
    
    void specialKeyUp(int key) {
        if (key >= 0 && key < 256) {
            specialKeys[key] = false;
        }
    }
    
    bool isKeyDown(unsigned char key) const {
        return keys[key];
    }
    
    bool isSpecialKeyDown(int key) const {
        return (key >= 0 && key < 256) ? specialKeys[key] : false;
    }
    
    // ========================================================================
    // MOUSE HANDLING
    // ========================================================================
    void mouseMove(int x, int y) {
        if (mouseWarped) {
            mouseWarped = false;
            lastMouseX = x;
            lastMouseY = y;
            return;
        }
        
        mouseDeltaX = x - windowCenterX;
        mouseDeltaY = y - windowCenterY;
        
        lastMouseX = mouseX;
        lastMouseY = mouseY;
        mouseX = x;
        mouseY = y;
    }
    
    void resetMouseDelta() {
        mouseDeltaX = 0;
        mouseDeltaY = 0;
    }
    
    // Check movement keys
    bool isMovingForward() const {
        return keys['w'] || keys['W'];
    }
    
    bool isMovingBackward() const {
        return keys['s'] || keys['S'];
    }
    
    bool isMovingLeft() const {
        return keys['a'] || keys['A'];
    }
    
    bool isMovingRight() const {
        return keys['d'] || keys['D'];
    }
};

#endif // INPUT_H
