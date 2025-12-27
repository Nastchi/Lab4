#include "reflection_models.h"
#include <cmath>
#include <algorithm>

static float dot(const Vector3& a, const Vector3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static Vector3 normalize(const Vector3& v) {
    float len = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len > 0.0001f) {
        return Vector3(v.x / len, v.y / len, v.z / len);
    }
    return Vector3(0.0f, 1.0f, 0.0f);
}

static Vector3 cross(const Vector3& a, const Vector3& b) {
    return Vector3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

// Основные функции
Vector3 ReflectionModels::lambert(const Vector3& normal,
    const Vector3& lightDir,
    const Vector3& lightColor,
    float intensity) {
    float dotProduct = dot(normal, lightDir);
    float diffuse = std::max(0.0f, -dotProduct); 

   
    diffuse = pow(diffuse, 0.7f); 

    return Vector3(
        lightColor.x * intensity * diffuse * 2.0f, 
        lightColor.y * intensity * diffuse * 2.0f,
        lightColor.z * intensity * diffuse * 2.0f
    );
}

Vector3 ReflectionModels::phongBlinn(const Vector3& normal,
    const Vector3& lightDir,
    const Vector3& viewDir,
    const Vector3& lightColor,
    float intensity,
    float shininess) {

    Vector3 diffusePart = lambert(normal, lightDir, lightColor, intensity);


    Vector3 halfVector = normalize(Vector3(
        -lightDir.x + viewDir.x,
        -lightDir.y + viewDir.y,
        -lightDir.z + viewDir.z
    ));

    float specularIntensity = dot(normal, halfVector);
    specularIntensity = std::max(0.0f, specularIntensity);


    specularIntensity = pow(specularIntensity, shininess * 0.5f);
    specularIntensity *= 3.0f; 

    Vector3 specularPart = Vector3(
        lightColor.x * intensity * specularIntensity,
        lightColor.y * intensity * specularIntensity,
        lightColor.z * intensity * specularIntensity
    );


    return Vector3(
        diffusePart.x * 0.7f + specularPart.x * 1.3f,
        diffusePart.y * 0.7f + specularPart.y * 1.3f,
        diffusePart.z * 0.7f + specularPart.z * 1.3f
    );
}

Vector3 ReflectionModels::torranceSparrow(const Vector3& normal,
    const Vector3& lightDir,
    const Vector3& viewDir,
    const Vector3& lightColor,
    float intensity,
    float roughness,
    float reflectance) {

   
    Vector3 h = normalize(Vector3(
        -lightDir.x + viewDir.x,
        -lightDir.y + viewDir.y,
        -lightDir.z + viewDir.z
    ));

    float ndotv = dot(normal, viewDir);
    float ndotl = dot(normal, lightDir);
    float ndoth = dot(normal, h);
    float vdoth = dot(viewDir, h);

    ndotv = std::max(0.001f, ndotv);
    ndotl = std::max(0.001f, -ndotl);
    ndoth = std::max(0.001f, ndoth);
    vdoth = std::max(0.001f, vdoth);


    float alpha = roughness * roughness;
    float ndoth2 = ndoth * ndoth;
    float exponent = -(1.0f - ndoth2) / (ndoth2 * alpha);
    float D = exp(exponent) / (3.14159f * alpha * ndoth2 * ndoth2);

    float G = std::min(1.0f, std::min(
        2.0f * ndoth * ndotv / vdoth,
        2.0f * ndoth * ndotl / vdoth
    ));

    Vector3 F0 = Vector3(reflectance, reflectance, reflectance);
    float fresnel = pow(1.0f - vdoth, 5.0f);
    Vector3 F = Vector3(
        F0.x + (1.0f - F0.x) * fresnel,
        F0.y + (1.0f - F0.y) * fresnel,
        F0.z + (1.0f - F0.z) * fresnel
    );


    Vector3 specular = Vector3(
        (D * G * F.x) / (4.0f * ndotv * ndotl) * 4.0f, 
        (D * G * F.y) / (4.0f * ndotv * ndotl) * 4.0f,
        (D * G * F.z) / (4.0f * ndotv * ndotl) * 4.0f
    );


    Vector3 diffuse = lambert(normal, lightDir, lightColor, intensity * 0.3f);

    return Vector3(
        diffuse.x + specular.x * lightColor.x * intensity * 2.0f,
        diffuse.y + specular.y * lightColor.y * intensity * 2.0f,
        diffuse.z + specular.z * lightColor.z * intensity * 2.0f
    );
}

Vector3 ReflectionModels::computeNormal(const Vector3& v1,
    const Vector3& v2,
    const Vector3& v3) {
    Vector3 edge1 = Vector3(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
    Vector3 edge2 = Vector3(v3.x - v1.x, v3.y - v1.y, v3.z - v1.z);
    return normalize(cross(edge1, edge2));
}

float ReflectionModels::clamp(float value, float min, float max) {
    return std::max(min, std::min(value, max));
}