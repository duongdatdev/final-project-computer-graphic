/*******************************************************************************
 * THE SHIFTING MAZE - Maze System Header
 * 
 * Implements the maze grid and dynamic walls:
 * - 10x10 or 12x12 grid maze
 * - Static walls (3D boxes)
 * - Dynamic walls with:
 *   - Rotation around Y axis
 *   - Slide along Bézier curves
 *   - Scale fade-out/fade-in
 * - Exit gate
 * - Collision detection
 ******************************************************************************/

#ifndef MAZE_H
#define MAZE_H

#include "matrix.h"
#include "bezier.h"
#include <vector>
#include <cstdlib>
#include <ctime>

// Cell types
enum CellType {
    CELL_EMPTY = 0,
    CELL_WALL = 1,
    CELL_DYNAMIC_ROTATE = 2,
    CELL_DYNAMIC_SLIDE = 3,
    CELL_DYNAMIC_SCALE = 4,
    CELL_START = 5,
    CELL_EXIT = 6,
    CELL_TRAP = 7
};

// ============================================================================
// DYNAMIC WALL - Tường động
// ============================================================================
class DynamicWall {
public:
    int gridX, gridZ;           // Grid position
    Vec4 position;              // World position
    Vec4 originalPosition;      // Original position for reset
    CellType type;              // Type of dynamic behavior
    
    // For rotation
    float rotationAngle;        // Current rotation angle
    float rotationSpeed;        // Degrees per second
    Vec4 rotationAxis;          // Axis of rotation (Y by default)
    
    // For Bézier slide
    BezierCurve slidePath;      // Bézier path for sliding
    float slideT;               // Current parameter t on curve [0, 1]
    float slideSpeed;           // Speed along curve
    int slideDirection;         // 1 = forward, -1 = backward
    
    // For scale animation
    float scale;                // Current scale
    float targetScale;          // Target scale (0 = invisible, 1 = visible)
    float scaleSpeed;           // Scale change per second
    bool isVisible;             // Visibility state
    
    // Timing
    float stateTimer;           // Timer for state changes
    bool isActive;              // Is the wall currently active/solid
    
    DynamicWall() {
        gridX = gridZ = 0;
        position = Vec4(0, 0, 0);
        originalPosition = position;
        type = CELL_WALL;
        
        rotationAngle = 0;
        rotationSpeed = 45.0f;  // 45 degrees per second
        rotationAxis = Vec4(0, 1, 0);
        
        slideT = 0;
        slideSpeed = 0.3f;
        slideDirection = 1;
        
        scale = 1.0f;
        targetScale = 1.0f;
        scaleSpeed = 1.0f;
        isVisible = true;
        
        stateTimer = 0;
        isActive = true;
    }
    
    // Setup Bézier path for sliding wall
    void setupSlidePath(const Vec4& start, const Vec4& end) {
        slidePath.clear();
        slidePath.addPoint(start);
        
        // Add control points for smooth curve
        Vec4 mid1 = start;
        mid1.x += (end.x - start.x) * 0.33f;
        mid1.z += (end.z - start.z) * 0.33f + 1.0f;
        slidePath.addPoint(mid1);
        
        Vec4 mid2 = start;
        mid2.x += (end.x - start.x) * 0.66f;
        mid2.z += (end.z - start.z) * 0.66f - 1.0f;
        slidePath.addPoint(mid2);
        
        slidePath.addPoint(end);
        originalPosition = start;
    }
    
    // ========================================================================
    // UPDATE DYNAMIC WALL
    // ========================================================================
    void update(float deltaTime) {
        switch (type) {
            case CELL_DYNAMIC_ROTATE:
                updateRotation(deltaTime);
                break;
            case CELL_DYNAMIC_SLIDE:
                updateSlide(deltaTime);
                break;
            case CELL_DYNAMIC_SCALE:
                updateScale(deltaTime);
                break;
            default:
                break;
        }
    }
    
    // Update rotation around Y axis
    void updateRotation(float deltaTime) {
        rotationAngle += rotationSpeed * deltaTime;
        if (rotationAngle >= 360.0f) rotationAngle -= 360.0f;
    }
    
    // Update sliding along Bézier curve
    void updateSlide(float deltaTime) {
        slideT += slideSpeed * slideDirection * deltaTime;
        
        if (slideT >= 1.0f) {
            slideT = 1.0f;
            slideDirection = -1;
        } else if (slideT <= 0.0f) {
            slideT = 0.0f;
            slideDirection = 1;
        }
        
        // Update position using Bézier curve
        position = slidePath.compute(slideT);
    }
    
    // Update scale fade
    void updateScale(float deltaTime) {
        stateTimer += deltaTime;
        
        // Change visibility every few seconds
        if (stateTimer > 5.0f) {
            stateTimer = 0;
            targetScale = (targetScale > 0.5f) ? 0.0f : 1.0f;
        }
        
        // Animate scale towards target
        if (scale < targetScale) {
            scale += scaleSpeed * deltaTime;
            if (scale > targetScale) scale = targetScale;
        } else if (scale > targetScale) {
            scale -= scaleSpeed * deltaTime;
            if (scale < targetScale) scale = targetScale;
        }
        
        isVisible = (scale > 0.01f);
        isActive = (scale > 0.5f);
    }
    
    // ========================================================================
    // GET TRANSFORMATION MATRIX
    // Combines translation, rotation, and scale
    // ========================================================================
    Matrix4x4 getTransformMatrix() const {
        // Translation to position
        Matrix4x4 T = createTranslationMatrix(position.x, position.y, position.z);
        
        // Rotation around Y axis at wall center
        float radAngle = rotationAngle * M_PI / 180.0f;
        Matrix4x4 R = createRotationYMatrix(radAngle);
        
        // Scale
        Matrix4x4 S = createScaleMatrix(scale, scale, scale);
        
        // Combine: first scale, then rotate, then translate
        // M = S * R * T (applied right to left)
        Matrix4x4 result = multiplyMatrix(S, R);
        result = multiplyMatrix(result, T);
        
        return result;
    }
    
    // Check collision with player
    bool checkCollision(const Vec4& playerPos, float playerRadius, float wallHalfSize) const {
        if (!isActive) return false;
        
        // Simple AABB collision adjusted for rotation
        float dx = fabs(playerPos.x - position.x);
        float dz = fabs(playerPos.z - position.z);
        
        float effectiveSize = wallHalfSize * scale;
        
        // For rotating walls, use circular collision
        if (type == CELL_DYNAMIC_ROTATE) {
            float dist = sqrt(dx * dx + dz * dz);
            return dist < (playerRadius + effectiveSize * 1.414f);
        }
        
        return (dx < playerRadius + effectiveSize) && (dz < playerRadius + effectiveSize);
    }
};

// ============================================================================
// MAZE CLASS
// ============================================================================
class Maze {
public:
    static const int SIZE = 10;     // 10x10 grid
    int grid[SIZE][SIZE];           // Cell types
    std::vector<DynamicWall> dynamicWalls;
    
    float cellSize;                 // Size of each cell in world units
    Vec4 offset;                    // Maze offset in world
    
    int startX, startZ;             // Player start position
    int exitX, exitZ;               // Exit position
    
    float shiftTimer;               // Timer for maze shifting
    float shiftInterval;            // Interval between shifts (30 seconds)
    
    Maze() {
        cellSize = 2.0f;
        offset = Vec4(-SIZE * cellSize / 2, 0, -SIZE * cellSize / 2);
        shiftTimer = 0;
        shiftInterval = 30.0f;
        startX = 1; startZ = 1;
        exitX = SIZE - 2; exitZ = SIZE - 2;
    }
    
    // ========================================================================
    // GENERATE MAZE
    // ========================================================================
    void generate() {
        srand((unsigned int)time(NULL));
        
        // Initialize with walls
        for (int x = 0; x < SIZE; x++) {
            for (int z = 0; z < SIZE; z++) {
                grid[x][z] = CELL_WALL;
            }
        }
        
        // Generate paths using simple maze algorithm
        generatePaths(1, 1);
        
        // Set start and exit
        grid[startX][startZ] = CELL_START;
        grid[exitX][exitZ] = CELL_EXIT;
        
        // Ensure path to exit exists
        ensurePathToExit();
        
        // Add dynamic walls
        addDynamicWalls();
    }
    
    // Simple recursive backtracking maze generation
    void generatePaths(int x, int z) {
        grid[x][z] = CELL_EMPTY;
        
        // Directions: up, right, down, left
        int dirs[4][2] = {{0, -2}, {2, 0}, {0, 2}, {-2, 0}};
        
        // Shuffle directions
        for (int i = 3; i > 0; i--) {
            int j = rand() % (i + 1);
            std::swap(dirs[i][0], dirs[j][0]);
            std::swap(dirs[i][1], dirs[j][1]);
        }
        
        for (int i = 0; i < 4; i++) {
            int nx = x + dirs[i][0];
            int nz = z + dirs[i][1];
            
            if (nx > 0 && nx < SIZE - 1 && nz > 0 && nz < SIZE - 1) {
                if (grid[nx][nz] == CELL_WALL) {
                    // Carve path
                    grid[x + dirs[i][0] / 2][z + dirs[i][1] / 2] = CELL_EMPTY;
                    generatePaths(nx, nz);
                }
            }
        }
    }
    
    // Ensure there's a path from start to exit
    void ensurePathToExit() {
        // Simple: create a direct path if needed
        int x = startX;
        int z = startZ;
        
        while (x != exitX || z != exitZ) {
            if (x < exitX) {
                x++;
                if (grid[x][z] == CELL_WALL) grid[x][z] = CELL_EMPTY;
            } else if (x > exitX) {
                x--;
                if (grid[x][z] == CELL_WALL) grid[x][z] = CELL_EMPTY;
            }
            
            if (z < exitZ) {
                z++;
                if (grid[x][z] == CELL_WALL) grid[x][z] = CELL_EMPTY;
            } else if (z > exitZ) {
                z--;
                if (grid[x][z] == CELL_WALL) grid[x][z] = CELL_EMPTY;
            }
        }
    }
    
    // Add dynamic walls at random positions
    void addDynamicWalls() {
        dynamicWalls.clear();
        
        int numDynamic = 5;  // Number of dynamic walls
        
        for (int i = 0; i < numDynamic; i++) {
            int attempts = 0;
            while (attempts < 50) {
                int x = 1 + rand() % (SIZE - 2);
                int z = 1 + rand() % (SIZE - 2);
                
                if (grid[x][z] == CELL_WALL) {
                    // Check if it's not blocking essential paths
                    DynamicWall wall;
                    wall.gridX = x;
                    wall.gridZ = z;
                    wall.position = gridToWorld(x, z);
                    wall.position.y = 1.0f;
                    wall.originalPosition = wall.position;
                    
                    // Assign random type
                    int typeChoice = rand() % 3;
                    switch (typeChoice) {
                        case 0:
                            wall.type = CELL_DYNAMIC_ROTATE;
                            grid[x][z] = CELL_DYNAMIC_ROTATE;
                            break;
                        case 1:
                            wall.type = CELL_DYNAMIC_SLIDE;
                            grid[x][z] = CELL_DYNAMIC_SLIDE;
                            // Setup slide path
                            {
                                Vec4 start = wall.position;
                                Vec4 end = start;
                                end.x += (rand() % 2 == 0 ? 1 : -1) * cellSize;
                                wall.setupSlidePath(start, end);
                            }
                            break;
                        case 2:
                            wall.type = CELL_DYNAMIC_SCALE;
                            grid[x][z] = CELL_DYNAMIC_SCALE;
                            break;
                    }
                    
                    dynamicWalls.push_back(wall);
                    break;
                }
                attempts++;
            }
        }
    }
    
    // Convert grid coordinates to world coordinates
    Vec4 gridToWorld(int x, int z) const {
        return Vec4(
            offset.x + x * cellSize + cellSize / 2,
            0,
            offset.z + z * cellSize + cellSize / 2
        );
    }
    
    // Convert world coordinates to grid coordinates
    void worldToGrid(const Vec4& worldPos, int& x, int& z) const {
        x = (int)((worldPos.x - offset.x) / cellSize);
        z = (int)((worldPos.z - offset.z) / cellSize);
    }
    
    // Get player start position in world coordinates
    Vec4 getStartPosition() const {
        Vec4 pos = gridToWorld(startX, startZ);
        pos.y = 1.5f;  // Player eye height
        return pos;
    }
    
    // Get exit position in world coordinates
    Vec4 getExitPosition() const {
        return gridToWorld(exitX, exitZ);
    }
    
    // ========================================================================
    // UPDATE MAZE (shift walls every 30 seconds)
    // ========================================================================
    void update(float deltaTime) {
        // Update dynamic walls
        for (auto& wall : dynamicWalls) {
            wall.update(deltaTime);
        }
        
        // Check for maze shift
        shiftTimer += deltaTime;
        if (shiftTimer >= shiftInterval) {
            shiftTimer = 0;
            triggerShift();
        }
    }
    
    // Trigger maze shift event
    void triggerShift() {
        // Randomly change some walls
        for (auto& wall : dynamicWalls) {
            // Reset and randomize behavior
            wall.stateTimer = 0;
            wall.rotationAngle = 0;
            wall.slideT = 0;
            wall.slideDirection = 1;
            
            // Chance to change type
            if (rand() % 3 == 0) {
                int newType = rand() % 3;
                switch (newType) {
                    case 0: wall.type = CELL_DYNAMIC_ROTATE; break;
                    case 1: wall.type = CELL_DYNAMIC_SLIDE; break;
                    case 2: wall.type = CELL_DYNAMIC_SCALE; break;
                }
            }
        }
        
        // Add a new random trap or close/open passages
        int x = 1 + rand() % (SIZE - 2);
        int z = 1 + rand() % (SIZE - 2);
        if (grid[x][z] == CELL_EMPTY && (x != startX || z != startZ) && (x != exitX || z != exitZ)) {
            // Maybe add a temporary wall
            if (rand() % 2 == 0) {
                DynamicWall newWall;
                newWall.gridX = x;
                newWall.gridZ = z;
                newWall.position = gridToWorld(x, z);
                newWall.position.y = 1.0f;
                newWall.type = CELL_DYNAMIC_SCALE;
                newWall.scale = 0;
                newWall.targetScale = 1.0f;
                dynamicWalls.push_back(newWall);
            }
        }
    }
    
    // ========================================================================
    // COLLISION DETECTION
    // ========================================================================
    bool checkCollision(const Vec4& pos, float radius) const {
        int gx, gz;
        worldToGrid(pos, gx, gz);
        
        // Check nearby cells
        for (int dx = -1; dx <= 1; dx++) {
            for (int dz = -1; dz <= 1; dz++) {
                int x = gx + dx;
                int z = gz + dz;
                
                if (x >= 0 && x < SIZE && z >= 0 && z < SIZE) {
                    int cell = grid[x][z];
                    if (cell == CELL_WALL || cell == CELL_DYNAMIC_ROTATE || 
                        cell == CELL_DYNAMIC_SLIDE || cell == CELL_DYNAMIC_SCALE) {
                        
                        Vec4 wallPos = gridToWorld(x, z);
                        float halfCell = cellSize / 2 - 0.1f;
                        
                        // AABB collision
                        if (pos.x + radius > wallPos.x - halfCell &&
                            pos.x - radius < wallPos.x + halfCell &&
                            pos.z + radius > wallPos.z - halfCell &&
                            pos.z - radius < wallPos.z + halfCell) {
                            
                            // Check if it's a dynamic wall that's currently not blocking
                            bool blocked = true;
                            for (const auto& dw : dynamicWalls) {
                                if (dw.gridX == x && dw.gridZ == z) {
                                    blocked = dw.isActive;
                                    break;
                                }
                            }
                            if (blocked) return true;
                        }
                    }
                }
            }
        }
        
        return false;
    }
    
    // Check if player reached exit
    bool checkExit(const Vec4& pos) const {
        int gx, gz;
        worldToGrid(pos, gx, gz);
        return (gx == exitX && gz == exitZ);
    }
    
    // Get cell type at grid position
    int getCell(int x, int z) const {
        if (x >= 0 && x < SIZE && z >= 0 && z < SIZE) {
            return grid[x][z];
        }
        return CELL_WALL;
    }
};

#endif // MAZE_H
