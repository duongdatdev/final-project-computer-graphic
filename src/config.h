/*******************************************************************************
 * THE SHIFTING MAZE - Configuration Header
 * 
 * Contains all game constants and configuration settings
 ******************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// WINDOW SETTINGS
// ============================================================================
namespace Config {
    // Window
    const int WINDOW_WIDTH = 1024;
    const int WINDOW_HEIGHT = 768;
    const char* const WINDOW_TITLE = "THE SHIFTING MAZE - Computer Graphics Project";
    
    // ============================================================================
    // PLAYER SETTINGS
    // ============================================================================
    const float PLAYER_RADIUS = 0.3f;
    const float PLAYER_HEIGHT = 1.5f;
    const float PLAYER_SPEED = 5.0f;
    const float MOUSE_SENSITIVITY = 0.002f;
    
    // ============================================================================
    // MAZE SETTINGS
    // ============================================================================
    const int MAZE_SIZE = 11;
    const float CELL_SIZE = 2.0f;
    const float WALL_HEIGHT = 2.0f;
    
    // ============================================================================
    // GAME SETTINGS
    // ============================================================================
    const float GAME_TIME = 180.0f;      // 3 minutes total
    const int TARGET_FPS = 60;
    const int FRAME_TIME_MS = 1000 / TARGET_FPS;  // ~16ms
    
    // ============================================================================
    // GRAPHICS SETTINGS
    // ============================================================================
    const float NEAR_PLANE = 0.1f;
    const float FAR_PLANE = 100.0f;
    const float FOV = 60.0f;
    
    // ============================================================================
    // HUD SETTINGS
    // ============================================================================
    const float MINIMAP_X = 10.0f;
    const float MINIMAP_Y = 10.0f;
    const float MINIMAP_SIZE = 150.0f;
}

// ============================================================================
// GAME STATES
// ============================================================================
enum GameState {
    STATE_PLAYING,
    STATE_WIN,
    STATE_LOSE,
    STATE_PAUSED
};

#endif // CONFIG_H
