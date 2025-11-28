/*******************************************************************************
 * THE SHIFTING MAZE - Enemy System Header
 * 
 * Implements enemies that move along Bézier curves:
 * - Sphere or cylinder shaped enemies
 * - Movement along Bézier paths
 * - Collision detection with player
 * - Chase AI (follows player)
 * 
 * Computer Graphics Algorithms:
 * - Bézier curves for patrol paths
 * - Vector normalization for direction
 * - Parametric sphere equations for rendering
 * - 3D distance calculations
 ******************************************************************************/

#ifndef ENEMY_H
#define ENEMY_H

#include "matrix.h"
#include "bezier.h"
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// ENEMY TYPES
// ============================================================================
enum EnemyType {
    ENEMY_PATROL,       // Moves along Bézier path
    ENEMY_CIRCULAR,     // Circular patrol
    ENEMY_CHASE,        // Chases player
    ENEMY_GUARD         // Guards area, chases if player is near
};

// ============================================================================
// ENEMY CLASS
// ============================================================================
class Enemy {
public:
    Vec4 position;              // Current position
    Vec4 startPosition;         // Starting position
    Vec4 targetPosition;        // For chase enemies
    float radius;               // Collision radius
    
    BezierCurve path;           // Bézier path for movement
    float pathT;                // Current parameter t on path [0, 1]
    float speed;                // Movement speed
    float baseSpeed;            // Original speed
    int direction;              // 1 = forward, -1 = backward
    
    Color color;                // Enemy color
    bool isAlive;               // Is enemy active
    
    // Enemy type and AI
    EnemyType type;
    float detectionRange;       // Range to detect player
    float chaseRange;           // Range to start chasing
    bool isChasing;             // Currently chasing player
    Vec4 guardPosition;         // Center of guard area
    float guardRadius;          // Radius of guard area
    
    // Animation
    float pulsePhase;           // For pulsing effect
    float rotationY;            // Y rotation for visual variety
    
    // For sphere rendering
    int slices;                 // Longitude divisions
    int stacks;                 // Latitude divisions
    
    Enemy() {
        position = Vec4(0, 0.5f, 0);
        startPosition = position;
        targetPosition = position;
        radius = 0.4f;
        pathT = 0;
        speed = 0.15f;
        baseSpeed = 0.15f;
        direction = 1;
        color = Color(0.8f, 0.1f, 0.1f);
        isAlive = true;
        
        type = ENEMY_PATROL;
        detectionRange = 8.0f;
        chaseRange = 5.0f;
        isChasing = false;
        guardPosition = Vec4(0, 0, 0);
        guardRadius = 3.0f;
        
        pulsePhase = 0;
        rotationY = 0;
        
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
        type = ENEMY_CIRCULAR;
    }
    
    // Setup chase enemy
    void setupChaseEnemy(const Vec4& startPos, float height = 0.5f) {
        position = startPos;
        position.y = height;
        startPosition = position;
        type = ENEMY_CHASE;
        color = Color(1.0f, 0.3f, 0.0f);  // Orange for chase enemies
        speed = 0.12f;  // Slightly slower but persistent
        baseSpeed = speed;
        detectionRange = 10.0f;
    }
    
    // Setup guard enemy
    void setupGuardEnemy(const Vec4& guardPos, float radius, float height = 0.5f) {
        guardPosition = guardPos;
        guardRadius = radius;
        position = guardPos;
        position.y = height;
        startPosition = position;
        type = ENEMY_GUARD;
        color = Color(0.6f, 0.0f, 0.8f);  // Purple for guard enemies
        speed = 0.18f;
        baseSpeed = speed;
        chaseRange = guardRadius * 1.5f;
    }
    
    // ========================================================================
    // UPDATE ENEMY POSITION
    // ========================================================================
    void update(float deltaTime, const Vec4* playerPos = nullptr) {
        if (!isAlive) return;
        
        // Update animation
        pulsePhase += deltaTime * 3.0f;
        if (pulsePhase >= 2.0f * M_PI) pulsePhase -= 2.0f * M_PI;
        rotationY += deltaTime * 45.0f;
        if (rotationY >= 360.0f) rotationY -= 360.0f;
        
        switch (type) {
            case ENEMY_PATROL:
            case ENEMY_CIRCULAR:
                updatePatrol(deltaTime);
                break;
            case ENEMY_CHASE:
                updateChase(deltaTime, playerPos);
                break;
            case ENEMY_GUARD:
                updateGuard(deltaTime, playerPos);
                break;
        }
    }
    
    // Patrol along Bézier path
    void updatePatrol(float deltaTime) {
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
    
    // Chase player
    void updateChase(float deltaTime, const Vec4* playerPos) {
        if (!playerPos) {
            updatePatrol(deltaTime);
            return;
        }
        
        float dist = distanceTo(*playerPos);
        
        if (dist < detectionRange) {
            isChasing = true;
            targetPosition = *playerPos;
        } else {
            isChasing = false;
        }
        
        if (isChasing) {
            // Move towards player
            // Direction vector: D = normalize(P_player - P_enemy)
            Vec4 dir;
            dir.x = playerPos->x - position.x;
            dir.y = 0;  // Stay at same height
            dir.z = playerPos->z - position.z;
            
            // Normalize direction
            float len = sqrt(dir.x * dir.x + dir.z * dir.z);
            if (len > 0.01f) {
                dir.x /= len;
                dir.z /= len;
                
                // Move towards player
                position.x += dir.x * speed * deltaTime * 5.0f;
                position.z += dir.z * speed * deltaTime * 5.0f;
            }
        } else {
            // Return to start position
            Vec4 dir;
            dir.x = startPosition.x - position.x;
            dir.z = startPosition.z - position.z;
            float len = sqrt(dir.x * dir.x + dir.z * dir.z);
            
            if (len > 0.5f) {
                dir.x /= len;
                dir.z /= len;
                position.x += dir.x * speed * deltaTime * 3.0f;
                position.z += dir.z * speed * deltaTime * 3.0f;
            }
        }
    }
    
    // Guard behavior
    void updateGuard(float deltaTime, const Vec4* playerPos) {
        if (!playerPos) return;
        
        float playerDistToGuardArea = sqrt(
            (playerPos->x - guardPosition.x) * (playerPos->x - guardPosition.x) +
            (playerPos->z - guardPosition.z) * (playerPos->z - guardPosition.z)
        );
        
        // Chase if player enters guard area
        if (playerDistToGuardArea < chaseRange) {
            isChasing = true;
            
            Vec4 dir;
            dir.x = playerPos->x - position.x;
            dir.z = playerPos->z - position.z;
            float len = sqrt(dir.x * dir.x + dir.z * dir.z);
            
            if (len > 0.1f) {
                dir.x /= len;
                dir.z /= len;
                position.x += dir.x * speed * deltaTime * 6.0f;
                position.z += dir.z * speed * deltaTime * 6.0f;
            }
        } else {
            isChasing = false;
            
            // Patrol around guard position
            float distToGuard = sqrt(
                (position.x - guardPosition.x) * (position.x - guardPosition.x) +
                (position.z - guardPosition.z) * (position.z - guardPosition.z)
            );
            
            if (distToGuard > guardRadius) {
                // Return to guard area
                Vec4 dir;
                dir.x = guardPosition.x - position.x;
                dir.z = guardPosition.z - position.z;
                float len = sqrt(dir.x * dir.x + dir.z * dir.z);
                dir.x /= len;
                dir.z /= len;
                position.x += dir.x * speed * deltaTime * 4.0f;
                position.z += dir.z * speed * deltaTime * 4.0f;
            } else {
                // Circular patrol around guard point
                float angle = pathT * 2.0f * M_PI;
                pathT += speed * deltaTime * 0.5f;
                if (pathT >= 1.0f) pathT -= 1.0f;
                
                float targetX = guardPosition.x + cos(angle) * guardRadius * 0.8f;
                float targetZ = guardPosition.z + sin(angle) * guardRadius * 0.8f;
                
                position.x += (targetX - position.x) * deltaTime * 2.0f;
                position.z += (targetZ - position.z) * deltaTime * 2.0f;
            }
        }
    }
    
    // Calculate distance to a position
    float distanceTo(const Vec4& pos) const {
        float dx = position.x - pos.x;
        float dy = position.y - pos.y;
        float dz = position.z - pos.z;
        return sqrt(dx * dx + dy * dy + dz * dz);
    }
    
    // Get pulse scale for animation
    float getPulseScale() const {
        return 1.0f + 0.1f * sin(pulsePhase);
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
    
    // Player position for AI
    Vec4 lastPlayerPos;
    
    EnemyManager() {
        lastPlayerPos = Vec4(0, 0, 0);
    }
    
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
    
    // Add chase enemy
    void addChaseEnemy(const Vec4& startPos) {
        Enemy enemy;
        enemy.setupChaseEnemy(startPos);
        enemies.push_back(enemy);
    }
    
    // Add guard enemy
    void addGuardEnemy(const Vec4& guardPos, float radius) {
        Enemy enemy;
        enemy.setupGuardEnemy(guardPos, radius);
        enemies.push_back(enemy);
    }
    
    // Update all enemies
    void update(float deltaTime, const Vec4& playerPos) {
        lastPlayerPos = playerPos;
        for (auto& enemy : enemies) {
            enemy.update(deltaTime, &playerPos);
        }
    }
    
    // Backwards compatible update
    void update(float deltaTime) {
        for (auto& enemy : enemies) {
            enemy.update(deltaTime, nullptr);
        }
    }
    
    // Check collision with player
    bool checkPlayerCollision(const Vec4& playerPos, float playerRadius, bool isInvincible = false) const {
        if (isInvincible) return false;
        
        for (const auto& enemy : enemies) {
            if (enemy.checkCollision(playerPos, playerRadius)) {
                return true;
            }
        }
        return false;
    }
    
    // Get nearest enemy distance (for tension audio/visual feedback)
    float getNearestEnemyDistance(const Vec4& playerPos) const {
        float minDist = 1000.0f;
        for (const auto& enemy : enemies) {
            if (!enemy.isAlive) continue;
            float dist = enemy.distanceTo(playerPos);
            if (dist < minDist) minDist = dist;
        }
        return minDist;
    }
    
    // Check if any enemy is chasing
    bool isAnyChasing() const {
        for (const auto& enemy : enemies) {
            if (enemy.isChasing) return true;
        }
        return false;
    }
    
    // Set speed multiplier for all enemies
    void setSpeedMultiplier(float multiplier) {
        for (auto& enemy : enemies) {
            enemy.speed = enemy.baseSpeed * multiplier;
        }
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
    
    // Get enemy count
    int getCount() const {
        return enemies.size();
    }
    
    // Get active (alive) enemy count
    int getActiveCount() const {
        int count = 0;
        for (const auto& enemy : enemies) {
            if (enemy.isAlive) count++;
        }
        return count;
    }
};

#endif // ENEMY_H
