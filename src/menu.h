/*******************************************************************************
 * THE SHIFTING MAZE - Menu System Header
 * 
 * Implements game menus:
 * - Main menu
 * - Pause menu
 * - Level select
 * - Game over/Win screens
 * 
 * Computer Graphics Algorithms:
 * - Orthographic projection for 2D menu rendering
 * - Alpha blending for overlays
 * - 2D transformations for animations
 ******************************************************************************/

#ifndef MENU_H
#define MENU_H

#include "config.h"
#include "lighting.h"
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

// ============================================================================
// MENU STATES
// ============================================================================
enum MenuState {
    MENU_NONE,          // No menu (gameplay)
    MENU_MAIN,          // Main menu
    MENU_PAUSE,         // Pause menu
    MENU_LEVEL_SELECT,  // Level selection
    MENU_OPTIONS,       // Options/settings
    MENU_GAME_OVER,     // Game over screen
    MENU_WIN,           // Victory screen
    MENU_LEVEL_COMPLETE // Level complete screen
};

// ============================================================================
// MENU ITEM
// ============================================================================
struct MenuItem {
    std::string text;
    bool isSelected;
    bool isEnabled;
    int action;         // Action ID when selected
    
    MenuItem() : text(""), isSelected(false), isEnabled(true), action(0) {}
    MenuItem(const std::string& t, int a) : text(t), isSelected(false), isEnabled(true), action(a) {}
};

// ============================================================================
// MENU ACTIONS
// ============================================================================
enum MenuAction {
    ACTION_NONE = 0,
    ACTION_START_GAME,
    ACTION_CONTINUE,
    ACTION_RESTART,
    ACTION_NEXT_LEVEL,
    ACTION_LEVEL_SELECT,
    ACTION_OPTIONS,
    ACTION_QUIT,
    ACTION_RESUME,
    ACTION_MAIN_MENU,
    ACTION_SELECT_LEVEL_1,
    ACTION_SELECT_LEVEL_2,
    ACTION_SELECT_LEVEL_3,
    ACTION_SELECT_LEVEL_4,
    ACTION_SELECT_LEVEL_5
};

// ============================================================================
// MENU SYSTEM
// ============================================================================
class MenuSystem {
public:
    MenuState currentMenu;
    std::vector<MenuItem> items;
    int selectedIndex;
    
    // Animation
    float fadeAlpha;
    float animationTime;
    bool fadeIn;
    
    // Screen dimensions
    int screenWidth;
    int screenHeight;
    
    // Colors
    Color backgroundColor;
    Color titleColor;
    Color textColor;
    Color selectedColor;
    Color disabledColor;
    
    // Display info
    std::string titleText;
    std::string subtitleText;
    int displayScore;
    float displayTime;
    int displayLevel;
    
    MenuSystem() {
        currentMenu = MENU_MAIN;
        selectedIndex = 0;
        fadeAlpha = 0;
        animationTime = 0;
        fadeIn = true;
        screenWidth = 1024;
        screenHeight = 768;
        
        backgroundColor = Color(0.05f, 0.05f, 0.1f);
        titleColor = Color(1.0f, 0.85f, 0.0f);
        textColor = Color(0.8f, 0.8f, 0.8f);
        selectedColor = Color(0.0f, 1.0f, 0.5f);
        disabledColor = Color(0.4f, 0.4f, 0.4f);
        
        displayScore = 0;
        displayTime = 0;
        displayLevel = 1;
    }
    
    void setScreenSize(int w, int h) {
        screenWidth = w;
        screenHeight = h;
    }
    
    // ========================================================================
    // MENU SETUP
    // ========================================================================
    
    void showMainMenu() {
        currentMenu = MENU_MAIN;
        items.clear();
        items.push_back(MenuItem("START GAME", ACTION_START_GAME));
        items.push_back(MenuItem("LEVEL SELECT", ACTION_LEVEL_SELECT));
        items.push_back(MenuItem("QUIT", ACTION_QUIT));
        selectedIndex = 0;
        items[0].isSelected = true;
        
        titleText = "THE SHIFTING MAZE";
        subtitleText = "A Computer Graphics Adventure";
        fadeAlpha = 0;
        fadeIn = true;
    }
    
    void showPauseMenu() {
        currentMenu = MENU_PAUSE;
        items.clear();
        items.push_back(MenuItem("RESUME", ACTION_RESUME));
        items.push_back(MenuItem("RESTART", ACTION_RESTART));
        items.push_back(MenuItem("MAIN MENU", ACTION_MAIN_MENU));
        items.push_back(MenuItem("QUIT", ACTION_QUIT));
        selectedIndex = 0;
        items[0].isSelected = true;
        
        titleText = "PAUSED";
        subtitleText = "";
    }
    
    void showLevelSelect(int highestUnlocked) {
        currentMenu = MENU_LEVEL_SELECT;
        items.clear();
        
        const char* levelNames[] = {
            "Level 1: The Beginning",
            "Level 2: Dark Corridors",
            "Level 3: The Labyrinth",
            "Level 4: Chaos Zone",
            "Level 5: The Final Escape"
        };
        
        for (int i = 0; i < 5; i++) {
            MenuItem item(levelNames[i], ACTION_SELECT_LEVEL_1 + i);
            item.isEnabled = (i <= highestUnlocked);
            items.push_back(item);
        }
        items.push_back(MenuItem("BACK", ACTION_MAIN_MENU));
        
        selectedIndex = 0;
        items[0].isSelected = true;
        
        titleText = "SELECT LEVEL";
        subtitleText = "";
    }
    
    void showGameOver(int score, float timeRemaining) {
        currentMenu = MENU_GAME_OVER;
        items.clear();
        items.push_back(MenuItem("RETRY", ACTION_RESTART));
        items.push_back(MenuItem("MAIN MENU", ACTION_MAIN_MENU));
        items.push_back(MenuItem("QUIT", ACTION_QUIT));
        selectedIndex = 0;
        items[0].isSelected = true;
        
        titleText = "GAME OVER";
        subtitleText = "Better luck next time!";
        displayScore = score;
        displayTime = timeRemaining;
    }
    
    void showWin(int score, float timeRemaining, int level, bool isFinalLevel) {
        currentMenu = isFinalLevel ? MENU_WIN : MENU_LEVEL_COMPLETE;
        items.clear();
        
        if (isFinalLevel) {
            items.push_back(MenuItem("PLAY AGAIN", ACTION_START_GAME));
        } else {
            items.push_back(MenuItem("NEXT LEVEL", ACTION_NEXT_LEVEL));
        }
        items.push_back(MenuItem("MAIN MENU", ACTION_MAIN_MENU));
        selectedIndex = 0;
        items[0].isSelected = true;
        
        titleText = isFinalLevel ? "VICTORY!" : "LEVEL COMPLETE!";
        subtitleText = isFinalLevel ? "You escaped the maze!" : "Prepare for the next challenge...";
        displayScore = score;
        displayTime = timeRemaining;
        displayLevel = level;
    }
    
    void hide() {
        currentMenu = MENU_NONE;
        items.clear();
    }
    
    bool isActive() const {
        return currentMenu != MENU_NONE;
    }
    
    // ========================================================================
    // INPUT HANDLING
    // ========================================================================
    
    void navigateUp() {
        if (items.empty()) return;
        
        items[selectedIndex].isSelected = false;
        
        do {
            selectedIndex--;
            if (selectedIndex < 0) selectedIndex = items.size() - 1;
        } while (!items[selectedIndex].isEnabled && selectedIndex != 0);
        
        items[selectedIndex].isSelected = true;
    }
    
    void navigateDown() {
        if (items.empty()) return;
        
        items[selectedIndex].isSelected = false;
        
        do {
            selectedIndex++;
            if (selectedIndex >= (int)items.size()) selectedIndex = 0;
        } while (!items[selectedIndex].isEnabled && selectedIndex != (int)items.size() - 1);
        
        items[selectedIndex].isSelected = true;
    }
    
    int select() {
        if (items.empty() || selectedIndex < 0 || selectedIndex >= (int)items.size()) {
            return ACTION_NONE;
        }
        
        if (items[selectedIndex].isEnabled) {
            return items[selectedIndex].action;
        }
        return ACTION_NONE;
    }
    
    // ========================================================================
    // UPDATE ANIMATION
    // ========================================================================
    void update(float deltaTime) {
        animationTime += deltaTime;
        
        if (fadeIn && fadeAlpha < 1.0f) {
            fadeAlpha += deltaTime * 2.0f;
            if (fadeAlpha > 1.0f) fadeAlpha = 1.0f;
        }
    }
    
    // ========================================================================
    // RENDER MENU
    // Uses orthographic projection for 2D rendering
    // ========================================================================
    void render() {
        if (currentMenu == MENU_NONE) return;
        
        // Setup orthographic projection
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, screenWidth, 0, screenHeight);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        // Disable 3D features
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Draw semi-transparent background overlay
        glColor4f(backgroundColor.r, backgroundColor.g, backgroundColor.b, 0.85f * fadeAlpha);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(screenWidth, 0);
        glVertex2f(screenWidth, screenHeight);
        glVertex2f(0, screenHeight);
        glEnd();
        
        // Draw title
        float titleY = screenHeight * 0.75f;
        glColor4f(titleColor.r, titleColor.g, titleColor.b, fadeAlpha);
        drawCenteredText(titleText.c_str(), titleY, GLUT_BITMAP_TIMES_ROMAN_24);
        
        // Draw subtitle
        if (!subtitleText.empty()) {
            glColor4f(textColor.r, textColor.g, textColor.b, fadeAlpha * 0.8f);
            drawCenteredText(subtitleText.c_str(), titleY - 40, GLUT_BITMAP_HELVETICA_18);
        }
        
        // Draw score/time for game over and win screens
        if (currentMenu == MENU_GAME_OVER || currentMenu == MENU_WIN || currentMenu == MENU_LEVEL_COMPLETE) {
            char scoreStr[64];
            sprintf(scoreStr, "Score: %d", displayScore);
            glColor4f(textColor.r, textColor.g, textColor.b, fadeAlpha);
            drawCenteredText(scoreStr, titleY - 80, GLUT_BITMAP_HELVETICA_18);
            
            if (displayTime > 0) {
                int mins = (int)(displayTime / 60);
                int secs = (int)displayTime % 60;
                char timeStr[64];
                sprintf(timeStr, "Time Remaining: %d:%02d", mins, secs);
                drawCenteredText(timeStr, titleY - 105, GLUT_BITMAP_HELVETICA_18);
            }
        }
        
        // Draw menu items
        float startY = screenHeight * 0.45f;
        float itemSpacing = 45.0f;
        
        for (size_t i = 0; i < items.size(); i++) {
            float y = startY - i * itemSpacing;
            
            if (!items[i].isEnabled) {
                glColor4f(disabledColor.r, disabledColor.g, disabledColor.b, fadeAlpha * 0.5f);
            } else if (items[i].isSelected) {
                // Pulsing selection effect
                float pulse = 0.8f + 0.2f * sin(animationTime * 5.0f);
                glColor4f(selectedColor.r * pulse, selectedColor.g * pulse, selectedColor.b, fadeAlpha);
            } else {
                glColor4f(textColor.r, textColor.g, textColor.b, fadeAlpha);
            }
            
            // Draw selection indicator
            if (items[i].isSelected) {
                drawCenteredText(("> " + items[i].text + " <").c_str(), y, GLUT_BITMAP_HELVETICA_18);
            } else {
                drawCenteredText(items[i].text.c_str(), y, GLUT_BITMAP_HELVETICA_18);
            }
        }
        
        // Draw controls hint
        glColor4f(0.5f, 0.5f, 0.5f, fadeAlpha * 0.7f);
        drawCenteredText("UP/DOWN: Navigate | ENTER: Select | ESC: Back", 30, GLUT_BITMAP_HELVETICA_12);
        
        // Restore state
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
    
private:
    void drawCenteredText(const char* text, float y, void* font) {
        // Calculate text width
        int width = 0;
        for (const char* c = text; *c; c++) {
            width += glutBitmapWidth(font, *c);
        }
        
        float x = (screenWidth - width) / 2.0f;
        
        glRasterPos2f(x, y);
        for (const char* c = text; *c; c++) {
            glutBitmapCharacter(font, *c);
        }
    }
};

#endif // MENU_H
