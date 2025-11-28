/*******************************************************************************
 * THE SHIFTING MAZE - Door System Header
 * 
 * Implements doors that require keys:
 * - Locked doors block passages
 * - Keys unlock corresponding doors
 * - Door opening animation using transformations
 * 
 * Computer Graphics Algorithms:
 * - 3D transformations for door animation
 * - Rotation around arbitrary axis (door hinge)
 * - Bézier interpolation for smooth animation
 ******************************************************************************/

#ifndef DOOR_H
#define DOOR_H

#include "matrix.h"
#include "bezier.h"
#include "lighting.h"
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// DOOR STATES
// ============================================================================
enum DoorState {
    DOOR_LOCKED,        // Cannot be opened (no key)
    DOOR_UNLOCKED,      // Can be opened (has key)
    DOOR_OPENING,       // Currently animating open
    DOOR_OPEN,          // Fully open
    DOOR_CLOSING,       // Currently animating close
    DOOR_CLOSED         // Closed but can be reopened
};

// ============================================================================
// DOOR CLASS
// ============================================================================
class Door {
public:
    // Position and dimensions
    Vec4 position;          // World position (center of door frame)
    float width;            // Door width
    float height;           // Door height
    float thickness;        // Door thickness
    
    // Grid position
    int gridX, gridZ;
    
    // State
    DoorState state;
    int requiredKeyId;      // Which key opens this door (-1 = any key)
    bool isVertical;        // Door orientation (vertical = along Z axis)
    
    // Animation
    float openAngle;        // Current rotation angle (0 = closed, 90 = open)
    float targetAngle;      // Target angle
    float animationSpeed;   // Degrees per second
    float animationT;       // Bézier parameter for smooth animation
    
    // Visual
    Color frameColor;
    Color doorColor;
    Color lockedColor;
    Color unlockedColor;
    
    // Hinge position (pivot point for rotation)
    Vec4 hingeOffset;       // Offset from center to hinge
    
    Door() {
        position = Vec4(0, 0, 0);
        width = 1.8f;
        height = 2.0f;
        thickness = 0.15f;
        
        gridX = gridZ = 0;
        
        state = DOOR_LOCKED;
        requiredKeyId = -1;
        isVertical = true;
        
        openAngle = 0;
        targetAngle = 0;
        animationSpeed = 90.0f;  // 90 degrees per second
        animationT = 0;
        
        frameColor = Color(0.4f, 0.3f, 0.2f);     // Brown wood
        doorColor = Color(0.5f, 0.35f, 0.25f);
        lockedColor = Color(0.6f, 0.2f, 0.2f);    // Red tint when locked
        unlockedColor = Color(0.2f, 0.5f, 0.3f);  // Green tint when unlocked
        
        hingeOffset = Vec4(-width / 2, 0, 0);
    }
    
    void setPosition(float x, float y, float z, bool vertical = true) {
        position = Vec4(x, y, z);
        isVertical = vertical;
        
        // Set hinge offset based on orientation
        if (isVertical) {
            hingeOffset = Vec4(-width / 2, 0, 0);
        } else {
            hingeOffset = Vec4(0, 0, -width / 2);
        }
    }
    
    // ========================================================================
    // DOOR OPERATIONS
    // ========================================================================
    
    bool tryUnlock(int keyId) {
        if (state != DOOR_LOCKED) return false;
        
        if (requiredKeyId == -1 || requiredKeyId == keyId) {
            state = DOOR_UNLOCKED;
            return true;
        }
        return false;
    }
    
    void open() {
        if (state == DOOR_UNLOCKED || state == DOOR_CLOSED) {
            state = DOOR_OPENING;
            targetAngle = 90.0f;
            animationT = 0;
        }
    }
    
    void close() {
        if (state == DOOR_OPEN) {
            state = DOOR_CLOSING;
            targetAngle = 0;
            animationT = 0;
        }
    }
    
    void toggle() {
        if (state == DOOR_OPEN || state == DOOR_CLOSING) {
            close();
        } else if (state == DOOR_UNLOCKED || state == DOOR_CLOSED) {
            open();
        }
    }
    
    // ========================================================================
    // UPDATE ANIMATION
    // Uses Bézier interpolation for smooth easing
    // ========================================================================
    void update(float deltaTime) {
        if (state == DOOR_OPENING) {
            // Smooth animation using Bézier-like easing
            // f(t) = 3t² - 2t³ (smoothstep)
            animationT += deltaTime / 1.0f;  // 1 second to open
            if (animationT >= 1.0f) {
                animationT = 1.0f;
                state = DOOR_OPEN;
            }
            
            // Smoothstep interpolation
            float t = animationT;
            float smooth = t * t * (3.0f - 2.0f * t);
            openAngle = smooth * targetAngle;
        }
        else if (state == DOOR_CLOSING) {
            animationT += deltaTime / 0.8f;  // Slightly faster close
            if (animationT >= 1.0f) {
                animationT = 1.0f;
                state = DOOR_CLOSED;
            }
            
            float t = animationT;
            float smooth = t * t * (3.0f - 2.0f * t);
            openAngle = 90.0f * (1.0f - smooth);
        }
    }
    
    // ========================================================================
    // COLLISION DETECTION
    // ========================================================================
    bool checkCollision(const Vec4& playerPos, float playerRadius) const {
        // Open doors don't block
        if (state == DOOR_OPEN) return false;
        
        // Partially open doors - check based on angle
        if (openAngle > 45.0f) return false;
        
        // AABB collision for door
        float halfWidth = width / 2;
        float halfThickness = thickness / 2;
        
        if (isVertical) {
            float dx = fabs(playerPos.x - position.x);
            float dz = fabs(playerPos.z - position.z);
            return (dx < halfWidth + playerRadius) && (dz < halfThickness + playerRadius);
        } else {
            float dx = fabs(playerPos.x - position.x);
            float dz = fabs(playerPos.z - position.z);
            return (dx < halfThickness + playerRadius) && (dz < halfWidth + playerRadius);
        }
    }
    
    // Check if player is near door (for interaction)
    bool isPlayerNear(const Vec4& playerPos, float interactDistance = 2.0f) const {
        float dx = playerPos.x - position.x;
        float dz = playerPos.z - position.z;
        float dist = sqrt(dx * dx + dz * dz);
        return dist < interactDistance;
    }
    
    // ========================================================================
    // GET TRANSFORMATION MATRIX
    // Rotation around hinge (arbitrary axis rotation at offset point)
    // T = Tr(-hinge) * Ry(angle) * Tr(hinge) * Tr(position)
    // ========================================================================
    Matrix4x4 getDoorTransformMatrix() const {
        float radAngle = openAngle * M_PI / 180.0f;
        
        // If vertical door, rotate around Y axis at hinge
        if (isVertical) {
            // Move to hinge, rotate, move back, then translate to world position
            Matrix4x4 toOrigin = createTranslationMatrix(-hingeOffset.x, 0, 0);
            Matrix4x4 rotate = createRotationYMatrix(radAngle);
            Matrix4x4 fromOrigin = createTranslationMatrix(hingeOffset.x, 0, 0);
            Matrix4x4 toWorld = createTranslationMatrix(position.x, position.y + height / 2, position.z);
            
            Matrix4x4 result = multiplyMatrix(toOrigin, rotate);
            result = multiplyMatrix(result, fromOrigin);
            result = multiplyMatrix(result, toWorld);
            return result;
        } else {
            // Horizontal door
            Matrix4x4 toOrigin = createTranslationMatrix(0, 0, -hingeOffset.z);
            Matrix4x4 rotate = createRotationYMatrix(radAngle);
            Matrix4x4 fromOrigin = createTranslationMatrix(0, 0, hingeOffset.z);
            Matrix4x4 toWorld = createTranslationMatrix(position.x, position.y + height / 2, position.z);
            
            Matrix4x4 result = multiplyMatrix(toOrigin, rotate);
            result = multiplyMatrix(result, fromOrigin);
            result = multiplyMatrix(result, toWorld);
            return result;
        }
    }
    
    // Get current color based on state
    Color getCurrentColor() const {
        switch (state) {
            case DOOR_LOCKED:
                return lockedColor;
            case DOOR_UNLOCKED:
            case DOOR_CLOSED:
                return unlockedColor;
            default:
                return doorColor;
        }
    }
};

// ============================================================================
// DOOR MANAGER
// ============================================================================
class DoorManager {
public:
    std::vector<Door> doors;
    
    DoorManager() {}
    
    // Add a door at grid position
    void addDoor(float worldX, float worldZ, int gx, int gz, bool vertical = true, int keyId = -1) {
        Door door;
        door.setPosition(worldX, 0, worldZ, vertical);
        door.gridX = gx;
        door.gridZ = gz;
        door.requiredKeyId = keyId;
        doors.push_back(door);
    }
    
    // Update all doors
    void update(float deltaTime) {
        for (auto& door : doors) {
            door.update(deltaTime);
        }
    }
    
    // Try to unlock door at player position
    bool tryUnlockNearby(const Vec4& playerPos, int keyId) {
        for (auto& door : doors) {
            if (door.isPlayerNear(playerPos) && door.state == DOOR_LOCKED) {
                if (door.tryUnlock(keyId)) {
                    return true;
                }
            }
        }
        return false;
    }
    
    // Try to open nearby unlocked door
    bool tryOpenNearby(const Vec4& playerPos) {
        for (auto& door : doors) {
            if (door.isPlayerNear(playerPos)) {
                if (door.state == DOOR_UNLOCKED || door.state == DOOR_CLOSED) {
                    door.open();
                    return true;
                }
            }
        }
        return false;
    }
    
    // Toggle nearby door
    void toggleNearby(const Vec4& playerPos) {
        for (auto& door : doors) {
            if (door.isPlayerNear(playerPos)) {
                door.toggle();
            }
        }
    }
    
    // Check collision with all doors
    bool checkCollision(const Vec4& playerPos, float playerRadius) const {
        for (const auto& door : doors) {
            if (door.checkCollision(playerPos, playerRadius)) {
                return true;
            }
        }
        return false;
    }
    
    // Get door at grid position
    Door* getDoorAt(int gx, int gz) {
        for (auto& door : doors) {
            if (door.gridX == gx && door.gridZ == gz) {
                return &door;
            }
        }
        return nullptr;
    }
    
    // Unlock all doors (cheat/debug)
    void unlockAll() {
        for (auto& door : doors) {
            if (door.state == DOOR_LOCKED) {
                door.state = DOOR_UNLOCKED;
            }
        }
    }
    
    // Reset all doors
    void reset() {
        for (auto& door : doors) {
            door.state = DOOR_LOCKED;
            door.openAngle = 0;
            door.animationT = 0;
        }
    }
    
    void clear() {
        doors.clear();
    }
};

#endif // DOOR_H
