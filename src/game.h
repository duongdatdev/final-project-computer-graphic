/*******************************************************************************
 * THE SHIFTING MAZE - Game Logic Header
 * 
 * Contains main game class that manages all game components
 ******************************************************************************/

#ifndef GAME_H
#define GAME_H

#include "config.h"
#include "matrix.h"
#include "camera.h"
#include "bezier.h"
#include "lighting.h"
#include "rendering.h"
#include "maze.h"
#include "enemy.h"
#include "hud.h"
#include "input.h"
#include "draw.h"

#include <ctime>
#include <cstdio>

// ============================================================================
// GAME CLASS - Main game controller
// ============================================================================
class Game {
public:
    // Core components
    Camera camera;
    Maze maze;
    EnemyManager enemies;
    HUD hud;
    InputManager input;
    ParametricSurface floorSurface;
    
    // Lighting
    Light mainLight;
    Light playerLight;
    
    // Materials
    Material wallMaterial;
    Material floorMaterial;
    Material exitMaterial;
    Material enemyMaterial;
    
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
        initFloor();
        initCamera();
        initHUD();
        
        state = STATE_PLAYING;
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
        
        // Enemy material - red
        enemyMaterial.ambient = Color(0.3f, 0.0f, 0.0f);
        enemyMaterial.diffuse = Color(0.8f, 0.1f, 0.1f);
        enemyMaterial.specular = Color(0.5f, 0.3f, 0.3f);
        enemyMaterial.shininess = 30.0f;
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
        
        // Setup enemies
        enemies.clear();
        
        Vec4 pos1 = maze.gridToWorld(3, 3);
        Vec4 pos2 = maze.gridToWorld(5, 3);
        enemies.addEnemy(pos1, pos2);
        
        Vec4 pos3 = maze.gridToWorld(7, 7);
        Vec4 pos4 = maze.gridToWorld(7, 5);
        enemies.addEnemy(pos3, pos4);
        
        Vec4 center = maze.gridToWorld(5, 5);
        enemies.addCircularEnemy(center, 1.5f);
    }
    
    void initFloor() {
        floorSurface.xMin = maze.offset.x;
        floorSurface.xMax = maze.offset.x + Maze::SIZE * maze.cellSize;
        floorSurface.yMin = maze.offset.z;
        floorSurface.yMax = maze.offset.z + Maze::SIZE * maze.cellSize;
        floorSurface.resolutionX = Config::FLOOR_RESOLUTION;
        floorSurface.resolutionY = Config::FLOOR_RESOLUTION;
        floorSurface.amplitude = Config::FLOOR_AMPLITUDE;
        floorSurface.frequencyX = Config::FLOOR_FREQ_X;
        floorSurface.frequencyY = Config::FLOOR_FREQ_Y;
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
        hud.gameTime = Config::GAME_TIME;
        hud.reset();
    }
    
    // ========================================================================
    // GAME RESTART
    // ========================================================================
    void restart() {
        initMaze();
        initCamera();
        hud.reset();
        enemies.reset();
        state = STATE_PLAYING;
    }
    
    // ========================================================================
    // UPDATE LOGIC
    // ========================================================================
    void update(float currentTime) {
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        if (state != STATE_PLAYING) return;
        
        // Update HUD timer
        hud.update(deltaTime);
        
        // Check time up
        if (hud.isTimeUp()) {
            state = STATE_LOSE;
            hud.setLose("Time's up!");
            return;
        }
        
        // Handle player movement
        updatePlayer();
        
        // Update maze
        maze.update(deltaTime);
        
        // Update enemies
        enemies.update(deltaTime);
        
        // Check enemy collision
        if (enemies.checkPlayerCollision(camera.position, Config::PLAYER_RADIUS)) {
            state = STATE_LOSE;
            hud.setLose("Caught by enemy!");
            return;
        }
        
        // Check exit
        if (maze.checkExit(camera.position)) {
            state = STATE_WIN;
            hud.setWin();
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
        
        // Collision detection
        if (maze.checkCollision(camera.position, Config::PLAYER_RADIUS)) {
            camera.position = oldPos;
            camera.updateLookAt();
        }
        
        // Handle mouse look
        if (input.mouseDeltaX != 0 || input.mouseDeltaY != 0) {
            camera.rotate(
                input.mouseDeltaX * Config::MOUSE_SENSITIVITY,
                -input.mouseDeltaY * Config::MOUSE_SENSITIVITY
            );
            input.resetMouseDelta();
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
        drawEnemies();
        drawHUD();
        
        glutSwapBuffers();
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
            camera.position.x, camera.position.y, camera.position.z,
            camera.lookAt.x, camera.lookAt.y, camera.lookAt.z,
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
    // DRAW FLOOR - Parametric Surface z = sin(x/10) * cos(y/10)
    // ========================================================================
    void drawFloor() {
        glColor3f(floorMaterial.diffuse.r, floorMaterial.diffuse.g, floorMaterial.diffuse.b);
        
        float dx = (floorSurface.xMax - floorSurface.xMin) / floorSurface.resolutionX;
        float dy = (floorSurface.yMax - floorSurface.yMin) / floorSurface.resolutionY;
        
        for (int i = 0; i < floorSurface.resolutionX; i++) {
            for (int j = 0; j < floorSurface.resolutionY; j++) {
                float x0 = floorSurface.xMin + i * dx;
                float x1 = floorSurface.xMin + (i + 1) * dx;
                float y0 = floorSurface.yMin + j * dy;
                float y1 = floorSurface.yMin + (j + 1) * dy;
                
                float h00 = floorSurface.computeHeight(x0, y0);
                float h10 = floorSurface.computeHeight(x1, y0);
                float h01 = floorSurface.computeHeight(x0, y1);
                float h11 = floorSurface.computeHeight(x1, y1);
                
                Vec4 n00 = floorSurface.computeNormal(x0, y0);
                Vec4 n10 = floorSurface.computeNormal(x1, y0);
                Vec4 n01 = floorSurface.computeNormal(x0, y1);
                Vec4 n11 = floorSurface.computeNormal(x1, y1);
                
                glBegin(GL_QUADS);
                glNormal3f(n00.x, n00.y, n00.z); glVertex3f(x0, h00, y0);
                glNormal3f(n10.x, n10.y, n10.z); glVertex3f(x1, h10, y0);
                glNormal3f(n11.x, n11.y, n11.z); glVertex3f(x1, h11, y1);
                glNormal3f(n01.x, n01.y, n01.z); glVertex3f(x0, h01, y1);
                glEnd();
            }
        }
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
                    glColor3f(exitMaterial.diffuse.r, exitMaterial.diffuse.g, exitMaterial.diffuse.b);
                    
                    // Exit frame
                    drawCube(pos.x - wallWidth / 2, wallHeight / 2, pos.z, 0.2f, wallHeight, wallWidth);
                    drawCube(pos.x + wallWidth / 2, wallHeight / 2, pos.z, 0.2f, wallHeight, wallWidth);
                    drawCube(pos.x, wallHeight - 0.1f, pos.z, wallWidth, 0.2f, wallWidth);
                    
                    glColor3f(wallMaterial.diffuse.r, wallMaterial.diffuse.g, wallMaterial.diffuse.b);
                }
            }
        }
        
        // Dynamic walls
        for (const auto& wall : maze.dynamicWalls) {
            if (!wall.isVisible) continue;
            
            Vec4 pos = wall.position;
            float scale = wall.scale;
            float rotAngle = wall.rotationAngle;
            
            if (wall.type == CELL_DYNAMIC_ROTATE) {
                glColor3f(0.5f, 0.4f, 0.6f);
            } else if (wall.type == CELL_DYNAMIC_SLIDE) {
                glColor3f(0.4f, 0.5f, 0.6f);
            } else {
                glColor3f(0.6f, 0.5f, 0.4f);
            }
            
            drawCube(pos.x, wallHeight / 2, pos.z, 
                     wallWidth * scale, wallHeight * scale, wallWidth * scale, rotAngle);
        }
        
        glColor3f(1, 1, 1);
    }
    
    // ========================================================================
    // DRAW ENEMIES
    // ========================================================================
    void drawEnemies() {
        glColor3f(enemyMaterial.diffuse.r, enemyMaterial.diffuse.g, enemyMaterial.diffuse.b);
        
        for (const auto& enemy : enemies.enemies) {
            if (!enemy.isAlive) continue;
            drawSphereAt(enemy.position.x, enemy.position.y, enemy.position.z,
                         enemy.radius, enemy.slices, enemy.stacks);
        }
    }
    
    // ========================================================================
    // DRAW HUD - Orthographic Projection
    // ========================================================================
    void drawHUD() {
        // Switch to orthographic
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, windowWidth, 0, windowHeight);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        
        // Timer
        Color timerCol = hud.getTimerColor();
        glColor3f(timerCol.r, timerCol.g, timerCol.b);
        std::string timeStr = "Time: " + hud.getTimeString();
        drawText(windowWidth - 150, windowHeight - 30, timeStr.c_str());
        
        // Mini-map
        if (hud.showMiniMap) {
            drawMiniMap();
        }
        
        // Win/Lose message
        if (hud.showWinMessage || hud.showLoseMessage) {
            glColor3f(hud.showWinMessage ? 0.0f : 1.0f, 
                      hud.showWinMessage ? 1.0f : 0.0f, 0.0f);
            drawText(windowWidth / 2 - 150, windowHeight / 2, hud.message.c_str());
        }
        
        // Controls hint
        glColor3f(0.7f, 0.7f, 0.7f);
        drawText(10, 20, "WASD: Move | Mouse: Look | M: Map | R: Restart | ESC: Quit",
                 GLUT_BITMAP_HELVETICA_12);
        
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
    
    void drawMiniMap() {
        float mapScale = hud.mapSize / Maze::SIZE;
        float mapStartX = hud.mapX;
        float mapStartY = windowHeight - hud.mapY - hud.mapSize;
        
        // Background
        glColor4f(0.1f, 0.1f, 0.1f, 0.7f);
        drawQuad2D(mapStartX, mapStartY, hud.mapSize, hud.mapSize);
        
        // Cells
        for (int x = 0; x < Maze::SIZE; x++) {
            for (int z = 0; z < Maze::SIZE; z++) {
                int cell = maze.getCell(x, z);
                float cx = mapStartX + x * mapScale;
                float cy = mapStartY + (Maze::SIZE - 1 - z) * mapScale;
                
                if (cell == CELL_WALL || cell == CELL_DYNAMIC_ROTATE || 
                    cell == CELL_DYNAMIC_SLIDE || cell == CELL_DYNAMIC_SCALE) {
                    glColor3f(0.4f, 0.4f, 0.4f);
                } else if (cell == CELL_EXIT) {
                    glColor3f(1.0f, 1.0f, 0.0f);
                } else {
                    glColor3f(0.2f, 0.2f, 0.2f);
                }
                
                drawQuad2D(cx, cy, mapScale - 1, mapScale - 1);
            }
        }
        
        // Player
        int playerGridX, playerGridZ;
        maze.worldToGrid(camera.position, playerGridX, playerGridZ);
        float px = mapStartX + playerGridX * mapScale + mapScale / 2;
        float py = mapStartY + (Maze::SIZE - 1 - playerGridZ) * mapScale + mapScale / 2;
        
        glColor3f(0.0f, 1.0f, 0.0f);
        drawCircle2D(px, py, 4);
        
        // Enemies
        glColor3f(1.0f, 0.0f, 0.0f);
        for (const auto& enemy : enemies.enemies) {
            int ex, ez;
            maze.worldToGrid(enemy.position, ex, ez);
            float epx = mapStartX + ex * mapScale + mapScale / 2;
            float epy = mapStartY + (Maze::SIZE - 1 - ez) * mapScale + mapScale / 2;
            drawCircle2D(epx, epy, 3);
        }
    }
    
    // ========================================================================
    // INPUT HANDLERS
    // ========================================================================
    void handleKeyDown(unsigned char key) {
        input.keyDown(key);
        
        if (key == 27) {  // ESC
            exit(0);
        }
        if (key == 'r' || key == 'R') {
            restart();
        }
        if (key == 'm' || key == 'M') {
            hud.showMiniMap = !hud.showMiniMap;
        }
    }
    
    void handleKeyUp(unsigned char key) {
        input.keyUp(key);
    }
    
    void handleMouseMove(int x, int y) {
        if (state == STATE_PLAYING) {
            input.mouseMove(x, y);
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
        printf("   THE SHIFTING MAZE - Computer Graphics\n");
        printf("==============================================\n");
        printf("Controls:\n");
        printf("  W/A/S/D - Move\n");
        printf("  Mouse   - Look around\n");
        printf("  M       - Toggle mini-map\n");
        printf("  R       - Restart game\n");
        printf("  ESC     - Quit\n");
        printf("==============================================\n");
        printf("Objective: Find the exit before time runs out!\n");
        printf("Watch out for enemies and shifting walls!\n");
        printf("==============================================\n");
    }
};

#endif // GAME_H
