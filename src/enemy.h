/*******************************************************************************
 * THE SHIFTING MAZE - Enemy System Header
 * 
 * Implements enemies that move along Bézier curves:
 * - Sphere or cylinder shaped enemies
 * - Movement along Bézier paths
 * - Collision detection with player
 ******************************************************************************/

#ifndef ENEMY_H
#define ENEMY_H

#include "matrix.h"
#include "bezier.h"
#include <vector>

// ============================================================================
// ENEMY CLASS
// ============================================================================
class Enemy {
public:
    Vec4 position;              // Current position
    Vec4 startPosition;         // Starting position
    float radius;               // Collision radius
    
    BezierCurve path;           // Bézier path for movement
    float pathT;                // Current parameter t on path [0, 1]
    float speed;                // Movement speed
    int direction;              // 1 = forward, -1 = backward
    
    Color color;                // Enemy color
    bool isAlive;               // Is enemy active
    
    // For sphere rendering
    int slices;                 // Longitude divisions
    int stacks;                 // Latitude divisions
    
    Enemy() {
        position = Vec4(0, 0.5f, 0);
        startPosition = position;
        radius = 0.4f;
        pathT = 0;
        speed = 0.15f;
        direction = 1;
        color = Color(0.8f, 0.1f, 0.1f);
        isAlive = true;
        slices = 16;
        stacks = 8;
    }
    
    // ========================================================================
    // SETUP PATROL PATH (Bézier curve)
    // ========================================================================
    void setupPath(const Vec4& start, const Vec4& end, float height = 0.5f) {
        path.clear();
        
        // Start point
        Vec4 p0 = start;
        p0.y = height;
        path.addPoint(p0);
        
        // Control point 1
        Vec4 p1;
        p1.x = start.x + (end.x - start.x) * 0.25f;
        p1.y = height + 0.3f;  // Slight vertical variation
        p1.z = start.z + (end.z - start.z) * 0.25f + 0.5f;
        path.addPoint(p1);
        
        // Control point 2
        Vec4 p2;
        p2.x = start.x + (end.x - start.x) * 0.75f;
        p2.y = height + 0.2f;
        p2.z = start.z + (end.z - start.z) * 0.75f - 0.5f;
        path.addPoint(p2);
        
        // End point
        Vec4 p3 = end;
        p3.y = height;
        path.addPoint(p3);
        
        startPosition = p0;
        position = p0;
    }
    
    // Setup circular patrol path
    void setupCircularPath(const Vec4& center, float pathRadius, float height = 0.5f) {
        path.clear();
        
        // Create a circular-ish path using 4 control points
        // This approximates a circle using Bézier curves
        float k = 0.5523f;  // Magic number for circle approximation
        
        Vec4 p0(center.x + pathRadius, height, center.z);
        Vec4 p1(center.x + pathRadius, height, center.z + pathRadius * k);
        Vec4 p2(center.x + pathRadius * k, height, center.z + pathRadius);
        Vec4 p3(center.x, height, center.z + pathRadius);
        
        path.addPoint(p0);
        path.addPoint(p1);
        path.addPoint(p2);
        path.addPoint(p3);
        
        // Add more points for full circle
        Vec4 p4(center.x - pathRadius * k, height, center.z + pathRadius);
        Vec4 p5(center.x - pathRadius, height, center.z + pathRadius * k);
        Vec4 p6(center.x - pathRadius, height, center.z);
        path.addPoint(p4);
        path.addPoint(p5);
        path.addPoint(p6);
        
        startPosition = p0;
        position = p0;
    }
    
    // ========================================================================
    // UPDATE ENEMY POSITION
    // ========================================================================
    void update(float deltaTime) {
        if (!isAlive) return;
        
        // Move along Bézier path
        pathT += speed * direction * deltaTime;
        
        // Bounce at ends
        if (pathT >= 1.0f) {
            pathT = 1.0f;
            direction = -1;
        } else if (pathT <= 0.0f) {
            pathT = 0.0f;
            direction = 1;
        }
        
        // Update position using Bézier curve (Bernstein formula)
        position = path.computeBernstein(pathT);
    }
    
    // ========================================================================
    // COLLISION DETECTION WITH PLAYER
    // ========================================================================
    bool checkCollision(const Vec4& playerPos, float playerRadius) const {
        if (!isAlive) return false;
        
        float dx = position.x - playerPos.x;
        float dy = position.y - playerPos.y;
        float dz = position.z - playerPos.z;
        
        float distSquared = dx * dx + dy * dy + dz * dz;
        float radiusSum = radius + playerRadius;
        
        return distSquared < (radiusSum * radiusSum);
    }
    
    // Reset to start
    void reset() {
        position = startPosition;
        pathT = 0;
        direction = 1;
        isAlive = true;
    }
};

// ============================================================================
// ENEMY MANAGER
// ============================================================================
class EnemyManager {
public:
    std::vector<Enemy> enemies;
    
    EnemyManager() {}
    
    // Add enemy with patrol path
    void addEnemy(const Vec4& start, const Vec4& end) {
        Enemy enemy;
        enemy.setupPath(start, end);
        enemies.push_back(enemy);
    }
    
    // Add enemy with circular patrol
    void addCircularEnemy(const Vec4& center, float radius) {
        Enemy enemy;
        enemy.setupCircularPath(center, radius);
        enemies.push_back(enemy);
    }
    
    // Update all enemies
    void update(float deltaTime) {
        for (auto& enemy : enemies) {
            enemy.update(deltaTime);
        }
    }
    
    // Check collision with player
    bool checkPlayerCollision(const Vec4& playerPos, float playerRadius) const {
        for (const auto& enemy : enemies) {
            if (enemy.checkCollision(playerPos, playerRadius)) {
                return true;
            }
        }
        return false;
    }
    
    // Reset all enemies
    void reset() {
        for (auto& enemy : enemies) {
            enemy.reset();
        }
    }
    
    // Clear all enemies
    void clear() {
        enemies.clear();
    }
};

#endif // ENEMY_H
