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
        
        glPushMatrix();
        Matrix4x4 T = createTranslationMatrix(position.x, position.y, position.z);
        glMultMatrixf(T.ptr());
        
        // Draw body as sphere
        drawManualSphere(radius, 15, 15);
        
        // Draw spikes (Cones) - CG.5
        for(int i=0; i<8; i++) {
            glPushMatrix();
            float angle = i * 45.0f * 3.14159f / 180.0f;
            Matrix4x4 R = createRotationYMatrix(angle);
            Matrix4x4 T2 = createTranslationMatrix(radius * 0.8f, 0, 0);
            Matrix4x4 R2 = createRotationZMatrix(-90.0f * 3.14159f / 180.0f);
            
            Matrix4x4 M = R * T2 * R2;
            glMultMatrixf(M.ptr());
            
            drawManualCone(radius * 0.3f, radius * 0.6f, 10);
            glPopMatrix();
        }
        
        glPopMatrix();
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
        
        // Manual Matrix Transformation for Key
        // Order: Translate -> Rotate Y -> Rotate X (Tilt)
        
        Matrix4x4 T = createTranslationMatrix(position.x, position.y + 0.5f, position.z);
        Matrix4x4 Ry = createRotationYMatrix(rotation * 3.14159f / 180.0f);
        Matrix4x4 Rx = createRotationXMatrix(45.0f * 3.14159f / 180.0f); // Tilt 45 degrees
        
        // Combine: T * Ry * Rx
        Matrix4x4 M = T * Ry * Rx;
        glMultMatrixf(M.ptr());
        
        // Draw key shape using CG.5 primitives
        glColor3f(1.0f, 0.8f, 0.0f); // Gold
        
        // Shaft (Cylinder)
        glPushMatrix();
        Matrix4x4 T_shaft = createTranslationMatrix(0, 0, 0);
        glMultMatrixf(T_shaft.ptr());
        drawManualCylinder(0.05f, 0.6f, 12);
        glPopMatrix();
        
        // Handle (Torus)
        glPushMatrix();
        Matrix4x4 T_handle = createTranslationMatrix(0, 0.3f, 0);
        Matrix4x4 R_handle = createRotationXMatrix(90.0f * 3.14159f / 180.0f);
        Matrix4x4 M_handle = T_handle * R_handle;
        glMultMatrixf(M_handle.ptr());
        drawManualTorus(0.05f, 0.15f, 10, 20);
        glPopMatrix();
        
        // Teeth (Cube)
        drawCube(0, -0.2f, 0.1f, 0.05f, 0.05f, 0.15f);
        drawCube(0, -0.1f, 0.1f, 0.05f, 0.05f, 0.1f);
        
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
        
        drawHUD();
        
        glutSwapBuffers();
    }
    
    // Draw 2D HUD using CG.3 Algorithms
    void drawHUD() {
        // Switch to 2D Orthographic projection
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, windowWidth, 0, windowHeight);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        
        // Draw Crosshair using Bresenham Line (CG.3)
        glColor3f(0.0f, 1.0f, 0.0f);
        int cx = windowWidth / 2;
        int cy = windowHeight / 2;
        int size = 10;
        
        drawLineBresenham(cx - size, cy, cx + size, cy);
        drawLineBresenham(cx, cy - size, cx, cy + size);
        
        // Draw Circle around crosshair using Midpoint Circle (CG.3)
        drawCircleMidpoint(cx, cy, size + 5);
        
        // Restore 3D state
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
    
    void drawEntities() {
        for (auto& monster : monsters) {
            monster.draw();
        }
        
        if (!hasKey) {
            key.draw();
        }
        
        // Draw a magic Bezier path above the maze (CG.5)
        glColor3f(0.5f, 0.0f, 1.0f);
        glLineWidth(3.0f);
        Vec4 p0(0, 5, 0);
        Vec4 p1(5, 8, 5);
        Vec4 p2(10, 4, -5);
        Vec4 p3(15, 6, 0);
        
        // Animate control points slightly
        float t = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        p1.y += sin(t) * 2.0f;
        p2.y += cos(t) * 2.0f;
        
        drawBezierCurve(p0, p1, p2, p3, 50);
        glLineWidth(1.0f);
    }
    
    void setupProjection() {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        
        // Use custom matrix implementation instead of gluPerspective
        Matrix4x4 projMat = createPerspectiveMatrix(
            Config::FOV, 
            (float)windowWidth / windowHeight, 
            Config::NEAR_PLANE, 
            Config::FAR_PLANE
        );
        
        glLoadMatrixf(projMat.ptr());
    }
    
    void setupCamera() {
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        // Use custom matrix implementation instead of gluLookAt
        Matrix4x4 viewMat = createLookAtMatrix(
            camera.position,
            camera.lookAt,
            camera.up
        );
        
        glLoadMatrixf(viewMat.ptr());
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
