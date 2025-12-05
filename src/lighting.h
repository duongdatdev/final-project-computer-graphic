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

#endif // LIGHTING_H
