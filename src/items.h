/*******************************************************************************
 * THE SHIFTING MAZE - Collectible Items Header
 * 
 * Implements collectible items (keys, coins, power-ups):
 * - Keys to unlock doors
 * - Coins for score
 * - Power-ups (speed boost, invincibility, time bonus)
 * 
 * Computer Graphics Algorithms:
 * - Bézier curves for floating animation
 * - 3D transformations for rotation and scaling
 * - Parametric equations for item shapes
 ******************************************************************************/

#ifndef ITEMS_H
#define ITEMS_H

#include "matrix.h"
#include "bezier.h"
#include "lighting.h"
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// ITEM TYPES
// ============================================================================
enum ItemType {
    ITEM_COIN,
    ITEM_KEY,
    ITEM_SPEED_BOOST,
    ITEM_INVINCIBILITY,
    ITEM_TIME_BONUS,
    ITEM_HEALTH
};

// ============================================================================
// COLLECTIBLE ITEM CLASS
// ============================================================================
class Item {
public:
    Vec4 position;          // World position
    Vec4 basePosition;      // Base position (for floating animation)
    ItemType type;
    float radius;           // Collision radius
    bool isCollected;
    bool isActive;
    
    // Animation parameters
    float rotationY;        // Current Y rotation
    float rotationSpeed;    // Rotation speed (degrees/sec)
    float floatPhase;       // Phase for floating animation
    float floatAmplitude;   // How high it floats
    float floatSpeed;       // Floating speed
    
    // Visual properties
    Color primaryColor;
    Color secondaryColor;
    float scale;
    float pulsePhase;       // For pulsing glow effect
    
    // Power-up specific
    float duration;         // Duration of power-up effect
    float value;            // Value (time bonus amount, speed multiplier, etc.)
    
    Item() {
        position = Vec4(0, 0.5f, 0);
        basePosition = position;
        type = ITEM_COIN;
        radius = 0.3f;
        isCollected = false;
        isActive = true;
        
        rotationY = 0;
        rotationSpeed = 90.0f;  // 90 degrees per second
        floatPhase = 0;
        floatAmplitude = 0.15f;
        floatSpeed = 2.0f;
        
        scale = 1.0f;
        pulsePhase = 0;
        
        duration = 5.0f;
        value = 1.0f;
        
        setTypeProperties(ITEM_COIN);
    }
    
    void setTypeProperties(ItemType itemType) {
        type = itemType;
        
        switch (type) {
            case ITEM_COIN:
                primaryColor = Color(1.0f, 0.85f, 0.0f);    // Gold
                secondaryColor = Color(1.0f, 0.95f, 0.5f);
                radius = 0.25f;
                rotationSpeed = 120.0f;
                floatAmplitude = 0.1f;
                value = 100;  // Score value
                break;
                
            case ITEM_KEY:
                primaryColor = Color(0.7f, 0.7f, 0.8f);     // Silver
                secondaryColor = Color(0.9f, 0.9f, 1.0f);
                radius = 0.3f;
                rotationSpeed = 60.0f;
                floatAmplitude = 0.12f;
                value = 1;  // Key count
                break;
                
            case ITEM_SPEED_BOOST:
                primaryColor = Color(0.0f, 0.8f, 1.0f);     // Cyan
                secondaryColor = Color(0.5f, 1.0f, 1.0f);
                radius = 0.3f;
                rotationSpeed = 180.0f;
                floatAmplitude = 0.2f;
                duration = 8.0f;
                value = 1.5f;  // Speed multiplier
                break;
                
            case ITEM_INVINCIBILITY:
                primaryColor = Color(1.0f, 0.5f, 0.0f);     // Orange
                secondaryColor = Color(1.0f, 1.0f, 0.0f);
                radius = 0.35f;
                rotationSpeed = 200.0f;
                floatAmplitude = 0.25f;
                duration = 5.0f;
                value = 1;
                break;
                
            case ITEM_TIME_BONUS:
                primaryColor = Color(0.0f, 1.0f, 0.5f);     // Green
                secondaryColor = Color(0.5f, 1.0f, 0.8f);
                radius = 0.3f;
                rotationSpeed = 90.0f;
                floatAmplitude = 0.15f;
                value = 30.0f;  // Bonus seconds
                break;
                
            case ITEM_HEALTH:
                primaryColor = Color(1.0f, 0.2f, 0.2f);     // Red
                secondaryColor = Color(1.0f, 0.5f, 0.5f);
                radius = 0.3f;
                rotationSpeed = 45.0f;
                floatAmplitude = 0.1f;
                value = 1;  // Health points
                break;
        }
    }
    
    void setPosition(float x, float y, float z) {
        position = Vec4(x, y, z);
        basePosition = position;
        // Random starting phase for variety
        floatPhase = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
        pulsePhase = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
    }
    
    // ========================================================================
    // UPDATE ITEM ANIMATION
    // Uses Bézier-like smooth interpolation and parametric equations
    // ========================================================================
    void update(float deltaTime) {
        if (!isActive || isCollected) return;
        
        // Rotation animation (3D transformation)
        rotationY += rotationSpeed * deltaTime;
        if (rotationY >= 360.0f) rotationY -= 360.0f;
        
        // Floating animation using sine wave (parametric curve)
        // y = baseY + amplitude * sin(phase)
        floatPhase += floatSpeed * deltaTime;
        if (floatPhase >= 2.0f * M_PI) floatPhase -= 2.0f * M_PI;
        position.y = basePosition.y + floatAmplitude * sin(floatPhase);
        
        // Pulse effect for glow
        pulsePhase += 3.0f * deltaTime;
        if (pulsePhase >= 2.0f * M_PI) pulsePhase -= 2.0f * M_PI;
    }
    
    // Get current pulse intensity (0.5 to 1.0)
    float getPulseIntensity() const {
        return 0.75f + 0.25f * sin(pulsePhase);
    }
    
    // ========================================================================
    // COLLISION DETECTION
    // ========================================================================
    bool checkCollision(const Vec4& playerPos, float playerRadius) const {
        if (!isActive || isCollected) return false;
        
        // Only check X and Z distance (ignore Y height difference)
        float dx = position.x - playerPos.x;
        float dz = position.z - playerPos.z;
        
        float distSquared = dx * dx + dz * dz;
        float radiusSum = radius + playerRadius;
        
        return distSquared < (radiusSum * radiusSum);
    }
    
    // Mark as collected
    void collect() {
        isCollected = true;
        isActive = false;
    }
    
    // Reset item
    void reset() {
        isCollected = false;
        isActive = true;
        position = basePosition;
        rotationY = 0;
        floatPhase = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
    }
    
    // ========================================================================
    // GET TRANSFORMATION MATRIX
    // Combines translation, rotation, scale, and floating offset
    // ========================================================================
    Matrix4x4 getTransformMatrix() const {
        // Translation to current position (includes float offset)
        Matrix4x4 T = createTranslationMatrix(position.x, position.y, position.z);
        
        // Rotation around Y axis
        float radAngle = rotationY * M_PI / 180.0f;
        Matrix4x4 R = createRotationYMatrix(radAngle);
        
        // Scale with pulse effect
        float pulseScale = 1.0f + 0.1f * sin(pulsePhase);
        Matrix4x4 S = createScaleMatrix(scale * pulseScale, scale * pulseScale, scale * pulseScale);
        
        // Combine: S * R * T
        Matrix4x4 result = multiplyMatrix(S, R);
        result = multiplyMatrix(result, T);
        
        return result;
    }
};

// ============================================================================
// ITEM MANAGER
// ============================================================================
class ItemManager {
public:
    std::vector<Item> items;
    
    // Collected counts
    int coinsCollected;
    int keysCollected;
    int keysRequired;
    
    // Active power-ups
    bool hasSpeedBoost;
    bool hasInvincibility;
    float speedBoostTimer;
    float invincibilityTimer;
    float speedMultiplier;
    
    // Score
    int totalScore;
    
    ItemManager() {
        coinsCollected = 0;
        keysCollected = 0;
        keysRequired = 0;
        hasSpeedBoost = false;
        hasInvincibility = false;
        speedBoostTimer = 0;
        invincibilityTimer = 0;
        speedMultiplier = 1.0f;
        totalScore = 0;
    }
    
    // ========================================================================
    // ADD ITEMS
    // ========================================================================
    void addItem(ItemType type, float x, float y, float z) {
        Item item;
        item.setTypeProperties(type);
        item.setPosition(x, y, z);
        items.push_back(item);
    }
    
    void addCoin(float x, float z) {
        addItem(ITEM_COIN, x, 0.6f, z);
    }
    
    void addKey(float x, float z) {
        addItem(ITEM_KEY, x, 0.7f, z);
    }
    
    void addPowerUp(ItemType type, float x, float z) {
        addItem(type, x, 0.8f, z);
    }
    
    // ========================================================================
    // UPDATE ALL ITEMS
    // ========================================================================
    void update(float deltaTime) {
        // Update item animations
        for (auto& item : items) {
            item.update(deltaTime);
        }
        
        // Update power-up timers
        if (hasSpeedBoost) {
            speedBoostTimer -= deltaTime;
            if (speedBoostTimer <= 0) {
                hasSpeedBoost = false;
                speedMultiplier = 1.0f;
            }
        }
        
        if (hasInvincibility) {
            invincibilityTimer -= deltaTime;
            if (invincibilityTimer <= 0) {
                hasInvincibility = false;
            }
        }
    }
    
    // ========================================================================
    // CHECK COLLECTION
    // Returns collected item type or -1 if none
    // ========================================================================
    int checkCollection(const Vec4& playerPos, float playerRadius) {
        for (auto& item : items) {
            if (item.checkCollision(playerPos, playerRadius)) {
                item.collect();
                return processCollection(item);
            }
        }
        return -1;
    }
    
    // Process item collection
    int processCollection(Item& item) {
        switch (item.type) {
            case ITEM_COIN:
                coinsCollected++;
                totalScore += (int)item.value;
                return ITEM_COIN;
                
            case ITEM_KEY:
                keysCollected++;
                return ITEM_KEY;
                
            case ITEM_SPEED_BOOST:
                hasSpeedBoost = true;
                speedBoostTimer = item.duration;
                speedMultiplier = item.value;
                return ITEM_SPEED_BOOST;
                
            case ITEM_INVINCIBILITY:
                hasInvincibility = true;
                invincibilityTimer = item.duration;
                return ITEM_INVINCIBILITY;
                
            case ITEM_TIME_BONUS:
                // Time bonus handled externally
                return ITEM_TIME_BONUS;
                
            case ITEM_HEALTH:
                return ITEM_HEALTH;
        }
        return -1;
    }
    
    // Check if all keys collected
    bool hasAllKeys() const {
        return keysCollected >= keysRequired;
    }
    
    // Get speed multiplier (for player movement)
    float getSpeedMultiplier() const {
        return speedMultiplier;
    }
    
    // Check invincibility
    bool isInvincible() const {
        return hasInvincibility;
    }
    
    // Get remaining power-up time
    float getSpeedBoostTime() const { return speedBoostTimer; }
    float getInvincibilityTime() const { return invincibilityTimer; }
    
    // ========================================================================
    // RESET
    // ========================================================================
    void reset() {
        for (auto& item : items) {
            item.reset();
        }
        coinsCollected = 0;
        keysCollected = 0;
        hasSpeedBoost = false;
        hasInvincibility = false;
        speedBoostTimer = 0;
        invincibilityTimer = 0;
        speedMultiplier = 1.0f;
        totalScore = 0;
    }
    
    void clear() {
        items.clear();
        reset();
    }
    
    // Count items
    int getCoinCount() const {
        int count = 0;
        for (const auto& item : items) {
            if (item.type == ITEM_COIN && !item.isCollected) count++;
        }
        return count;
    }
    
    int getKeyCount() const {
        int count = 0;
        for (const auto& item : items) {
            if (item.type == ITEM_KEY && !item.isCollected) count++;
        }
        return count;
    }
};

#endif // ITEMS_H
