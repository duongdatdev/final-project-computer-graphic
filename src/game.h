/*******************************************************************************
 * THE SHIFTING MAZE - Game Logic Header
 * 
 * Contains main game class that manages all game components
 * 
 * NEW FEATURES:
 * - Multiple levels with increasing difficulty
 * - Collectible items (coins, keys, power-ups)
 * - Door/key mechanics
 * - Particle effects
 * - Menu system
 * - Improved enemy AI (patrol, chase, guard)
 * 
 * Computer Graphics Algorithms:
 * - 3D Transformations
 * - Bézier curves/surfaces
 * - Parametric surfaces
 * - Back-face culling
 * - Z-buffer
 * - Lambert/Gouraud shading
 * - Orthographic projection (HUD/Menu)
 * - Perspective projection (3D view)
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
#include "level.h"
#include "items.h"
#include "particles.h"
#include "door.h"
#include "menu.h"

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
    
    // New systems
    LevelManager levelManager;
    ItemManager items;
    ParticleSystem particles;
    DoorManager doors;
    MenuSystem menu;
    
    // Lighting
    Light mainLight;
    Light playerLight;
    
    // Materials
    Material wallMaterial;
    Material floorMaterial;
    Material exitMaterial;
    Material enemyMaterial;
    Material itemMaterial;
    Material doorMaterial;
    
    // Game state
    GameState state;
    
    // Timing
    float lastTime;
    float deltaTime;
    float totalPlayTime;
    
    // Window
    int windowWidth;
    int windowHeight;
    
    // Player stats
    int score;
    int lives;
    float speedBoostTime;
    float invincibilityTime;
    bool isInvincible;
    float speedMultiplier;
    
    // Effects
    float screenShakeTime;
    float screenShakeIntensity;
    
    // Fog settings
    bool fogEnabled;
    float fogDensity;
    Color fogColor;
    
    Game() {
        windowWidth = Config::WINDOW_WIDTH;
        windowHeight = Config::WINDOW_HEIGHT;
        state = STATE_PLAYING;
        lastTime = 0;
        deltaTime = 0;
        totalPlayTime = 0;
        
        score = 0;
        lives = 3;
        speedBoostTime = 0;
        invincibilityTime = 0;
        isInvincible = false;
        speedMultiplier = 1.0f;
        
        screenShakeTime = 0;
        screenShakeIntensity = 0;
        
        fogEnabled = true;
        fogDensity = 0.02f;
        fogColor = Color(0.05f, 0.05f, 0.1f);
    }
    
    // ========================================================================
    // INITIALIZATION
    // ========================================================================
    void init() {
        srand((unsigned int)time(NULL));
        
        initMaterials();
        initLights();
        
        // Show main menu first
        menu.setScreenSize(windowWidth, windowHeight);
        menu.showMainMenu();
        state = STATE_PAUSED;
    }
    
    void startGame() {
        levelManager.reset();
        score = 0;
        lives = 3;
        loadCurrentLevel();
        menu.hide();
        state = STATE_PLAYING;
    }
    
    void loadCurrentLevel() {
        LevelData& level = levelManager.getCurrentLevel();
        
        // Update maze size
        // Note: Would need to make Maze::SIZE non-const for true dynamic sizing
        // For now, we use the fixed size but adjust other parameters
        
        initMaze();
        initFloor();
        initCamera();
        initItems();
        initDoors();
        initHUD();
        
        // Apply level theme
        applyLevelTheme(level);
        
        // Reset particles
        particles.clear();
        
        // Reset power-ups
        speedBoostTime = 0;
        invincibilityTime = 0;
        isInvincible = false;
        speedMultiplier = 1.0f;
        
        totalPlayTime = 0;
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
        
        // Item material
        itemMaterial.ambient = Color(0.4f, 0.4f, 0.0f);
        itemMaterial.diffuse = Color(1.0f, 0.9f, 0.2f);
        itemMaterial.specular = Color(1.0f, 1.0f, 0.8f);
        itemMaterial.shininess = 60.0f;
        
        // Door material
        doorMaterial.ambient = Color(0.3f, 0.2f, 0.1f);
        doorMaterial.diffuse = Color(0.5f, 0.35f, 0.2f);
        doorMaterial.specular = Color(0.2f, 0.15f, 0.1f);
        doorMaterial.shininess = 15.0f;
    }
    
    void applyLevelTheme(const LevelData& level) {
        wallMaterial.diffuse = level.wallColor;
        wallMaterial.ambient = Color(level.wallColor.r * 0.5f, 
                                     level.wallColor.g * 0.5f, 
                                     level.wallColor.b * 0.5f);
        
        floorMaterial.diffuse = level.floorColor;
        floorMaterial.ambient = Color(level.floorColor.r * 0.5f,
                                      level.floorColor.g * 0.5f,
                                      level.floorColor.b * 0.5f);
        
        fogColor = level.fogColor;
        fogDensity = level.fogDensity;
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
        LevelData& level = levelManager.getCurrentLevel();
        
        maze.shiftInterval = level.shiftInterval;
        maze.generate();
        
        // Setup enemies based on level
        enemies.clear();
        
        // Add patrol enemies
        for (int i = 0; i < level.numEnemies; i++) {
            int x1 = 2 + rand() % (Maze::SIZE - 4);
            int z1 = 2 + rand() % (Maze::SIZE - 4);
            int x2 = 2 + rand() % (Maze::SIZE - 4);
            int z2 = 2 + rand() % (Maze::SIZE - 4);
            
            Vec4 pos1 = maze.gridToWorld(x1, z1);
            Vec4 pos2 = maze.gridToWorld(x2, z2);
            enemies.addEnemy(pos1, pos2);
        }
        
        // Add chase enemies
        for (int i = 0; i < level.numChaseEnemies; i++) {
            int x = 3 + rand() % (Maze::SIZE - 6);
            int z = 3 + rand() % (Maze::SIZE - 6);
            Vec4 pos = maze.gridToWorld(x, z);
            enemies.addChaseEnemy(pos);
        }
        
        // Add a circular patrol enemy
        if (level.numEnemies > 0) {
            Vec4 center = maze.gridToWorld(Maze::SIZE / 2, Maze::SIZE / 2);
            enemies.addCircularEnemy(center, 1.5f);
        }
        
        // Set enemy speed based on level
        enemies.setSpeedMultiplier(level.enemySpeed);
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
        LevelData& level = levelManager.getCurrentLevel();
        
        hud.setScreenSize(windowWidth, windowHeight);
        hud.gameTime = level.gameTime;
        hud.reset();
    }
    
    void initItems() {
        LevelData& level = levelManager.getCurrentLevel();
        
        items.clear();
        items.keysRequired = level.numKeys;
        
        // Add coins at random empty positions
        for (int i = 0; i < level.numCoins; i++) {
            int attempts = 0;
            while (attempts < 50) {
                int x = 1 + rand() % (Maze::SIZE - 2);
                int z = 1 + rand() % (Maze::SIZE - 2);
                
                if (maze.getCell(x, z) == CELL_EMPTY) {
                    Vec4 pos = maze.gridToWorld(x, z);
                    items.addCoin(pos.x, pos.z);
                    break;
                }
                attempts++;
            }
        }
        
        // Add keys
        for (int i = 0; i < level.numKeys; i++) {
            int attempts = 0;
            while (attempts < 50) {
                int x = 2 + rand() % (Maze::SIZE - 4);
                int z = 2 + rand() % (Maze::SIZE - 4);
                
                if (maze.getCell(x, z) == CELL_EMPTY) {
                    Vec4 pos = maze.gridToWorld(x, z);
                    items.addKey(pos.x, pos.z);
                    break;
                }
                attempts++;
            }
        }
        
        // Add power-ups (1-2 per level)
        int numPowerUps = 1 + rand() % 2;
        for (int i = 0; i < numPowerUps; i++) {
            int attempts = 0;
            while (attempts < 50) {
                int x = 2 + rand() % (Maze::SIZE - 4);
                int z = 2 + rand() % (Maze::SIZE - 4);
                
                if (maze.getCell(x, z) == CELL_EMPTY) {
                    Vec4 pos = maze.gridToWorld(x, z);
                    ItemType powerUpType = (rand() % 3 == 0) ? ITEM_SPEED_BOOST : 
                                          (rand() % 2 == 0) ? ITEM_INVINCIBILITY : ITEM_TIME_BONUS;
                    items.addPowerUp(powerUpType, pos.x, pos.z);
                    break;
                }
                attempts++;
            }
        }
    }
    
    void initDoors() {
        LevelData& level = levelManager.getCurrentLevel();
        
        doors.clear();
        
        // Add doors if keys are required
        if (level.numKeys > 0) {
            // Place a door near the exit
            int doorX = maze.exitX - 1;
            int doorZ = maze.exitZ;
            
            if (doorX >= 0 && maze.getCell(doorX, doorZ) == CELL_EMPTY) {
                Vec4 pos = maze.gridToWorld(doorX, doorZ);
                doors.addDoor(pos.x, pos.z, doorX, doorZ, true, -1);
            }
        }
    }
    
    // ========================================================================
    // GAME RESTART
    // ========================================================================
    void restart() {
        loadCurrentLevel();
        state = STATE_PLAYING;
    }
    
    void nextLevel() {
        if (levelManager.nextLevel()) {
            loadCurrentLevel();
            state = STATE_PLAYING;
        } else {
            // All levels complete - show final win
            menu.showWin(score, hud.remainingTime, levelManager.currentLevel + 1, true);
            state = STATE_PAUSED;
        }
    }
    
    // ========================================================================
    // UPDATE LOGIC
    // ========================================================================
    void update(float currentTime) {
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        // Update menu animation
        if (menu.isActive()) {
            menu.update(deltaTime);
            return;
        }
        
        if (state != STATE_PLAYING) return;
        
        totalPlayTime += deltaTime;
        
        // Update HUD timer
        hud.update(deltaTime);
        
        // Check time up
        if (hud.isTimeUp()) {
            onGameOver("Time's up!");
            return;
        }
        
        // Update power-ups
        updatePowerUps();
        
        // Handle player movement
        updatePlayer();
        
        // Update maze
        maze.update(deltaTime);
        
        // Update enemies with player position for AI
        enemies.update(deltaTime, camera.position);
        
        // Update items
        items.update(deltaTime);
        
        // Update doors
        doors.update(deltaTime);
        
        // Update particles
        particles.update(deltaTime);
        
        // Update screen shake
        if (screenShakeTime > 0) {
            screenShakeTime -= deltaTime;
        }
        
        // Check item collection
        checkItemCollection();
        
        // Check enemy collision
        if (!isInvincible && enemies.checkPlayerCollision(camera.position, Config::PLAYER_RADIUS)) {
            onPlayerHit();
            return;
        }
        
        // Check exit
        if (maze.checkExit(camera.position)) {
            // Check if all keys collected
            if (items.hasAllKeys()) {
                onLevelComplete();
            } else {
                // Need more keys - visual feedback
                screenShakeTime = 0.2f;
                screenShakeIntensity = 0.1f;
            }
        }
        
        // Update player light position
        playerLight.position = camera.position;
        playerLight.position.y += 0.5f;
    }
    
    void updatePowerUps() {
        // Speed boost
        if (speedBoostTime > 0) {
            speedBoostTime -= deltaTime;
            if (speedBoostTime <= 0) {
                speedMultiplier = 1.0f;
                camera.moveSpeed = Config::PLAYER_SPEED;
            }
        }
        
        // Invincibility
        if (invincibilityTime > 0) {
            invincibilityTime -= deltaTime;
            if (invincibilityTime <= 0) {
                isInvincible = false;
            }
        }
    }
    
    void checkItemCollection() {
        for (auto& item : items.items) {
            if (item.checkCollision(camera.position, Config::PLAYER_RADIUS)) {
                item.collect();
                
                // Spawn particle effect
                switch (item.type) {
                    case ITEM_COIN:
                        particles.effectCoinCollect(item.position);
                        score += (int)item.value;
                        break;
                    case ITEM_KEY:
                        particles.effectKeyCollect(item.position);
                        items.keysCollected++;
                        // Try to unlock nearby doors
                        doors.tryUnlockNearby(camera.position, items.keysCollected);
                        break;
                    case ITEM_SPEED_BOOST:
                        particles.effectPowerUp(item.position, item.primaryColor);
                        speedBoostTime = item.duration;
                        speedMultiplier = item.value;
                        camera.moveSpeed = Config::PLAYER_SPEED * speedMultiplier;
                        break;
                    case ITEM_INVINCIBILITY:
                        particles.effectPowerUp(item.position, item.primaryColor);
                        invincibilityTime = item.duration;
                        isInvincible = true;
                        break;
                    case ITEM_TIME_BONUS:
                        particles.effectPowerUp(item.position, item.primaryColor);
                        hud.remainingTime += item.value;
                        break;
                    default:
                        break;
                }
            }
        }
    }
    
    void onPlayerHit() {
        lives--;
        particles.effectPlayerHit(camera.position);
        screenShakeTime = 0.3f;
        screenShakeIntensity = 0.2f;
        
        if (lives <= 0) {
            onGameOver("Caught by enemy!");
        } else {
            // Respawn at start with temporary invincibility
            Vec4 startPos = maze.getStartPosition();
            camera.setPosition(startPos.x, startPos.y, startPos.z);
            camera.updateLookAt();
            invincibilityTime = 2.0f;
            isInvincible = true;
        }
    }
    
    void onGameOver(const std::string& reason) {
        state = STATE_LOSE;
        hud.setLose(reason);
        menu.showGameOver(score, hud.remainingTime);
    }
    
    void onLevelComplete() {
        // Calculate score bonus
        int levelScore = levelManager.calculateLevelScore(
            hud.remainingTime, 
            items.coinsCollected, 
            levelManager.getCurrentLevel().numCoins
        );
        score += levelScore;
        levelManager.addScore(levelScore);
        
        particles.effectWin(camera.position);
        
        state = STATE_WIN;
        hud.setWin();
        
        bool isFinal = levelManager.isLastLevel();
        menu.showWin(score, hud.remainingTime, levelManager.currentLevel + 1, isFinal);
    }
    
    void updatePlayer() {
        Vec4 oldPos = camera.position;
        
        float moveSpeed = camera.moveSpeed * items.getSpeedMultiplier();
        camera.moveSpeed = moveSpeed;
        
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
        
        // Collision with doors
        if (doors.checkCollision(camera.position, Config::PLAYER_RADIUS)) {
            camera.position = oldPos;
            camera.updateLookAt();
        }
        
        // Speed boost trail effect
        if (speedBoostTime > 0 && (input.isMovingForward() || input.isMovingBackward() ||
            input.isMovingLeft() || input.isMovingRight())) {
            particles.spawnTrail(camera.position, Color(0.0f, 0.8f, 1.0f));
        }
        
        // Invincibility aura effect
        if (isInvincible && rand() % 5 == 0) {
            Vec4 auraPos = camera.position;
            auraPos.y -= 0.5f;
            particles.spawnTrail(auraPos, Color(1.0f, 0.5f, 0.0f));
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
        
        // Render menu if active
        if (menu.isActive()) {
            // Draw faded game scene behind menu
            setupProjection();
            setupCamera();
            setupLights();
            setupFog();
            
            drawFloor();
            drawMaze();
            drawEnemies();
            
            // Draw menu overlay
            menu.render();
            glutSwapBuffers();
            return;
        }
        
        setupProjection();
        setupCamera();
        setupLights();
        setupFog();
        
        drawFloor();
        drawMaze();
        drawItems();
        drawDoors();
        drawEnemies();
        
        // Draw particles (after 3D scene, before HUD)
        particles.render();
        
        drawHUD();
        
        glutSwapBuffers();
    }
    
    void setupFog() {
        if (fogEnabled) {
            glEnable(GL_FOG);
            glFogi(GL_FOG_MODE, GL_EXP2);
            GLfloat fogCol[] = {fogColor.r, fogColor.g, fogColor.b, 1.0f};
            glFogfv(GL_FOG_COLOR, fogCol);
            glFogf(GL_FOG_DENSITY, fogDensity);
            glHint(GL_FOG_HINT, GL_NICEST);
        } else {
            glDisable(GL_FOG);
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
        
        // Apply screen shake if active
        Vec4 shakeOffset(0, 0, 0);
        if (screenShakeTime > 0) {
            shakeOffset.x = (float)(rand() % 100 - 50) / 500.0f * screenShakeIntensity;
            shakeOffset.y = (float)(rand() % 100 - 50) / 500.0f * screenShakeIntensity;
        }
        
        gluLookAt(
            camera.position.x + shakeOffset.x, 
            camera.position.y + shakeOffset.y, 
            camera.position.z,
            camera.lookAt.x + shakeOffset.x, 
            camera.lookAt.y + shakeOffset.y, 
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
        for (const auto& enemy : enemies.enemies) {
            if (!enemy.isAlive) continue;
            
            // Different colors for different enemy types
            Color col = enemy.color;
            if (enemy.isChasing) {
                // Brighter when chasing
                col.r = fmin(1.0f, col.r * 1.5f);
                col.g = fmin(1.0f, col.g * 1.5f);
            }
            
            glColor3f(col.r, col.g, col.b);
            
            float scale = enemy.getPulseScale();
            drawSphereAt(enemy.position.x, enemy.position.y, enemy.position.z,
                         enemy.radius * scale, enemy.slices, enemy.stacks);
        }
    }
    
    // ========================================================================
    // DRAW ITEMS
    // ========================================================================
    void drawItems() {
        for (const auto& item : items.items) {
            if (!item.isActive || item.isCollected) continue;
            
            float pulse = item.getPulseIntensity();
            Color col = item.primaryColor;
            glColor3f(col.r * pulse, col.g * pulse, col.b * pulse);
            
            glPushMatrix();
            glTranslatef(item.position.x, item.position.y, item.position.z);
            glRotatef(item.rotationY, 0, 1, 0);
            
            float scale = item.scale * (1.0f + 0.1f * sin(item.pulsePhase));
            glScalef(scale, scale, scale);
            
            // Different shapes for different items
            switch (item.type) {
                case ITEM_COIN:
                    // Flat cylinder (coin shape)
                    drawCylinder(0.2f, 0.05f, 16);
                    break;
                case ITEM_KEY:
                    // Simple key shape (cylinder + small box)
                    drawCylinder(0.05f, 0.3f, 8);
                    glTranslatef(0, 0.2f, 0);
                    drawCube(0, 0, 0, 0.15f, 0.1f, 0.05f);
                    break;
                case ITEM_SPEED_BOOST:
                case ITEM_INVINCIBILITY:
                case ITEM_TIME_BONUS:
                    // Star/orb shape
                    drawSphere(0.15f, 12, 6);
                    break;
                default:
                    drawSphere(0.15f, 8, 4);
                    break;
            }
            
            glPopMatrix();
        }
    }
    
    // ========================================================================
    // DRAW DOORS
    // ========================================================================
    void drawDoors() {
        for (const auto& door : doors.doors) {
            // Door frame
            glColor3f(doorMaterial.diffuse.r, doorMaterial.diffuse.g, doorMaterial.diffuse.b);
            
            float frameWidth = 0.15f;
            float height = door.height;
            float width = door.width;
            
            // Left frame
            drawCube(door.position.x - width/2, height/2, door.position.z,
                     frameWidth, height, frameWidth);
            // Right frame
            drawCube(door.position.x + width/2, height/2, door.position.z,
                     frameWidth, height, frameWidth);
            // Top frame
            drawCube(door.position.x, height, door.position.z,
                     width + frameWidth, frameWidth, frameWidth);
            
            // Door panel (with animation)
            Color doorCol = door.getCurrentColor();
            glColor3f(doorCol.r, doorCol.g, doorCol.b);
            
            glPushMatrix();
            
            // Apply door transform (rotation around hinge)
            glTranslatef(door.position.x - width/2 + 0.1f, height/2, door.position.z);
            glRotatef(door.openAngle, 0, 1, 0);
            glTranslatef(width/2 - 0.1f, 0, 0);
            
            drawCube(0, 0, 0, width - 0.2f, height - 0.2f, door.thickness);
            
            glPopMatrix();
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
        glDisable(GL_FOG);
        
        // Timer
        Color timerCol = hud.getTimerColor();
        glColor3f(timerCol.r, timerCol.g, timerCol.b);
        std::string timeStr = "Time: " + hud.getTimeString();
        drawText(windowWidth - 150, windowHeight - 30, timeStr.c_str());
        
        // Level info
        glColor3f(0.8f, 0.8f, 0.8f);
        char levelStr[64];
        sprintf(levelStr, "Level %d: %s", 
                levelManager.currentLevel + 1,
                levelManager.getCurrentLevel().levelName.c_str());
        drawText(10, windowHeight - 30, levelStr);
        
        // Score
        char scoreStr[32];
        sprintf(scoreStr, "Score: %d", score);
        drawText(10, windowHeight - 55, scoreStr);
        
        // Lives
        glColor3f(1.0f, 0.3f, 0.3f);
        char livesStr[32];
        sprintf(livesStr, "Lives: %d", lives);
        drawText(10, windowHeight - 80, livesStr);
        
        // Keys collected
        if (items.keysRequired > 0) {
            glColor3f(0.8f, 0.8f, 1.0f);
            char keysStr[32];
            sprintf(keysStr, "Keys: %d/%d", items.keysCollected, items.keysRequired);
            drawText(10, windowHeight - 105, keysStr);
        }
        
        // Power-up indicators
        if (speedBoostTime > 0) {
            glColor3f(0.0f, 0.8f, 1.0f);
            char boostStr[32];
            sprintf(boostStr, "SPEED BOOST: %.1f", speedBoostTime);
            drawText(windowWidth/2 - 80, windowHeight - 60, boostStr);
        }
        
        if (isInvincible && invincibilityTime > 0) {
            glColor3f(1.0f, 0.5f, 0.0f);
            char invStr[32];
            sprintf(invStr, "INVINCIBLE: %.1f", invincibilityTime);
            drawText(windowWidth/2 - 70, windowHeight - 85, invStr);
        }
        
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
        glColor3f(0.5f, 0.5f, 0.5f);
        drawText(10, 20, "WASD: Move | Mouse: Look | M: Map | E: Interact | P: Pause | R: Restart | ESC: Menu",
                 GLUT_BITMAP_HELVETICA_12);
        
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        if (fogEnabled) glEnable(GL_FOG);
        
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
        
        // Items on minimap
        glColor3f(1.0f, 0.85f, 0.0f);  // Gold for coins
        for (const auto& item : items.items) {
            if (item.isCollected || !item.isActive) continue;
            int ix, iz;
            maze.worldToGrid(item.position, ix, iz);
            float ipx = mapStartX + ix * mapScale + mapScale / 2;
            float ipy = mapStartY + (Maze::SIZE - 1 - iz) * mapScale + mapScale / 2;
            
            if (item.type == ITEM_KEY) {
                glColor3f(0.7f, 0.7f, 1.0f);  // Silver for keys
            } else if (item.type == ITEM_COIN) {
                glColor3f(1.0f, 0.85f, 0.0f);  // Gold for coins
            } else {
                glColor3f(0.0f, 1.0f, 1.0f);  // Cyan for power-ups
            }
            drawCircle2D(ipx, ipy, 2);
        }
        
        // Doors on minimap
        glColor3f(0.5f, 0.35f, 0.25f);
        for (const auto& door : doors.doors) {
            if (door.state == DOOR_OPEN) continue;
            float dpx = mapStartX + door.gridX * mapScale + mapScale / 2;
            float dpy = mapStartY + (Maze::SIZE - 1 - door.gridZ) * mapScale + mapScale / 2;
            
            if (door.state == DOOR_LOCKED) {
                glColor3f(0.8f, 0.2f, 0.2f);
            } else {
                glColor3f(0.2f, 0.8f, 0.3f);
            }
            drawQuad2D(dpx - 2, dpy - 2, 4, 4);
        }
        
        // Player
        int playerGridX, playerGridZ;
        maze.worldToGrid(camera.position, playerGridX, playerGridZ);
        float px = mapStartX + playerGridX * mapScale + mapScale / 2;
        float py = mapStartY + (Maze::SIZE - 1 - playerGridZ) * mapScale + mapScale / 2;
        
        if (isInvincible) {
            glColor3f(1.0f, 0.5f, 0.0f);  // Orange when invincible
        } else {
            glColor3f(0.0f, 1.0f, 0.0f);  // Green normally
        }
        drawCircle2D(px, py, 4);
        
        // Enemies
        for (const auto& enemy : enemies.enemies) {
            if (!enemy.isAlive) continue;
            int ex, ez;
            maze.worldToGrid(enemy.position, ex, ez);
            float epx = mapStartX + ex * mapScale + mapScale / 2;
            float epy = mapStartY + (Maze::SIZE - 1 - ez) * mapScale + mapScale / 2;
            
            if (enemy.isChasing) {
                glColor3f(1.0f, 0.5f, 0.0f);  // Orange when chasing
            } else {
                glColor3f(1.0f, 0.0f, 0.0f);  // Red normally
            }
            drawCircle2D(epx, epy, 3);
        }
    }
    
    // ========================================================================
    // INPUT HANDLERS
    // ========================================================================
    void handleKeyDown(unsigned char key) {
        // Menu navigation
        if (menu.isActive()) {
            if (key == 13) {  // Enter
                handleMenuSelect(menu.select());
            } else if (key == 27) {  // ESC
                if (menu.currentMenu == MENU_PAUSE) {
                    menu.hide();
                    state = STATE_PLAYING;
                } else if (menu.currentMenu != MENU_MAIN) {
                    menu.showMainMenu();
                }
            }
            return;
        }
        
        input.keyDown(key);
        
        if (key == 27) {  // ESC - Show pause menu
            menu.showPauseMenu();
            state = STATE_PAUSED;
        }
        
        if (key == 'r' || key == 'R') {
            restart();
        }
        
        if (key == 'm' || key == 'M') {
            hud.showMiniMap = !hud.showMiniMap;
        }
        
        if (key == 'p' || key == 'P') {
            if (state == STATE_PLAYING) {
                menu.showPauseMenu();
                state = STATE_PAUSED;
            }
        }
        
        if (key == 'e' || key == 'E') {
            // Interact with doors
            doors.tryOpenNearby(camera.position);
        }
    }
    
    void handleSpecialKeyDown(int key) {
        // Menu navigation with arrow keys
        if (menu.isActive()) {
            if (key == GLUT_KEY_UP) {
                menu.navigateUp();
            } else if (key == GLUT_KEY_DOWN) {
                menu.navigateDown();
            }
        }
    }
    
    void handleMenuSelect(int action) {
        switch (action) {
            case ACTION_START_GAME:
                startGame();
                break;
            case ACTION_CONTINUE:
            case ACTION_RESUME:
                menu.hide();
                state = STATE_PLAYING;
                break;
            case ACTION_RESTART:
                restart();
                menu.hide();
                break;
            case ACTION_NEXT_LEVEL:
                nextLevel();
                menu.hide();
                break;
            case ACTION_LEVEL_SELECT:
                menu.showLevelSelect(levelManager.highestUnlocked);
                break;
            case ACTION_MAIN_MENU:
                menu.showMainMenu();
                state = STATE_PAUSED;
                break;
            case ACTION_QUIT:
                exit(0);
                break;
            case ACTION_SELECT_LEVEL_1:
            case ACTION_SELECT_LEVEL_2:
            case ACTION_SELECT_LEVEL_3:
            case ACTION_SELECT_LEVEL_4:
            case ACTION_SELECT_LEVEL_5:
                levelManager.selectLevel(action - ACTION_SELECT_LEVEL_1);
                loadCurrentLevel();
                menu.hide();
                state = STATE_PLAYING;
                break;
        }
    }
    
    void handleKeyUp(unsigned char key) {
        input.keyUp(key);
    }
    
    void handleMouseMove(int x, int y) {
        if (state == STATE_PLAYING && !menu.isActive()) {
            input.mouseMove(x, y);
        }
    }
    
    void handleResize(int w, int h) {
        windowWidth = w;
        windowHeight = h;
        glViewport(0, 0, w, h);
        hud.setScreenSize(w, h);
        menu.setScreenSize(w, h);
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
        printf("  E       - Interact (open doors)\n");
        printf("  P       - Pause\n");
        printf("  R       - Restart level\n");
        printf("  ESC     - Menu\n");
        printf("==============================================\n");
        printf("NEW FEATURES:\n");
        printf("  - 5 Levels with increasing difficulty\n");
        printf("  - Collectible coins and keys\n");
        printf("  - Power-ups (speed, invincibility, time)\n");
        printf("  - Doors that need keys\n");
        printf("  - Smarter enemies (patrol, chase, guard)\n");
        printf("  - Particle effects\n");
        printf("  - Fog and visual themes per level\n");
        printf("==============================================\n");
        printf("Objective: Find the exit before time runs out!\n");
        printf("Collect keys to unlock doors!\n");
        printf("Watch out for enemies and shifting walls!\n");
        printf("==============================================\n");
    }
};

#endif // GAME_H
