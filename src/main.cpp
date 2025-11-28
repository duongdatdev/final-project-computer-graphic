/*******************************************************************************
 * THE SHIFTING MAZE - Main Game File
 * 
 * A 3D maze game using C++ and OpenGL (fixed-function pipeline)
 * 
 * Implements Computer Graphics course algorithms:
 * - 3D Transformations (Translation, Scale, Rotation, Arbitrary Axis Rotation)
 * - 3D Viewing (WCS to Observer transformation, θ-φ angles)
 * - Perspective and Orthographic Projection
 * - Bézier Curves and Surfaces (Bernstein, De Casteljau)
 * - Parametric Surfaces (z = sin(x/10)*cos(y/10))
 * - Back-face Culling (dot product method)
 * - Z-buffer (depth testing)
 * - Lambert Shading
 * 
 * Controls:
 * - W/A/S/D: Move
 * - Mouse: Look around
 * - ESC: Exit
 * - R: Restart
 * - M: Toggle mini-map
 ******************************************************************************/

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>

#include "matrix.h"
#include "camera.h"
#include "bezier.h"
#include "lighting.h"
#include "rendering.h"
#include "maze.h"
#include "enemy.h"
#include "hud.h"

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// Window
int windowWidth = 1024;
int windowHeight = 768;
int windowCenterX, windowCenterY;

// Game objects
Camera camera;
Maze maze;
EnemyManager enemies;
HUD hud;
ParametricSurface floor_surface;
Light mainLight;
Light playerLight;

// Materials
Material wallMaterial;
Material floorMaterial;
Material exitMaterial;
Material enemyMaterial;

// Game state
enum GameState {
    STATE_PLAYING,
    STATE_WIN,
    STATE_LOSE
};
GameState gameState = STATE_PLAYING;

// Input
bool keys[256] = {false};
bool mouseWarped = false;
float mouseSensitivity = 0.002f;

// Timing
float lastTime = 0;
float deltaTime = 0;
const float PLAYER_RADIUS = 0.3f;

// ============================================================================
// INITIALIZATION
// ============================================================================

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
    
    // Player point light (follows player)
    playerLight.position = camera.position;
    playerLight.ambient = Color(0.1f, 0.1f, 0.1f);
    playerLight.diffuse = Color(0.8f, 0.7f, 0.6f);
    playerLight.constantAtt = 1.0f;
    playerLight.linearAtt = 0.1f;
    playerLight.quadraticAtt = 0.02f;
    playerLight.isEnabled = true;
}

void initMaze() {
    maze.generate();
    
    // Setup enemies based on maze
    enemies.clear();
    
    // Add enemies at strategic positions
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
    // Setup parametric surface for floor
    // z = sin(x/10) * cos(y/10)
    floor_surface.xMin = maze.offset.x;
    floor_surface.xMax = maze.offset.x + Maze::SIZE * maze.cellSize;
    floor_surface.yMin = maze.offset.z;
    floor_surface.yMax = maze.offset.z + Maze::SIZE * maze.cellSize;
    floor_surface.resolutionX = 40;
    floor_surface.resolutionY = 40;
    floor_surface.amplitude = 0.15f;
    floor_surface.frequencyX = 0.3f;
    floor_surface.frequencyY = 0.3f;
}

void initGame() {
    // Initialize random seed
    srand((unsigned int)time(NULL));
    
    // Initialize components
    initMaterials();
    initLights();
    initMaze();
    initFloor();
    
    // Setup camera at start position
    Vec4 startPos = maze.getStartPosition();
    camera.setPosition(startPos.x, startPos.y, startPos.z);
    camera.theta = 0;
    camera.phi = 0;
    camera.updateLookAt();
    
    // Setup HUD
    hud.setScreenSize(windowWidth, windowHeight);
    hud.reset();
    
    // Reset game state
    gameState = STATE_PLAYING;
}

void init() {
    // OpenGL settings
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    
    // Enable depth testing (Z-buffer) - as per course material
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Enable back-face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    
    // Enable color material
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    
    // Smooth shading (Gouraud)
    glShadeModel(GL_SMOOTH);
    
    // Enable normalization for proper lighting with scaling
    glEnable(GL_NORMALIZE);
    
    // Initialize game
    initGame();
    
    // Calculate window center
    windowCenterX = windowWidth / 2;
    windowCenterY = windowHeight / 2;
}

// ============================================================================
// DRAWING FUNCTIONS
// ============================================================================

// Draw a unit cube centered at origin
void drawUnitCube() {
    glBegin(GL_QUADS);
    
    // Front face (+Z)
    glNormal3f(0, 0, 1);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    
    // Back face (-Z)
    glNormal3f(0, 0, -1);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    
    // Top face (+Y)
    glNormal3f(0, 1, 0);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    
    // Bottom face (-Y)
    glNormal3f(0, -1, 0);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    
    // Right face (+X)
    glNormal3f(1, 0, 0);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    
    // Left face (-X)
    glNormal3f(-1, 0, 0);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    
    glEnd();
}

// Draw wall at position with transformations
void drawWall(float x, float y, float z, float scaleX, float scaleY, float scaleZ, float rotY = 0) {
    glPushMatrix();
    
    // Apply transformations: T * R * S (right to left)
    glTranslatef(x, y, z);
    glRotatef(rotY, 0, 1, 0);
    glScalef(scaleX, scaleY, scaleZ);
    
    drawUnitCube();
    
    glPopMatrix();
}

// Draw sphere using parametric surface formula
// x = r * cos(θ) * sin(φ)
// y = r * cos(φ)
// z = r * sin(θ) * sin(φ)
void drawSphere(float radius, int slices, int stacks) {
    for (int i = 0; i < stacks; i++) {
        float phi1 = M_PI * i / stacks;
        float phi2 = M_PI * (i + 1) / stacks;
        
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; j++) {
            float theta = 2.0f * M_PI * j / slices;
            
            // Vertex 1
            float x1 = radius * cos(theta) * sin(phi1);
            float y1 = radius * cos(phi1);
            float z1 = radius * sin(theta) * sin(phi1);
            glNormal3f(x1 / radius, y1 / radius, z1 / radius);
            glVertex3f(x1, y1, z1);
            
            // Vertex 2
            float x2 = radius * cos(theta) * sin(phi2);
            float y2 = radius * cos(phi2);
            float z2 = radius * sin(theta) * sin(phi2);
            glNormal3f(x2 / radius, y2 / radius, z2 / radius);
            glVertex3f(x2, y2, z2);
        }
        glEnd();
    }
}

// ============================================================================
// DRAW MAZE
// ============================================================================

void drawMaze() {
    float wallHeight = 2.0f;
    float wallWidth = maze.cellSize * 0.9f;
    
    // Set wall material
    glColor3f(wallMaterial.diffuse.r, wallMaterial.diffuse.g, wallMaterial.diffuse.b);
    
    // Draw static walls
    for (int x = 0; x < Maze::SIZE; x++) {
        for (int z = 0; z < Maze::SIZE; z++) {
            int cell = maze.getCell(x, z);
            
            if (cell == CELL_WALL) {
                Vec4 pos = maze.gridToWorld(x, z);
                drawWall(pos.x, wallHeight / 2, pos.z, wallWidth, wallHeight, wallWidth);
            }
            else if (cell == CELL_EXIT) {
                // Draw exit gate (golden)
                Vec4 pos = maze.gridToWorld(x, z);
                glColor3f(exitMaterial.diffuse.r, exitMaterial.diffuse.g, exitMaterial.diffuse.b);
                
                // Exit frame
                drawWall(pos.x - wallWidth / 2, wallHeight / 2, pos.z, 0.2f, wallHeight, wallWidth);
                drawWall(pos.x + wallWidth / 2, wallHeight / 2, pos.z, 0.2f, wallHeight, wallWidth);
                drawWall(pos.x, wallHeight - 0.1f, pos.z, wallWidth, 0.2f, wallWidth);
                
                // Restore wall color
                glColor3f(wallMaterial.diffuse.r, wallMaterial.diffuse.g, wallMaterial.diffuse.b);
            }
        }
    }
    
    // Draw dynamic walls
    for (const auto& wall : maze.dynamicWalls) {
        if (!wall.isVisible) continue;
        
        Vec4 pos = wall.position;
        float scale = wall.scale;
        float rotAngle = wall.rotationAngle;
        
        // Different color for dynamic walls
        if (wall.type == CELL_DYNAMIC_ROTATE) {
            glColor3f(0.5f, 0.4f, 0.6f);  // Purple tint
        } else if (wall.type == CELL_DYNAMIC_SLIDE) {
            glColor3f(0.4f, 0.5f, 0.6f);  // Blue tint
        } else {
            glColor3f(0.6f, 0.5f, 0.4f);  // Orange tint
        }
        
        glPushMatrix();
        glTranslatef(pos.x, wallHeight / 2, pos.z);
        glRotatef(rotAngle, 0, 1, 0);
        glScalef(wallWidth * scale, wallHeight * scale, wallWidth * scale);
        drawUnitCube();
        glPopMatrix();
    }
    
    // Reset color
    glColor3f(1, 1, 1);
}

// ============================================================================
// DRAW FLOOR - Using Parametric Surface
// z = sin(x/10) * cos(y/10)
// ============================================================================

void drawFloor() {
    glColor3f(floorMaterial.diffuse.r, floorMaterial.diffuse.g, floorMaterial.diffuse.b);
    
    float dx = (floor_surface.xMax - floor_surface.xMin) / floor_surface.resolutionX;
    float dy = (floor_surface.yMax - floor_surface.yMin) / floor_surface.resolutionY;
    
    for (int i = 0; i < floor_surface.resolutionX; i++) {
        for (int j = 0; j < floor_surface.resolutionY; j++) {
            float x0 = floor_surface.xMin + i * dx;
            float x1 = floor_surface.xMin + (i + 1) * dx;
            float y0 = floor_surface.yMin + j * dy;
            float y1 = floor_surface.yMin + (j + 1) * dy;
            
            // Compute heights using parametric surface formula
            // h = amplitude * sin(x * freqX) * cos(y * freqY)
            float h00 = floor_surface.computeHeight(x0, y0);
            float h10 = floor_surface.computeHeight(x1, y0);
            float h01 = floor_surface.computeHeight(x0, y1);
            float h11 = floor_surface.computeHeight(x1, y1);
            
            // Compute normals
            Vec4 n00 = floor_surface.computeNormal(x0, y0);
            Vec4 n10 = floor_surface.computeNormal(x1, y0);
            Vec4 n01 = floor_surface.computeNormal(x0, y1);
            Vec4 n11 = floor_surface.computeNormal(x1, y1);
            
            glBegin(GL_QUADS);
            
            glNormal3f(n00.x, n00.y, n00.z);
            glVertex3f(x0, h00, y0);
            
            glNormal3f(n10.x, n10.y, n10.z);
            glVertex3f(x1, h10, y0);
            
            glNormal3f(n11.x, n11.y, n11.z);
            glVertex3f(x1, h11, y1);
            
            glNormal3f(n01.x, n01.y, n01.z);
            glVertex3f(x0, h01, y1);
            
            glEnd();
        }
    }
}

// ============================================================================
// DRAW ENEMIES
// ============================================================================

void drawEnemies() {
    glColor3f(enemyMaterial.diffuse.r, enemyMaterial.diffuse.g, enemyMaterial.diffuse.b);
    
    for (const auto& enemy : enemies.enemies) {
        if (!enemy.isAlive) continue;
        
        glPushMatrix();
        glTranslatef(enemy.position.x, enemy.position.y, enemy.position.z);
        drawSphere(enemy.radius, enemy.slices, enemy.stacks);
        glPopMatrix();
    }
}

// ============================================================================
// DRAW HUD - Using Orthographic Projection
// ============================================================================

void drawText(float x, float y, const char* text, void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(font, *text);
        text++;
    }
}

void drawHUD() {
    // Switch to orthographic projection for 2D HUD
    // x' = x0, y' = y0 (direct mapping)
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Disable depth test and lighting for HUD
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    
    // Draw timer
    Color timerCol = hud.getTimerColor();
    glColor3f(timerCol.r, timerCol.g, timerCol.b);
    std::string timeStr = "Time: " + hud.getTimeString();
    drawText(windowWidth - 150, windowHeight - 30, timeStr.c_str());
    
    // Draw mini-map if enabled
    if (hud.showMiniMap) {
        float mapScale = hud.mapSize / Maze::SIZE;
        float mapStartX = hud.mapX;
        float mapStartY = windowHeight - hud.mapY - hud.mapSize;
        
        // Draw map background
        glColor4f(0.1f, 0.1f, 0.1f, 0.7f);
        glBegin(GL_QUADS);
        glVertex2f(mapStartX, mapStartY);
        glVertex2f(mapStartX + hud.mapSize, mapStartY);
        glVertex2f(mapStartX + hud.mapSize, mapStartY + hud.mapSize);
        glVertex2f(mapStartX, mapStartY + hud.mapSize);
        glEnd();
        
        // Draw cells
        for (int x = 0; x < Maze::SIZE; x++) {
            for (int z = 0; z < Maze::SIZE; z++) {
                int cell = maze.getCell(x, z);
                float cx = mapStartX + x * mapScale;
                float cy = mapStartY + (Maze::SIZE - 1 - z) * mapScale;
                
                if (cell == CELL_WALL || cell == CELL_DYNAMIC_ROTATE || 
                    cell == CELL_DYNAMIC_SLIDE || cell == CELL_DYNAMIC_SCALE) {
                    glColor3f(hud.mapWallColor.r, hud.mapWallColor.g, hud.mapWallColor.b);
                } else if (cell == CELL_EXIT) {
                    glColor3f(hud.mapExitColor.r, hud.mapExitColor.g, hud.mapExitColor.b);
                } else {
                    glColor3f(hud.mapEmptyColor.r, hud.mapEmptyColor.g, hud.mapEmptyColor.b);
                }
                
                glBegin(GL_QUADS);
                glVertex2f(cx, cy);
                glVertex2f(cx + mapScale - 1, cy);
                glVertex2f(cx + mapScale - 1, cy + mapScale - 1);
                glVertex2f(cx, cy + mapScale - 1);
                glEnd();
            }
        }
        
        // Draw player position
        int playerGridX, playerGridZ;
        maze.worldToGrid(camera.position, playerGridX, playerGridZ);
        float px = mapStartX + playerGridX * mapScale + mapScale / 2;
        float py = mapStartY + (Maze::SIZE - 1 - playerGridZ) * mapScale + mapScale / 2;
        
        glColor3f(hud.mapPlayerColor.r, hud.mapPlayerColor.g, hud.mapPlayerColor.b);
        glBegin(GL_TRIANGLE_FAN);
        for (int i = 0; i <= 8; i++) {
            float angle = 2.0f * M_PI * i / 8;
            glVertex2f(px + cos(angle) * 4, py + sin(angle) * 4);
        }
        glEnd();
        
        // Draw enemies on map
        glColor3f(hud.mapEnemyColor.r, hud.mapEnemyColor.g, hud.mapEnemyColor.b);
        for (const auto& enemy : enemies.enemies) {
            int ex, ez;
            maze.worldToGrid(enemy.position, ex, ez);
            float epx = mapStartX + ex * mapScale + mapScale / 2;
            float epy = mapStartY + (Maze::SIZE - 1 - ez) * mapScale + mapScale / 2;
            
            glBegin(GL_TRIANGLE_FAN);
            for (int i = 0; i <= 8; i++) {
                float angle = 2.0f * M_PI * i / 8;
                glVertex2f(epx + cos(angle) * 3, epy + sin(angle) * 3);
            }
            glEnd();
        }
    }
    
    // Draw win/lose message
    if (hud.showWinMessage || hud.showLoseMessage) {
        if (hud.showWinMessage) {
            glColor3f(0.0f, 1.0f, 0.0f);
        } else {
            glColor3f(1.0f, 0.0f, 0.0f);
        }
        drawText(windowWidth / 2 - 150, windowHeight / 2, hud.message.c_str());
    }
    
    // Draw controls hint
    glColor3f(0.7f, 0.7f, 0.7f);
    drawText(10, 20, "WASD: Move | Mouse: Look | M: Map | R: Restart | ESC: Quit", GLUT_BITMAP_HELVETICA_12);
    
    // Restore states
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// ============================================================================
// SETUP LIGHTS - Lambert Shading
// I = Ia * Ka + Ip * Kd * max(0, N · L)
// ============================================================================

void setupLights() {
    // Light 0: Main light (ambient + directional)
    GLfloat light0_position[] = {mainLight.position.x, mainLight.position.y, mainLight.position.z, 0.0f};
    GLfloat light0_ambient[] = {mainLight.ambient.r, mainLight.ambient.g, mainLight.ambient.b, 1.0f};
    GLfloat light0_diffuse[] = {mainLight.diffuse.r, mainLight.diffuse.g, mainLight.diffuse.b, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    
    // Light 1: Player point light (follows camera)
    playerLight.position = camera.position;
    playerLight.position.y += 0.5f;  // Slightly above player
    
    GLfloat light1_position[] = {playerLight.position.x, playerLight.position.y, playerLight.position.z, 1.0f};
    GLfloat light1_ambient[] = {playerLight.ambient.r, playerLight.ambient.g, playerLight.ambient.b, 1.0f};
    GLfloat light1_diffuse[] = {playerLight.diffuse.r, playerLight.diffuse.g, playerLight.diffuse.b, 1.0f};
    
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, playerLight.constantAtt);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, playerLight.linearAtt);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, playerLight.quadraticAtt);
}

// ============================================================================
// DISPLAY CALLBACK
// ============================================================================

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Setup perspective projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(camera.fov, (float)windowWidth / windowHeight, camera.nearPlane, camera.farPlane);
    
    // Setup camera view
    // Implements WCS to Observer transformation using gluLookAt
    // (x0, y0, z0, 1) = (x, y, z, 1) * A * B * C * D
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
        camera.position.x, camera.position.y, camera.position.z,
        camera.lookAt.x, camera.lookAt.y, camera.lookAt.z,
        camera.up.x, camera.up.y, camera.up.z
    );
    
    // Setup lights (Lambert shading through OpenGL)
    setupLights();
    
    // Draw scene
    drawFloor();
    drawMaze();
    drawEnemies();
    
    // Draw HUD (2D overlay with orthographic projection)
    drawHUD();
    
    glutSwapBuffers();
}

// ============================================================================
// UPDATE GAME LOGIC
// ============================================================================

void update(int value) {
    // Calculate delta time
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    
    if (gameState == STATE_PLAYING) {
        // Update HUD timer
        hud.update(deltaTime);
        
        // Check time up
        if (hud.isTimeUp()) {
            gameState = STATE_LOSE;
            hud.setLose("Time's up!");
        }
        
        // Handle movement
        Vec4 oldPos = camera.position;
        
        if (keys['w'] || keys['W']) {
            camera.moveForward(deltaTime);
        }
        if (keys['s'] || keys['S']) {
            camera.moveBackward(deltaTime);
        }
        if (keys['a'] || keys['A']) {
            camera.moveLeft(deltaTime);
        }
        if (keys['d'] || keys['D']) {
            camera.moveRight(deltaTime);
        }
        
        // Collision detection with maze
        if (maze.checkCollision(camera.position, PLAYER_RADIUS)) {
            camera.position = oldPos;
            camera.updateLookAt();
        }
        
        // Update maze (dynamic walls)
        maze.update(deltaTime);
        
        // Update enemies
        enemies.update(deltaTime);
        
        // Check enemy collision
        if (enemies.checkPlayerCollision(camera.position, PLAYER_RADIUS)) {
            gameState = STATE_LOSE;
            hud.setLose("Caught by enemy!");
        }
        
        // Check exit
        if (maze.checkExit(camera.position)) {
            gameState = STATE_WIN;
            hud.setWin();
        }
    }
    
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);  // ~60 FPS
}

// ============================================================================
// INPUT CALLBACKS
// ============================================================================

void keyboard(unsigned char key, int x, int y) {
    keys[key] = true;
    
    if (key == 27) {  // ESC
        exit(0);
    }
    
    if (key == 'r' || key == 'R') {
        initGame();
    }
    
    if (key == 'm' || key == 'M') {
        hud.showMiniMap = !hud.showMiniMap;
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    keys[key] = false;
}

void mouseMotion(int x, int y) {
    if (mouseWarped) {
        mouseWarped = false;
        return;
    }
    
    if (gameState != STATE_PLAYING) return;
    
    // Calculate mouse delta
    int dx = x - windowCenterX;
    int dy = y - windowCenterY;
    
    // Rotate camera based on mouse movement
    // Uses θ (theta) and φ (phi) angles as per course material
    camera.rotate(dx * mouseSensitivity, -dy * mouseSensitivity);
    
    // Warp mouse back to center
    mouseWarped = true;
    glutWarpPointer(windowCenterX, windowCenterY);
}

void mouseButton(int button, int state, int x, int y) {
    // Could be used for shooting or interaction
}

void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    windowCenterX = w / 2;
    windowCenterY = h / 2;
    
    glViewport(0, 0, w, h);
    hud.setScreenSize(w, h);
}

// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("THE SHIFTING MAZE - Computer Graphics Project");
    
    // Initialize OpenGL
    init();
    
    // Register callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutPassiveMotionFunc(mouseMotion);
    glutMouseFunc(mouseButton);
    
    // Hide cursor
    glutSetCursor(GLUT_CURSOR_NONE);
    
    // Start timer
    lastTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    glutTimerFunc(16, update, 0);
    
    // Print info
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
    
    // Start main loop
    glutMainLoop();
    
    return 0;
}
