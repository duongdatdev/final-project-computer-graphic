/*******************************************************************************
 * THE SHIFTING MAZE - Particle System Header
 * 
 * Implements particle effects for visual feedback:
 * - Item collection sparkles
 * - Player trail when speed boosted
 * - Enemy death effects
 * - Power-up auras
 * 
 * Computer Graphics Algorithms:
 * - Point rendering with size attenuation
 * - 3D transformations for particle positions
 * - Parametric curves for particle motion
 * - Alpha blending for transparency
 ******************************************************************************/

#ifndef PARTICLES_H
#define PARTICLES_H

#include "matrix.h"
#include "lighting.h"
#include <vector>
#include <cmath>
#include <cstdlib>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// PARTICLE TYPES
// ============================================================================
enum ParticleType {
    PARTICLE_SPARKLE,       // Collection effect
    PARTICLE_TRAIL,         // Speed boost trail
    PARTICLE_EXPLOSION,     // Death/hit effect
    PARTICLE_AURA,          // Power-up glow
    PARTICLE_DUST           // Ambient particles
};

// ============================================================================
// SINGLE PARTICLE
// ============================================================================
struct Particle {
    Vec4 position;          // Current position
    Vec4 velocity;          // Movement direction and speed
    Vec4 acceleration;      // Gravity or other forces
    
    Color color;            // Particle color
    float alpha;            // Transparency (0-1)
    float size;             // Particle size
    
    float life;             // Current life (0-1)
    float maxLife;          // Maximum lifetime in seconds
    float decay;            // How fast it fades (life decrease per second)
    
    ParticleType type;
    bool isActive;
    
    Particle() {
        position = Vec4(0, 0, 0);
        velocity = Vec4(0, 0, 0);
        acceleration = Vec4(0, -2.0f, 0);  // Gravity
        
        color = Color(1.0f, 1.0f, 1.0f);
        alpha = 1.0f;
        size = 0.1f;
        
        life = 1.0f;
        maxLife = 1.0f;
        decay = 1.0f;
        
        type = PARTICLE_SPARKLE;
        isActive = false;
    }
    
    void update(float deltaTime) {
        if (!isActive) return;
        
        // Physics update using parametric equations
        // v = v0 + a*t
        velocity.x += acceleration.x * deltaTime;
        velocity.y += acceleration.y * deltaTime;
        velocity.z += acceleration.z * deltaTime;
        
        // p = p0 + v*t
        position.x += velocity.x * deltaTime;
        position.y += velocity.y * deltaTime;
        position.z += velocity.z * deltaTime;
        
        // Life decay
        life -= decay * deltaTime;
        
        // Alpha based on life
        alpha = life;
        
        // Deactivate when dead
        if (life <= 0) {
            isActive = false;
        }
    }
    
    void spawn(const Vec4& pos, const Vec4& vel, const Color& col, 
               float sz, float lifetime, ParticleType t) {
        position = pos;
        velocity = vel;
        color = col;
        size = sz;
        maxLife = lifetime;
        life = 1.0f;
        decay = 1.0f / lifetime;
        type = t;
        isActive = true;
        alpha = 1.0f;
        
        // Type-specific settings
        switch (type) {
            case PARTICLE_SPARKLE:
                acceleration = Vec4(0, 0.5f, 0);  // Float up
                break;
            case PARTICLE_TRAIL:
                acceleration = Vec4(0, 0, 0);
                size *= 0.5f;
                break;
            case PARTICLE_EXPLOSION:
                acceleration = Vec4(0, -3.0f, 0);
                break;
            case PARTICLE_AURA:
                acceleration = Vec4(0, 0, 0);
                break;
            case PARTICLE_DUST:
                acceleration = Vec4(0, -0.2f, 0);
                size *= 0.3f;
                break;
        }
    }
};

// ============================================================================
// PARTICLE SYSTEM
// ============================================================================
class ParticleSystem {
public:
    static const int MAX_PARTICLES = 500;
    Particle particles[MAX_PARTICLES];
    int nextParticle;
    
    ParticleSystem() {
        nextParticle = 0;
    }
    
    // ========================================================================
    // SPAWN PARTICLES
    // ========================================================================
    
    // Spawn single particle
    void spawn(const Vec4& pos, const Vec4& vel, const Color& col, 
               float size, float life, ParticleType type) {
        particles[nextParticle].spawn(pos, vel, col, size, life, type);
        nextParticle = (nextParticle + 1) % MAX_PARTICLES;
    }
    
    // Spawn burst of particles (for collection effects)
    void spawnBurst(const Vec4& pos, const Color& col, int count, 
                    float speed, float size, float life, ParticleType type) {
        for (int i = 0; i < count; i++) {
            // Random direction on sphere surface
            // Using parametric sphere equations
            float theta = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
            float phi = ((float)rand() / RAND_MAX) * M_PI;
            
            Vec4 vel;
            vel.x = speed * sin(phi) * cos(theta);
            vel.y = speed * cos(phi);
            vel.z = speed * sin(phi) * sin(theta);
            
            // Slight color variation
            Color varCol;
            float variation = 0.2f;
            varCol.r = col.r + ((float)rand() / RAND_MAX - 0.5f) * variation;
            varCol.g = col.g + ((float)rand() / RAND_MAX - 0.5f) * variation;
            varCol.b = col.b + ((float)rand() / RAND_MAX - 0.5f) * variation;
            
            // Clamp colors
            varCol.r = fmax(0.0f, fmin(1.0f, varCol.r));
            varCol.g = fmax(0.0f, fmin(1.0f, varCol.g));
            varCol.b = fmax(0.0f, fmin(1.0f, varCol.b));
            
            // Slight size variation
            float varSize = size * (0.8f + ((float)rand() / RAND_MAX) * 0.4f);
            
            spawn(pos, vel, varCol, varSize, life, type);
        }
    }
    
    // Spawn ring of particles (for power-up effects)
    void spawnRing(const Vec4& pos, const Color& col, int count, 
                   float radius, float size, float life, ParticleType type) {
        for (int i = 0; i < count; i++) {
            float angle = (2.0f * M_PI * i) / count;
            
            Vec4 offset;
            offset.x = radius * cos(angle);
            offset.y = 0;
            offset.z = radius * sin(angle);
            
            Vec4 spawnPos;
            spawnPos.x = pos.x + offset.x;
            spawnPos.y = pos.y;
            spawnPos.z = pos.z + offset.z;
            
            // Velocity outward and up
            Vec4 vel;
            vel.x = offset.x * 0.5f;
            vel.y = 1.0f;
            vel.z = offset.z * 0.5f;
            
            spawn(spawnPos, vel, col, size, life, type);
        }
    }
    
    // Trail effect (for speed boost)
    void spawnTrail(const Vec4& pos, const Color& col) {
        Vec4 vel(
            ((float)rand() / RAND_MAX - 0.5f) * 0.5f,
            ((float)rand() / RAND_MAX) * 0.5f,
            ((float)rand() / RAND_MAX - 0.5f) * 0.5f
        );
        spawn(pos, vel, col, 0.1f, 0.5f, PARTICLE_TRAIL);
    }
    
    // ========================================================================
    // EFFECT HELPERS
    // ========================================================================
    
    // Coin collection effect
    void effectCoinCollect(const Vec4& pos) {
        Color gold(1.0f, 0.85f, 0.0f);
        spawnBurst(pos, gold, 20, 2.0f, 0.08f, 0.8f, PARTICLE_SPARKLE);
    }
    
    // Key collection effect
    void effectKeyCollect(const Vec4& pos) {
        Color silver(0.8f, 0.8f, 1.0f);
        spawnBurst(pos, silver, 30, 2.5f, 0.1f, 1.0f, PARTICLE_SPARKLE);
        spawnRing(pos, silver, 16, 0.5f, 0.12f, 0.6f, PARTICLE_SPARKLE);
    }
    
    // Power-up collection effect
    void effectPowerUp(const Vec4& pos, const Color& col) {
        spawnBurst(pos, col, 40, 3.0f, 0.12f, 1.2f, PARTICLE_SPARKLE);
        spawnRing(pos, col, 24, 0.8f, 0.15f, 0.8f, PARTICLE_AURA);
    }
    
    // Player hit effect
    void effectPlayerHit(const Vec4& pos) {
        Color red(1.0f, 0.2f, 0.2f);
        spawnBurst(pos, red, 30, 2.0f, 0.1f, 0.6f, PARTICLE_EXPLOSION);
    }
    
    // Win effect
    void effectWin(const Vec4& pos) {
        Color gold(1.0f, 0.9f, 0.3f);
        Color green(0.3f, 1.0f, 0.5f);
        spawnBurst(pos, gold, 50, 4.0f, 0.15f, 1.5f, PARTICLE_SPARKLE);
        spawnRing(pos, green, 32, 1.0f, 0.2f, 1.0f, PARTICLE_AURA);
    }
    
    // ========================================================================
    // UPDATE ALL PARTICLES
    // ========================================================================
    void update(float deltaTime) {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            particles[i].update(deltaTime);
        }
    }
    
    // ========================================================================
    // RENDER PARTICLES
    // Uses OpenGL point sprites or quads
    // ========================================================================
    void render() {
        // Save state
        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_POINT_BIT);
        
        // Enable blending for transparency (alpha blending)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Disable depth writing but keep testing
        glDepthMask(GL_FALSE);
        
        // Disable lighting for particles (emissive)
        glDisable(GL_LIGHTING);
        
        // Enable point smoothing
        glEnable(GL_POINT_SMOOTH);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        
        // Render each particle as a point or small quad
        glBegin(GL_POINTS);
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (!particles[i].isActive) continue;
            
            Particle& p = particles[i];
            glColor4f(p.color.r, p.color.g, p.color.b, p.alpha);
            glPointSize(p.size * 50.0f * p.life);  // Size decreases with life
            glVertex3f(p.position.x, p.position.y, p.position.z);
        }
        glEnd();
        
        // Restore state
        glDepthMask(GL_TRUE);
        glPopAttrib();
    }
    
    // Render as billboarded quads (better quality)
    void renderQuads(const Vec4& cameraRight, const Vec4& cameraUp) {
        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // Additive blending
        glDepthMask(GL_FALSE);
        glDisable(GL_LIGHTING);
        
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (!particles[i].isActive) continue;
            
            Particle& p = particles[i];
            float halfSize = p.size * p.life;
            
            glColor4f(p.color.r, p.color.g, p.color.b, p.alpha);
            
            // Billboarded quad facing camera
            glBegin(GL_QUADS);
            
            Vec4 corner;
            
            // Bottom-left
            corner.x = p.position.x - cameraRight.x * halfSize - cameraUp.x * halfSize;
            corner.y = p.position.y - cameraRight.y * halfSize - cameraUp.y * halfSize;
            corner.z = p.position.z - cameraRight.z * halfSize - cameraUp.z * halfSize;
            glVertex3f(corner.x, corner.y, corner.z);
            
            // Bottom-right
            corner.x = p.position.x + cameraRight.x * halfSize - cameraUp.x * halfSize;
            corner.y = p.position.y + cameraRight.y * halfSize - cameraUp.y * halfSize;
            corner.z = p.position.z + cameraRight.z * halfSize - cameraUp.z * halfSize;
            glVertex3f(corner.x, corner.y, corner.z);
            
            // Top-right
            corner.x = p.position.x + cameraRight.x * halfSize + cameraUp.x * halfSize;
            corner.y = p.position.y + cameraRight.y * halfSize + cameraUp.y * halfSize;
            corner.z = p.position.z + cameraRight.z * halfSize + cameraUp.z * halfSize;
            glVertex3f(corner.x, corner.y, corner.z);
            
            // Top-left
            corner.x = p.position.x - cameraRight.x * halfSize + cameraUp.x * halfSize;
            corner.y = p.position.y - cameraRight.y * halfSize + cameraUp.y * halfSize;
            corner.z = p.position.z - cameraRight.z * halfSize + cameraUp.z * halfSize;
            glVertex3f(corner.x, corner.y, corner.z);
            
            glEnd();
        }
        
        glDepthMask(GL_TRUE);
        glPopAttrib();
    }
    
    // Clear all particles
    void clear() {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            particles[i].isActive = false;
        }
        nextParticle = 0;
    }
    
    // Count active particles
    int getActiveCount() const {
        int count = 0;
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].isActive) count++;
        }
        return count;
    }
};

#endif // PARTICLES_H
