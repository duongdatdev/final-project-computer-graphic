/*******************************************************************************
 * THE SHIFTING MAZE - HUD (Heads-Up Display) Header
 * 
 * Implements 2D overlay using orthographic projection:
 * - Countdown timer
 * - Mini-map (top-down view)
 * - Win/Lose messages
 * 
 * Uses orthographic projection: x' = x0, y' = y0
 ******************************************************************************/

#ifndef HUD_H
#define HUD_H

#include "matrix.h"
#include "maze.h"
#include <string>
#include <cstdio>

// ============================================================================
// HUD CLASS
// ============================================================================
class HUD {
public:
    int screenWidth, screenHeight;
    
    // Timer
    float gameTime;         // Total game time in seconds
    float remainingTime;    // Remaining time
    bool timerActive;
    
    // Mini-map settings
    float mapX, mapY;       // Position on screen
    float mapSize;          // Size of mini-map
    bool showMiniMap;
    
    // Game state messages
    bool showWinMessage;
    bool showLoseMessage;
    std::string message;
    
    // Colors
    Color timerColor;
    Color mapWallColor;
    Color mapEmptyColor;
    Color mapPlayerColor;
    Color mapExitColor;
    Color mapEnemyColor;
    
    HUD() {
        screenWidth = 800;
        screenHeight = 600;
        
        gameTime = 180.0f;      // 3 minutes
        remainingTime = gameTime;
        timerActive = true;
        
        mapX = 10.0f;
        mapY = 10.0f;
        mapSize = 150.0f;
        showMiniMap = true;
        
        showWinMessage = false;
        showLoseMessage = false;
        message = "";
        
        timerColor = Color(1.0f, 1.0f, 1.0f);
        mapWallColor = Color(0.4f, 0.4f, 0.4f);
        mapEmptyColor = Color(0.2f, 0.2f, 0.2f);
        mapPlayerColor = Color(0.0f, 1.0f, 0.0f);
        mapExitColor = Color(1.0f, 1.0f, 0.0f);
        mapEnemyColor = Color(1.0f, 0.0f, 0.0f);
    }
    
    // Update timer
    void update(float deltaTime) {
        if (timerActive && remainingTime > 0) {
            remainingTime -= deltaTime;
            if (remainingTime <= 0) {
                remainingTime = 0;
                timerActive = false;
            }
        }
    }
    
    // Set screen size
    void setScreenSize(int width, int height) {
        screenWidth = width;
        screenHeight = height;
    }
    
    // Reset game
    void reset() {
        remainingTime = gameTime;
        timerActive = true;
        showWinMessage = false;
        showLoseMessage = false;
        message = "";
    }
    
    // Check if time is up
    bool isTimeUp() const {
        return remainingTime <= 0;
    }
    
    // Get formatted time string
    std::string getTimeString() const {
        int minutes = (int)(remainingTime / 60);
        int seconds = (int)remainingTime % 60;
        char buffer[32];
        sprintf(buffer, "%02d:%02d", minutes, seconds);
        return std::string(buffer);
    }
    
    // Set win state
    void setWin() {
        showWinMessage = true;
        timerActive = false;
        message = "YOU WIN! Press R to restart";
    }
    
    // Set lose state
    void setLose(const std::string& reason) {
        showLoseMessage = true;
        timerActive = false;
        message = "GAME OVER: " + reason + " - Press R to restart";
    }
    
    // ========================================================================
    // ORTHOGRAPHIC PROJECTION FOR HUD
    // x' = x0, y' = y0
    // Maps screen coordinates directly (no perspective distortion)
    // ========================================================================
    Vec4 projectHUD(float x, float y) const {
        // Direct mapping for 2D HUD elements
        return Vec4(x, y, 0);
    }
    
    // Get timer color (changes when low on time)
    Color getTimerColor() const {
        if (remainingTime <= 10) {
            // Flash red when very low
            return Color(1.0f, 0.0f, 0.0f);
        } else if (remainingTime <= 30) {
            // Orange when low
            return Color(1.0f, 0.5f, 0.0f);
        }
        return timerColor;
    }
};

#endif // HUD_H
