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
// LAMBERT SHADING - Tô bóng Lambert (Diffuse only)
// I = Ia * Ka + Ip * Kd * max(0, N · L)
// 
// Ia = ambient light intensity
// Ka = ambient reflectivity
// Ip = point light intensity
// Kd = diffuse reflectivity
// N = surface normal (normalized)
// L = light direction (normalized, pointing toward light)
// ============================================================================
inline Color lambertShading(
    const Vec4& point,
    const Vec4& normal,
    const Light& light,
    const Material& material
) {
    // Ambient component: Ia * Ka
    Color ambient = light.ambient * material.ambient;
    
    // Light direction: L = normalize(lightPos - point)
    Vec4 L;
    L.x = light.position.x - point.x;
    L.y = light.position.y - point.y;
    L.z = light.position.z - point.z;
    float distance = L.length();
    L.normalize();
    
    // Normalized surface normal
    Vec4 N = normal.normalized();
    
    // Diffuse component: Ip * Kd * max(0, N · L)
    float NdotL = std::max(0.0f, N.dot(L));
    Color diffuse = light.diffuse * material.diffuse * NdotL;
    
    // Apply attenuation
    float attenuation = light.getAttenuation(distance);
    diffuse = diffuse * attenuation;
    
    // Final color: ambient + diffuse
    Color result = ambient + diffuse;
    result.clamp();
    
    return result;
}

// ============================================================================
// PHONG SHADING FORMULA (for reference)
// I = Ia * Ka + Ip * (Kd * max(0, N · L) + Ks * max(0, R · V)^n)
// ============================================================================
inline Color phongShading(
    const Vec4& point,
    const Vec4& normal,
    const Vec4& viewPos,
    const Light& light,
    const Material& material
) {
    // Ambient component
    Color ambient = light.ambient * material.ambient;
    
    // Light direction
    Vec4 L;
    L.x = light.position.x - point.x;
    L.y = light.position.y - point.y;
    L.z = light.position.z - point.z;
    float distance = L.length();
    L.normalize();
    
    Vec4 N = normal.normalized();
    
    // Diffuse component
    float NdotL = std::max(0.0f, N.dot(L));
    Color diffuse = light.diffuse * material.diffuse * NdotL;
    
    // View direction
    Vec4 V;
    V.x = viewPos.x - point.x;
    V.y = viewPos.y - point.y;
    V.z = viewPos.z - point.z;
    V.normalize();
    
    // Reflection direction: R = 2 * (N · L) * N - L
    Vec4 R;
    float scale = 2.0f * NdotL;
    R.x = scale * N.x - L.x;
    R.y = scale * N.y - L.y;
    R.z = scale * N.z - L.z;
    R.normalize();
    
    // Specular component
    float RdotV = std::max(0.0f, R.dot(V));
    float specFactor = pow(RdotV, material.shininess);
    Color specular = light.specular * material.specular * specFactor;
    
    // Apply attenuation
    float attenuation = light.getAttenuation(distance);
    diffuse = diffuse * attenuation;
    specular = specular * attenuation;
    
    // Final color
    Color result = ambient + diffuse + specular;
    result.clamp();
    
    return result;
}

// ============================================================================
// GOURAUD SHADING HELPER
// Compute lighting at vertices, then interpolate colors across polygon
// ============================================================================
struct GouraudVertex {
    Vec4 position;
    Vec4 normal;
    Color color;
};

// Compute Gouraud vertex color
inline void computeGouraudVertex(
    GouraudVertex& vertex,
    const Light& light,
    const Material& material
) {
    vertex.color = lambertShading(vertex.position, vertex.normal, light, material);
}

// Interpolate color for Gouraud shading (bilinear interpolation)
inline Color interpolateColor(
    const Color& c1, const Color& c2, const Color& c3,
    float u, float v, float w
) {
    return Color(
        c1.r * u + c2.r * v + c3.r * w,
        c1.g * u + c2.g * v + c3.g * w,
        c1.b * u + c2.b * v + c3.b * w
    );
}

#endif // LIGHTING_H
