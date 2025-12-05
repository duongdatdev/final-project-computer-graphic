/*******************************************************************************
 * SIMPLE MAZE - Game Logic Header
 * 
 * Contains main game class that manages all game components
 ******************************************************************************/

#ifndef GAME_H
#define GAME_H

#include "config.h"
#include "matrix.h"
#include "camera.h"
#include "lighting.h"
#include "maze.h"
#include "hud.h"
#include "input.h"
#include "draw.h"

#include <ctime>
#include <cstdio>
#include <vector>

// ============================================================================
// GAME ENTITIES
// ============================================================================

struct Monster {
    Vec4 position;
    Vec4 direction;
    float speed;
    float radius;
    
    Monster(float x, float z) {
        position = Vec4(x, 0.5f, z);
        // Random direction
        float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
        direction = Vec4(cos(angle), 0, sin(angle));
        speed = 2.0f;
        radius = 0.3f;
    }
    
    void update(float dt, const Maze& maze) {
        Vec4 nextPos = position + direction * speed * dt;
        
        // Simple bounce logic
        if (maze.checkCollision(nextPos, radius)) {
            // Try to find a new valid direction
            // Reflect direction? Or just random new direction?
            // Let's try random new direction for simplicity
            float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
            direction = Vec4(cos(angle), 0, sin(angle));
        } else {
            position = nextPos;
        }
    }
    
    void draw() {
        glColor3f(1.0f, 0.0f, 0.0f); // Red monster
        drawSphere(position.x, position.y, position.z, radius);
    }
};

struct Key {
    Vec4 position;
    bool collected;
    float radius;
    float rotation;
    
    Key() {
        collected = false;
        radius = 0.3f;
        rotation = 0.0f;
    }
    
    void update(float dt) {
        rotation += 90.0f * dt;
        if (rotation > 360.0f) rotation -= 360.0f;
    }
    
    void draw() {
        if (collected) return;
        
        glPushMatrix();
        glTranslatef(position.x, position.y + 0.5f, position.z); // Floating
        glRotatef(rotation, 0, 1, 0);
        glRotatef(45, 1, 0, 0); // Tilt
        
        // Draw key shape (simple combination of cubes)
        glColor3f(1.0f, 0.8f, 0.0f); // Gold
        drawCube(0, 0, 0, 0.1f, 0.4f, 0.1f); // Shaft
        drawCube(0, 0.2f, 0, 0.3f, 0.1f, 0.1f); // Handle top
        drawCube(0, -0.15f, 0.1f, 0.1f, 0.1f, 0.2f); // Teeth
        
        glPopMatrix();
    }
};

// ============================================================================
// GAME CLASS - Main game controller
// ============================================================================
class Game {
public:
    // Core components
    Camera camera;
    Maze maze;
    HUD hud;
    InputManager input;
    
    // Entities
    std::vector<Monster> monsters;
    Key key;
    bool hasKey;
    
    // Lighting
    Light mainLight;
    Light playerLight;
    
    // Materials
    Material wallMaterial;
    Material floorMaterial;
    Material exitMaterial;
    
    // Game state
    GameState state;
    
    // Timing
    float lastTime;
    float deltaTime;
    
    // Window
    int windowWidth;
    int windowHeight;
    
    Game() {
        windowWidth = Config::WINDOW_WIDTH;
        windowHeight = Config::WINDOW_HEIGHT;
        state = STATE_PLAYING;
        lastTime = 0;
        deltaTime = 0;
    }
    
    // ========================================================================
    // INITIALIZATION
    // ========================================================================
    void init() {
        srand((unsigned int)time(NULL));
        
        initMaterials();
        initLights();
        initMaze();
        initCamera();
        initHUD();
        initEntities();
        
        state = STATE_PLAYING;
    }
    
    void initEntities() {
        monsters.clear();
        hasKey = false;
        key.collected = false;
        
        // Spawn monsters in random empty cells
        for (int i = 0; i < 5; i++) { // 5 monsters
            int x, z;
            maze.getRandomEmptyCell(x, z);
            Vec4 pos = maze.gridToWorld(x, z);
            // Ensure not too close to start
            if (abs(x - maze.startX) > 2 || abs(z - maze.startZ) > 2) {
                monsters.push_back(Monster(pos.x, pos.z));
            } else {
                i--; // Try again
            }
        }
        
        // Spawn key
        int kx, kz;
        do {
            maze.getRandomEmptyCell(kx, kz);
        } while ((abs(kx - maze.startX) < 3 && abs(kz - maze.startZ) < 3) || (kx == maze.exitX && kz == maze.exitZ));
        
        Vec4 keyPos = maze.gridToWorld(kx, kz);
        key.position = keyPos;
    }
    
    void initMaterials() {
        // Wall material - stone-like
        wallMaterial.ambient = Color(0.3f, 0.25f, 0.2f);
        wallMaterial.diffuse = Color(0.6f, 0.5f, 0.4f);
        wallMaterial.specular = Color(0.1f, 0.1f, 0.1f);
        wallMaterial.shininess = 10.0f;
        
        // Floor material
        floorMaterial.ambient = Color(0.2f, 0.3f, 0.2f);
        floorMaterial.diffuse = Color(0.4f, 0.5f, 0.4f);
        floorMaterial.specular = Color(0.1f, 0.1f, 0.1f);
        floorMaterial.shininess = 5.0f;
        
        // Exit material - glowing gold
        exitMaterial.ambient = Color(0.5f, 0.4f, 0.0f);
        exitMaterial.diffuse = Color(1.0f, 0.85f, 0.0f);
        exitMaterial.specular = Color(1.0f, 1.0f, 0.5f);
        exitMaterial.shininess = 50.0f;
    }
    
    void initLights() {
        // Main ambient/directional light
        mainLight.position = Vec4(0, 20, 0);
        mainLight.ambient = Color(0.3f, 0.3f, 0.35f);
        mainLight.diffuse = Color(0.5f, 0.5f, 0.45f);
        mainLight.isEnabled = true;
        
        // Player point light
        playerLight.ambient = Color(0.1f, 0.1f, 0.1f);
        playerLight.diffuse = Color(0.8f, 0.7f, 0.6f);
        playerLight.constantAtt = 1.0f;
        playerLight.linearAtt = 0.1f;
        playerLight.quadraticAtt = 0.02f;
        playerLight.isEnabled = true;
    }
    
    void initMaze() {
        maze.generate();
    }
    
    void initCamera() {
        Vec4 startPos = maze.getStartPosition();
        camera.setPosition(startPos.x, startPos.y, startPos.z);
        camera.theta = 0;
        camera.phi = 0;
        camera.updateLookAt();
        camera.moveSpeed = Config::PLAYER_SPEED;
    }
    
    void initHUD() {
        hud.setScreenSize(windowWidth, windowHeight);
        hud.reset();
    }
    
    // ========================================================================
    // UPDATE LOGIC
    // ========================================================================
    void update(float currentTime) {
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        if (state != STATE_PLAYING) return;
        
        // Handle player movement
        updatePlayer();
        updateEntities();
        
        // Check exit
        if (hasKey && maze.checkExit(camera.position)) {
            printf("You Win!\n");
            state = STATE_WIN;
            // Simple restart
            initMaze();
            initCamera();
            initEntities();
            state = STATE_PLAYING;
        }
        
        // Update player light position
        playerLight.position = camera.position;
        playerLight.position.y += 0.5f;
    }
    
    void updatePlayer() {
        Vec4 oldPos = camera.position;
        
        if (input.isMovingForward()) {
            camera.moveForward(deltaTime);
        }
        if (input.isMovingBackward()) {
            camera.moveBackward(deltaTime);
        }
        if (input.isMovingLeft()) {
            camera.moveLeft(deltaTime);
        }
        if (input.isMovingRight()) {
            camera.moveRight(deltaTime);
        }
        
        // Collision detection with maze
        if (maze.checkCollision(camera.position, Config::PLAYER_RADIUS)) {
            camera.position = oldPos;
            camera.updateLookAt();
        }
        
        // Mouse look is handled in handleMouseMove() for immediate response
    }
    
    void updateEntities() {
        // Update monsters
        for (auto& monster : monsters) {
            monster.update(deltaTime, maze);
            
            // Check collision with player (XZ plane only)
            float dx = monster.position.x - camera.position.x;
            float dz = monster.position.z - camera.position.z;
            float dist = sqrt(dx*dx + dz*dz);
            
            if (dist < monster.radius + Config::PLAYER_RADIUS) {
                printf("Caught by monster!\n");
                // Reset player position or game over
                // For now, just respawn player at start
                Vec4 startPos = maze.getStartPosition();
                camera.setPosition(startPos.x, startPos.y, startPos.z);
            }
        }
        
        // Update key
        if (!hasKey) {
            key.update(deltaTime);
            
            // Check collision with key (XZ plane only)
            float dx = key.position.x - camera.position.x;
            float dz = key.position.z - camera.position.z;
            float dist = sqrt(dx*dx + dz*dz);
            
            if (dist < key.radius + Config::PLAYER_RADIUS) {
                hasKey = true;
                key.collected = true;
                printf("Key collected! The gate is open.\n");
            }
        }
    }
    
    // ========================================================================
    // RENDERING
    // ========================================================================
    void render() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        setupProjection();
        setupCamera();
        setupLights();
        
        drawFloor();
        drawMaze();
        drawEntities();
        
        glutSwapBuffers();
    }
    
    void drawEntities() {
        for (auto& monster : monsters) {
            monster.draw();
        }
        
        if (!hasKey) {
            key.draw();
        }
    }
    
    void setupProjection() {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(Config::FOV, (float)windowWidth / windowHeight, 
                       Config::NEAR_PLANE, Config::FAR_PLANE);
    }
    
    void setupCamera() {
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        gluLookAt(
            camera.position.x, 
            camera.position.y, 
            camera.position.z,
            camera.lookAt.x, 
            camera.lookAt.y, 
            camera.lookAt.z,
            camera.up.x, camera.up.y, camera.up.z
        );
    }
    
    void setupLights() {
        // Light 0: Main light
        GLfloat light0_pos[] = {mainLight.position.x, mainLight.position.y, 
                                mainLight.position.z, 0.0f};
        GLfloat light0_amb[] = {mainLight.ambient.r, mainLight.ambient.g, 
                                mainLight.ambient.b, 1.0f};
        GLfloat light0_diff[] = {mainLight.diffuse.r, mainLight.diffuse.g, 
                                 mainLight.diffuse.b, 1.0f};
        
        glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
        glLightfv(GL_LIGHT0, GL_AMBIENT, light0_amb);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diff);
        
        // Light 1: Player point light
        GLfloat light1_pos[] = {playerLight.position.x, playerLight.position.y, 
                                playerLight.position.z, 1.0f};
        GLfloat light1_amb[] = {playerLight.ambient.r, playerLight.ambient.g, 
                                playerLight.ambient.b, 1.0f};
        GLfloat light1_diff[] = {playerLight.diffuse.r, playerLight.diffuse.g, 
                                 playerLight.diffuse.b, 1.0f};
        
        glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
        glLightfv(GL_LIGHT1, GL_AMBIENT, light1_amb);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diff);
        glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, playerLight.constantAtt);
        glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, playerLight.linearAtt);
        glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, playerLight.quadraticAtt);
    }
    
    // ========================================================================
    // DRAW FLOOR
    // ========================================================================
    void drawFloor() {
        glColor3f(floorMaterial.diffuse.r, floorMaterial.diffuse.g, floorMaterial.diffuse.b);
        
        float size = Maze::SIZE * maze.cellSize;
        float y = 0.0f;
        
        glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glVertex3f(maze.offset.x, y, maze.offset.z);
        glVertex3f(maze.offset.x + size, y, maze.offset.z);
        glVertex3f(maze.offset.x + size, y, maze.offset.z + size);
        glVertex3f(maze.offset.x, y, maze.offset.z + size);
        glEnd();
    }
    
    // ========================================================================
    // DRAW MAZE
    // ========================================================================
    void drawMaze() {
        float wallHeight = Config::WALL_HEIGHT;
        float wallWidth = maze.cellSize * 0.9f;
        
        glColor3f(wallMaterial.diffuse.r, wallMaterial.diffuse.g, wallMaterial.diffuse.b);
        
        // Static walls
        for (int x = 0; x < Maze::SIZE; x++) {
            for (int z = 0; z < Maze::SIZE; z++) {
                int cell = maze.getCell(x, z);
                
                if (cell == CELL_WALL) {
                    Vec4 pos = maze.gridToWorld(x, z);
                    drawCube(pos.x, wallHeight / 2, pos.z, wallWidth, wallHeight, wallWidth);
                }
                else if (cell == CELL_EXIT) {
                    Vec4 pos = maze.gridToWorld(x, z);
                    
                    if (hasKey) {
                        glColor3f(exitMaterial.diffuse.r, exitMaterial.diffuse.g, exitMaterial.diffuse.b); // Gold/Open
                    } else {
                        glColor3f(1.0f, 0.0f, 0.0f); // Red/Locked
                    }
                    
                    // Exit frame
                    drawCube(pos.x - wallWidth / 2, wallHeight / 2, pos.z, 0.2f, wallHeight, wallWidth);
                    drawCube(pos.x + wallWidth / 2, wallHeight / 2, pos.z, 0.2f, wallHeight, wallWidth);
                    drawCube(pos.x, wallHeight - 0.1f, pos.z, wallWidth, 0.2f, wallWidth);
                    
                    glColor3f(wallMaterial.diffuse.r, wallMaterial.diffuse.g, wallMaterial.diffuse.b);
                }
            }
        }
        
        glColor3f(1, 1, 1);
    }
    
    // ========================================================================
    // INPUT HANDLING
    // ========================================================================
    void handleKeyDown(unsigned char key) {
        input.keyDown(key);
        
        if (key == 27) { // ESC
            exit(0);
        }
    }
    
    void handleKeyUp(unsigned char key) {
        input.keyUp(key);
    }
    
    void handleMouseMove(int x, int y) {
        if (state == STATE_PLAYING) {
            input.mouseMove(x, y);
            
            // Apply camera rotation immediately
            if (input.mouseDeltaX != 0 || input.mouseDeltaY != 0) {
                camera.rotate(
                    input.mouseDeltaX * Config::MOUSE_SENSITIVITY,
                    -input.mouseDeltaY * Config::MOUSE_SENSITIVITY
                );
            }
        }
    }
    
    void handleResize(int w, int h) {
        windowWidth = w;
        windowHeight = h;
        glViewport(0, 0, w, h);
        hud.setScreenSize(w, h);
        input.setWindowCenter(w / 2, h / 2);
    }
    
    void printWelcome() {
        printf("==============================================\n");
        printf("   SIMPLE MAZE - Computer Graphics\n");
        printf("==============================================\n");
        printf("Controls:\n");
        printf("  W/A/S/D - Move\n");
        printf("  Mouse   - Look around\n");
        printf("  ESC     - Exit\n");
        printf("==============================================\n");
        printf("Objective: Find the exit!\n");
        printf("==============================================\n");
    }
};

#endif // GAME_H
