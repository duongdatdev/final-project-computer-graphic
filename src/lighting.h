/*******************************************************************************
 * THE SHIFTING MAZE - Lighting System Header
 * 
 * Implements lighting exactly as described in Computer Graphics course:
 * - Ambient Light
 * - Point Light (follows player)
 * - Lambert Shading: I = Ia * Ka + Ip * Kd * max(0, N · L)
 * - Gouraud Shading (vertex-based interpolation)
 ******************************************************************************/

#ifndef LIGHTING_H
#define LIGHTING_H

#include "matrix.h"
#include <algorithm>

// Light color structure
struct Color {
    float r, g, b, a;
    
    Color() : r(1), g(1), b(1), a(1) {}
    Color(float _r, float _g, float _b) : r(_r), g(_g), b(_b), a(1) {}
    Color(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
    
    Color operator*(float s) const { return Color(r * s, g * s, b * s, a); }
    Color operator+(const Color& c) const { 
        return Color(
            std::min(1.0f, r + c.r), 
            std::min(1.0f, g + c.g), 
            std::min(1.0f, b + c.b), 
            a
        ); 
    }
    Color operator*(const Color& c) const {
        return Color(r * c.r, g * c.g, b * c.b, a * c.a);
    }
    
    void clamp() {
        r = std::max(0.0f, std::min(1.0f, r));
        g = std::max(0.0f, std::min(1.0f, g));
        b = std::max(0.0f, std::min(1.0f, b));
    }
};

// ============================================================================
// LIGHT CLASS - Represents a light source
// ============================================================================
class Light {
public:
    Vec4 position;      // Light position
    Color ambient;      // Ambient light component (Ia)
    Color diffuse;      // Diffuse light component (Ip)
    Color specular;     // Specular light component (optional)
    
    float constantAtt;  // Constant attenuation
    float linearAtt;    // Linear attenuation
    float quadraticAtt; // Quadratic attenuation
    
    bool isEnabled;
    
    Light() {
        position = Vec4(0, 5, 0);
        ambient = Color(0.2f, 0.2f, 0.2f);
        diffuse = Color(1.0f, 1.0f, 0.9f);
        specular = Color(1.0f, 1.0f, 1.0f);
        constantAtt = 1.0f;
        linearAtt = 0.05f;
        quadraticAtt = 0.01f;
        isEnabled = true;
    }
    
    // Calculate attenuation based on distance
    float getAttenuation(float distance) const {
        return 1.0f / (constantAtt + linearAtt * distance + quadraticAtt * distance * distance);
    }
};

// ============================================================================
// MATERIAL CLASS - Surface material properties
// ============================================================================
class Material {
public:
    Color ambient;   // Ka - ambient reflectivity
    Color diffuse;   // Kd - diffuse reflectivity
    Color specular;  // Ks - specular reflectivity
    float shininess; // Specular exponent
    
    Material() {
        ambient = Color(0.3f, 0.3f, 0.3f);
        diffuse = Color(0.7f, 0.7f, 0.7f);
        specular = Color(0.2f, 0.2f, 0.2f);
        shininess = 32.0f;
    }
    
    // Create material with specific colors
    static Material create(const Color& amb, const Color& diff, const Color& spec, float shine) {
        Material m;
        m.ambient = amb;
        m.diffuse = diff;
        m.specular = spec;
        m.shininess = shine;
        return m;
    }
};

// ============================================================================
// MANUAL LIGHTING CALCULATIONS
// ============================================================================

// Calculate lighting for a single point using Lambert and Phong models
inline Color calculateLighting(const Vec4& position, const Vec4& normal, const Vec4& viewPos, 
                             const Light& light, const Material& material) {
    if (!light.isEnabled) return Color(0,0,0);

    // Ambient
    Color ambient = light.ambient * material.ambient;

    // Light direction
    Vec4 lightDir = light.position - position;
    float distance = lightDir.length();
    lightDir.normalize();

    // Diffuse (Lambert)
    // I = Is * kd * cos(theta)
    // cos(theta) = N . L
    // Use normalized vectors
    Vec4 N = normal;
    N.normalize();
    
    float diff = std::max(0.0f, N.dot(lightDir));
    Color diffuse = light.diffuse * material.diffuse * diff;

    // Specular (Phong)
    // R = 2*(N.L)*N - L
    // I = Is * ks * (R.V)^alpha
    Vec4 viewDir = viewPos - position;
    viewDir.normalize();
    
    // Reflect direction: R = 2*(N.L)*N - L
    Vec4 reflectDir = N * (2.0f * N.dot(lightDir)) - lightDir;
    reflectDir.normalize(); 
    
    float spec = std::pow(std::max(0.0f, viewDir.dot(reflectDir)), material.shininess);
    Color specular = light.specular * material.specular * spec;

    // Attenuation
    float attenuation = light.getAttenuation(distance);

    return ambient + (diffuse + specular) * attenuation;
}

// Check if a face is visible (Back-face culling)
// Returns true if the face is visible from viewPos
inline bool isFaceVisible(const Vec4& p1, const Vec4& p2, const Vec4& p3, const Vec4& viewPos) {
    // Calculate face normal
    Vec4 v1 = p2 - p1;
    Vec4 v2 = p3 - p1;
    Vec4 normal = v1.cross(v2);
    
    // Vector from face to camera
    Vec4 viewDir = viewPos - p1;
    
    // Dot product > 0 means face is pointing towards camera (angle < 90)
    return normal.dot(viewDir) > 0;
}

#endif // LIGHTING_H
