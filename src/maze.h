/*******************************************************************************
 * SIMPLE MAZE - Maze System Header
 * 
 * Implements the maze grid:
 * - 10x10 grid maze
 * - Static walls (3D boxes)
 * - Exit gate
 * - Collision detection
 ******************************************************************************/

#ifndef MAZE_H
#define MAZE_H

#include "matrix.h"
#include <vector>
#include <cstdlib>
#include <ctime>

// Cell types
enum CellType {
    CELL_EMPTY = 0,
    CELL_WALL = 1,
    CELL_START = 5,
    CELL_EXIT = 6
};

// ============================================================================
// MAZE CLASS
// ============================================================================
class Maze {
public:
    static const int SIZE = Config::MAZE_SIZE;     // max_size max_size grid
    int grid[SIZE][SIZE];           // Cell types
    
    float cellSize;                 // Size of each cell in world units
    Vec4 offset;                    // Maze offset in world
    
    int startX, startZ;             // Player start position
    int exitX, exitZ;               // Exit position
    
    Maze() {
        cellSize = Config::CELL_SIZE;
        offset = Vec4(-SIZE * cellSize / 2, 0, -SIZE * cellSize / 2);
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
                    if (cell == CELL_WALL) {
                        
                        Vec4 wallPos = gridToWorld(x, z);
                        float halfCell = cellSize / 2 - 0.1f;
                        
                        // AABB collision
                        if (pos.x + radius > wallPos.x - halfCell &&
                            pos.x - radius < wallPos.x + halfCell &&
                            pos.z + radius > wallPos.z - halfCell &&
                            pos.z - radius < wallPos.z + halfCell) {
                            return true;
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

    // Get a random empty cell
    void getRandomEmptyCell(int& x, int& z) {
        do {
            x = rand() % SIZE;
            z = rand() % SIZE;
        } while (grid[x][z] != CELL_EMPTY);
    }
};

#endif // MAZE_H
