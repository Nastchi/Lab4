#include "lighting_model.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

static float dot(const Vector3& a, const Vector3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static Vector3 normalize(const Vector3& v) {
    float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len < 1e-10f) return Vector3(0.0f, 1.0f, 0.0f);
    return Vector3(v.x / len, v.y / len, v.z / len);
}

// Добавьте оператор умножения Vector3 на скаляр
static Vector3 operator*(const Vector3& v, float s) {
    return Vector3(v.x * s, v.y * s, v.z * s);
}

Vector3 LightingModel::calculateColor(const Vector3& normal,
    const Vector3& lightDir,
    const Vector3& viewDir,
    const Vector3& surfaceColor,
    Model model,
    float lightIntensity,
    float shininess,
    float roughness,
    float reflectance) const {

    Vector3 norm = normalize(normal);
    Vector3 L = normalize(lightDir);
    Vector3 V = normalize(viewDir);

    switch (model) {
    case LAMBERT:
        return lambert(norm, L, surfaceColor, lightIntensity);
    case PHONG_BLINN:
        return phongBlinn(norm, L, V, surfaceColor, lightIntensity, shininess);
    case TORRANCE_SPARROW:
        return torranceSparrow(norm, L, V, surfaceColor, lightIntensity, roughness, reflectance);
    default:
        return surfaceColor * lightIntensity; // Теперь это будет работать
    }
}

Vector3 LightingModel::lambert(const Vector3& normal,
    const Vector3& lightDir,
    const Vector3& surfaceColor,
    float intensity) const {

    float NdotL = std::max(0.0f, dot(normal, lightDir));
    float diffuse = NdotL * intensity;

    return Vector3(
        surfaceColor.x * diffuse,
        surfaceColor.y * diffuse,
        surfaceColor.z * diffuse
    );
}

Vector3 LightingModel::phongBlinn(const Vector3& normal,
    const Vector3& lightDir,
    const Vector3& viewDir,
    const Vector3& surfaceColor,
    float intensity,
    float shininess) const {

    // Диффузная составляющая
    Vector3 diffuse = lambert(normal, lightDir, surfaceColor, intensity);

    // Полусуммарный вектор (Blinn)
    Vector3 H = normalize(Vector3(
        lightDir.x + viewDir.x,
        lightDir.y + viewDir.y,
        lightDir.z + viewDir.z
    ));

    float NdotH = std::max(0.0f, dot(normal, H));
    float specular = powf(NdotH, shininess) * intensity * 0.5f;

    return Vector3(
        diffuse.x + specular,
        diffuse.y + specular,
        diffuse.z + specular
    );
}

Vector3 LightingModel::torranceSparrow(const Vector3& normal,
    const Vector3& lightDir,
    const Vector3& viewDir,
    const Vector3& surfaceColor,
    float intensity,
    float roughness,
    float reflectance) const {

    Vector3 H = normalize(Vector3(
        lightDir.x + viewDir.x,
        lightDir.y + viewDir.y,
        lightDir.z + viewDir.z
    ));

    float NdotH = std::max(0.0f, dot(normal, H));
    float NdotV = std::max(0.0f, dot(normal, viewDir));
    float NdotL = std::max(0.0f, dot(normal, lightDir));
    float VdotH = std::max(0.0f, dot(viewDir, H));

    if (NdotV <= 0.001f || NdotL <= 0.001f) {
        return lambert(normal, lightDir, surfaceColor, intensity);
    }

    // Распределение микрограней (Beckmann)
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float cosTheta = NdotH;
    float cosTheta2 = cosTheta * cosTheta;
    float cosTheta4 = cosTheta2 * cosTheta2;
    float tanTheta2 = (1.0f - cosTheta2) / std::max(cosTheta2, 1e-10f);
    float D = expf(-tanTheta2 / alpha2) / (M_PI * alpha2 * cosTheta4);

    // Геометрическое ослабление
    float tanV = sqrtf(1.0f - NdotV * NdotV) / std::max(NdotV, 1e-10f);
    float tanL = sqrtf(1.0f - NdotL * NdotL) / std::max(NdotL, 1e-10f);
    float GV = (NdotV < 0) ? 0 : 2.0f / (1.0f + sqrtf(1.0f + alpha2 * tanV * tanV));
    float GL = (NdotL < 0) ? 0 : 2.0f / (1.0f + sqrtf(1.0f + alpha2 * tanL * tanL));
    float G = GV * GL;

    // Френелевское отражение
    float F0 = reflectance;
    float F = F0 + (1.0f - F0) * powf(1.0f - VdotH, 5.0f);

    // Итоговая формула
    float denominator = 4.0f * NdotV * NdotL;
    float specular = (denominator > 1e-10f) ? (D * G * F) / denominator : 0.0f;

    // Диффузная составляющая
    Vector3 diffuseComp = lambert(normal, lightDir, surfaceColor, intensity * (1.0f - F));

    // Зеркальная составляющая
    float spec = specular * intensity;

    return Vector3(
        diffuseComp.x + spec,
        diffuseComp.y + spec,
        diffuseComp.z + spec
    );
}