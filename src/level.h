/*******************************************************************************
 * THE SHIFTING MAZE - Level System Header
 * 
 * Implements multiple levels with increasing difficulty:
 * - Larger mazes at higher levels
 * - More enemies
 * - Less time
 * - Different visual themes
 * 
 * Computer Graphics Algorithms:
 * - All transformations and projections from previous levels
 * - Color interpolation for level themes
 ******************************************************************************/

#ifndef LEVEL_H
#define LEVEL_H

#include "config.h"
#include "lighting.h"
#include <string>

// ============================================================================
// LEVEL DATA
// ============================================================================
struct LevelData {
    int levelNumber;
    int mazeSize;           // Grid size (10, 12, 15, etc.)
    int numEnemies;         // Number of patrol enemies
    int numChaseEnemies;    // Number of chasing enemies
    float gameTime;         // Time limit in seconds
    float enemySpeed;       // Base enemy speed multiplier
    float shiftInterval;    // How often maze shifts (seconds)
    int numDynamicWalls;    // Number of dynamic walls
    int numKeys;            // Keys needed to unlock exit
    int numCoins;           // Coins available
    
    // Visual theme
    Color wallColor;
    Color floorColor;
    Color fogColor;
    float fogDensity;
    std::string levelName;
    
    LevelData() {
        levelNumber = 1;
        mazeSize = 10;
        numEnemies = 2;
        numChaseEnemies = 0;
        gameTime = 180.0f;
        enemySpeed = 1.0f;
        shiftInterval = 30.0f;
        numDynamicWalls = 5;
        numKeys = 0;
        numCoins = 3;
        
        wallColor = Color(0.6f, 0.5f, 0.4f);
        floorColor = Color(0.4f, 0.5f, 0.4f);
        fogColor = Color(0.05f, 0.05f, 0.1f);
        fogDensity = 0.02f;
        levelName = "The Beginning";
    }
};

// ============================================================================
// LEVEL MANAGER
// ============================================================================
class LevelManager {
public:
    static const int MAX_LEVELS = 5;
    LevelData levels[MAX_LEVELS];
    int currentLevel;
    int highestUnlocked;
    int totalScore;
    
    LevelManager() {
        currentLevel = 0;
        highestUnlocked = 0;
        totalScore = 0;
        initializeLevels();
    }
    
    void initializeLevels() {
        // ====================================================================
        // LEVEL 1: The Beginning
        // ====================================================================
        levels[0].levelNumber = 1;
        levels[0].levelName = "The Beginning";
        levels[0].mazeSize = 10;
        levels[0].numEnemies = 2;
        levels[0].numChaseEnemies = 0;
        levels[0].gameTime = 180.0f;    // 3 minutes
        levels[0].enemySpeed = 1.0f;
        levels[0].shiftInterval = 45.0f;
        levels[0].numDynamicWalls = 3;
        levels[0].numKeys = 0;
        levels[0].numCoins = 5;
        levels[0].wallColor = Color(0.6f, 0.5f, 0.4f);    // Stone gray
        levels[0].floorColor = Color(0.4f, 0.5f, 0.4f);   // Mossy green
        levels[0].fogColor = Color(0.05f, 0.05f, 0.1f);
        levels[0].fogDensity = 0.015f;
        
        // ====================================================================
        // LEVEL 2: Dark Corridors
        // ====================================================================
        levels[1].levelNumber = 2;
        levels[1].levelName = "Dark Corridors";
        levels[1].mazeSize = 12;
        levels[1].numEnemies = 3;
        levels[1].numChaseEnemies = 1;
        levels[1].gameTime = 150.0f;    // 2.5 minutes
        levels[1].enemySpeed = 1.2f;
        levels[1].shiftInterval = 35.0f;
        levels[1].numDynamicWalls = 5;
        levels[1].numKeys = 1;
        levels[1].numCoins = 7;
        levels[1].wallColor = Color(0.4f, 0.35f, 0.3f);   // Darker stone
        levels[1].floorColor = Color(0.3f, 0.3f, 0.35f);  // Dark blue-gray
        levels[1].fogColor = Color(0.02f, 0.02f, 0.05f);
        levels[1].fogDensity = 0.025f;
        
        // ====================================================================
        // LEVEL 3: The Labyrinth
        // ====================================================================
        levels[2].levelNumber = 3;
        levels[2].levelName = "The Labyrinth";
        levels[2].mazeSize = 15;
        levels[2].numEnemies = 4;
        levels[2].numChaseEnemies = 2;
        levels[2].gameTime = 180.0f;    // 3 minutes (bigger maze)
        levels[2].enemySpeed = 1.4f;
        levels[2].shiftInterval = 25.0f;
        levels[2].numDynamicWalls = 8;
        levels[2].numKeys = 2;
        levels[2].numCoins = 10;
        levels[2].wallColor = Color(0.5f, 0.4f, 0.5f);    // Purple-gray
        levels[2].floorColor = Color(0.35f, 0.3f, 0.4f);
        levels[2].fogColor = Color(0.03f, 0.02f, 0.05f);
        levels[2].fogDensity = 0.02f;
        
        // ====================================================================
        // LEVEL 4: Chaos Zone
        // ====================================================================
        levels[3].levelNumber = 4;
        levels[3].levelName = "Chaos Zone";
        levels[3].mazeSize = 15;
        levels[3].numEnemies = 5;
        levels[3].numChaseEnemies = 3;
        levels[3].gameTime = 150.0f;
        levels[3].enemySpeed = 1.6f;
        levels[3].shiftInterval = 20.0f;
        levels[3].numDynamicWalls = 12;
        levels[3].numKeys = 3;
        levels[3].numCoins = 12;
        levels[3].wallColor = Color(0.6f, 0.3f, 0.3f);    // Red-tinted
        levels[3].floorColor = Color(0.4f, 0.25f, 0.25f);
        levels[3].fogColor = Color(0.05f, 0.02f, 0.02f);
        levels[3].fogDensity = 0.03f;
        
        // ====================================================================
        // LEVEL 5: The Final Escape
        // ====================================================================
        levels[4].levelNumber = 5;
        levels[4].levelName = "The Final Escape";
        levels[4].mazeSize = 18;
        levels[4].numEnemies = 6;
        levels[4].numChaseEnemies = 4;
        levels[4].gameTime = 200.0f;
        levels[4].enemySpeed = 1.8f;
        levels[4].shiftInterval = 15.0f;
        levels[4].numDynamicWalls = 15;
        levels[4].numKeys = 4;
        levels[4].numCoins = 15;
        levels[4].wallColor = Color(0.2f, 0.2f, 0.25f);   // Dark slate
        levels[4].floorColor = Color(0.15f, 0.15f, 0.2f);
        levels[4].fogColor = Color(0.01f, 0.01f, 0.02f);
        levels[4].fogDensity = 0.035f;
    }
    
    LevelData& getCurrentLevel() {
        return levels[currentLevel];
    }
    
    const LevelData& getCurrentLevel() const {
        return levels[currentLevel];
    }
    
    bool nextLevel() {
        if (currentLevel < MAX_LEVELS - 1) {
            currentLevel++;
            if (currentLevel > highestUnlocked) {
                highestUnlocked = currentLevel;
            }
            return true;
        }
        return false;
    }
    
    bool prevLevel() {
        if (currentLevel > 0) {
            currentLevel--;
            return true;
        }
        return false;
    }
    
    void selectLevel(int level) {
        if (level >= 0 && level <= highestUnlocked && level < MAX_LEVELS) {
            currentLevel = level;
        }
    }
    
    bool isLastLevel() const {
        return currentLevel >= MAX_LEVELS - 1;
    }
    
    void reset() {
        currentLevel = 0;
        totalScore = 0;
    }
    
    void addScore(int points) {
        totalScore += points;
    }
    
    // Calculate bonus score based on remaining time and coins
    int calculateLevelScore(float remainingTime, int coinsCollected, int totalCoins) {
        int timeBonus = (int)(remainingTime * 10);
        int coinBonus = coinsCollected * 100;
        int levelBonus = (currentLevel + 1) * 500;
        return timeBonus + coinBonus + levelBonus;
    }
};

#endif // LEVEL_H
