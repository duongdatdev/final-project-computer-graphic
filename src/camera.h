/*******************************************************************************
 * THE SHIFTING MAZE - Camera/Viewing System Header
 * 
 * Implements 3D viewing exactly as described in Computer Graphics course:
 * - World Coordinate System (WCS) to Observer transformation
 * - (x0, y0, z0, 1) = (x, y, z, 1) * A * B * C * D
 * - Theta (θ) and Phi (φ) angles calculation
 * - Perspective Projection: x' = D/z0 * x0, y' = D/z0 * y0
 * - Orthographic Projection for HUD: x' = x0, y' = y0
 ******************************************************************************/

#ifndef CAMERA_H
#define CAMERA_H

#include "matrix.h"
#include <cmath>

class Camera {
public:
    // Camera position in World Coordinate System
    Vec4 position;
    
    // Look-at point (reference point)
    Vec4 lookAt;
    
    // Up vector
    Vec4 up;
    
    // Camera angles (as per course material)
    float theta;    // Horizontal angle (rotation around Y)
    float phi;      // Vertical angle (elevation)
    
    // Viewing parameters
    float distance;     // Distance from observer to projection plane (D)
    float nearPlane;
    float farPlane;
    float fov;          // Field of view
    
    // Movement speed
    float moveSpeed;
    float rotateSpeed;
    
    Camera() {
        position = Vec4(0, 1.5f, 0);
        lookAt = Vec4(0, 1.5f, -1);
        up = Vec4(0, 1, 0);
        theta = 0;
        phi = 0;
        distance = 1.0f;
        nearPlane = 0.1f;
        farPlane = 100.0f;
        fov = 60.0f;
        moveSpeed = 5.0f;
        rotateSpeed = 0.002f;
    }
    
    // ========================================================================
    // UPDATE CAMERA ANGLES
    // θ (theta) - horizontal rotation
    // φ (phi) - vertical rotation  
    // ========================================================================
    void rotate(float deltaTheta, float deltaPhi) {
        theta += deltaTheta;
        phi += deltaPhi;
        
        // Clamp phi to prevent camera flip
        if (phi > M_PI / 2 - 0.1f) phi = M_PI / 2 - 0.1f;
        if (phi < -M_PI / 2 + 0.1f) phi = -M_PI / 2 + 0.1f;
        
        // Update look-at point based on angles
        updateLookAt();
    }
    
    // ========================================================================
    // CALCULATE LOOK-AT DIRECTION FROM ANGLES
    // Using spherical coordinates:
    // dx = cos(φ) * sin(θ)
    // dy = sin(φ)
    // dz = -cos(φ) * cos(θ)
    // ========================================================================
    void updateLookAt() {
        float dx = cos(phi) * sin(theta);
        float dy = sin(phi);
        float dz = -cos(phi) * cos(theta);
        
        lookAt.x = position.x + dx;
        lookAt.y = position.y + dy;
        lookAt.z = position.z + dz;
    }
    
    // Get forward direction vector
    Vec4 getForward() const {
        Vec4 forward;
        forward.x = cos(phi) * sin(theta);
        forward.y = 0; // For movement, ignore vertical
        forward.z = -cos(phi) * cos(theta);
        forward.normalize();
        return forward;
    }
    
    // Get right direction vector
    Vec4 getRight() const {
        Vec4 right;
        right.x = cos(theta);
        right.y = 0;
        right.z = sin(theta);
        return right;
    }
    
    // ========================================================================
    // MOVE CAMERA
    // ========================================================================
    void moveForward(float delta) {
        Vec4 forward = getForward();
        position.x += forward.x * delta * moveSpeed;
        position.z += forward.z * delta * moveSpeed;
        updateLookAt();
    }
    
    void moveBackward(float delta) {
        moveForward(-delta);
    }
    
    void moveLeft(float delta) {
        Vec4 right = getRight();
        position.x -= right.x * delta * moveSpeed;
        position.z -= right.z * delta * moveSpeed;
        updateLookAt();
    }
    
    void moveRight(float delta) {
        Vec4 right = getRight();
        position.x += right.x * delta * moveSpeed;
        position.z += right.z * delta * moveSpeed;
        updateLookAt();
    }
    
    // ========================================================================
    // WCS TO OBSERVER TRANSFORMATION
    // (x0, y0, z0, 1) = (x, y, z, 1) * A * B * C * D
    // 
    // Matrix A: Translate to origin (move view reference point to origin)
    // Matrix B: Rotate around Y axis by -θ
    // Matrix C: Rotate around X axis by φ
    // Matrix D: Additional transformations if needed
    // ========================================================================
    Matrix4x4 getViewMatrix() const {
        // Step 1: Translate so camera is at origin
        // Matrix A: Translation by -camera position
        Matrix4x4 A = createTranslationMatrix(-position.x, -position.y, -position.z);
        
        // Step 2: Rotate around Y by -theta (to align with viewing direction)
        // Matrix B: Ry(-θ)
        Matrix4x4 B = createRotationYMatrix(-theta);
        
        // Step 3: Rotate around X by phi (elevation)
        // Matrix C: Rx(φ)
        Matrix4x4 C = createRotationXMatrix(phi);
        
        // Step 4: Identity for D (no additional transformation)
        Matrix4x4 D;
        D.setIdentity();
        
        // Combine: V = A * B * C * D
        Matrix4x4 view = multiplyMatrix(A, B);
        view = multiplyMatrix(view, C);
        view = multiplyMatrix(view, D);
        
        return view;
    }
    
    // ========================================================================
    // PERSPECTIVE PROJECTION
    // x' = (D / z0) * x0
    // y' = (D / z0) * y0
    // 
    // D = distance to projection plane
    // ========================================================================
    Vec4 perspectiveProject(const Vec4& point) const {
        Vec4 result;
        if (fabs(point.z) > 0.0001f) {
            result.x = (distance / point.z) * point.x;
            result.y = (distance / point.z) * point.y;
            result.z = point.z;
        }
        return result;
    }
    
    // ========================================================================
    // ORTHOGRAPHIC PROJECTION (for HUD)
    // x' = x0
    // y' = y0
    // ========================================================================
    Vec4 orthographicProject(const Vec4& point) const {
        return Vec4(point.x, point.y, point.z);
    }
    
    // Set position with collision check
    void setPosition(float x, float y, float z) {
        position.x = x;
        position.y = y;
        position.z = z;
        updateLookAt();
    }
};

#endif // CAMERA_H
