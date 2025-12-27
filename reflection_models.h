#pragma once
#include "config_reader.h" 

class ReflectionModels {
public:
    // Модель Ламберта 
    static Vector3 lambert(const Vector3& normal,
        const Vector3& lightDir,
        const Vector3& lightColor,
        float intensity);

    // Модель Фонга-Блинна
    static Vector3 phongBlinn(const Vector3& normal,
        const Vector3& lightDir,
        const Vector3& viewDir,
        const Vector3& lightColor,
        float intensity,
        float shininess);

    //  модель Торенса-Сперроу
    static Vector3 torranceSparrow(const Vector3& normal,
        const Vector3& lightDir,
        const Vector3& viewDir,
        const Vector3& lightColor,
        float intensity,
        float roughness = 0.3f,
        float reflectance = 0.5f);

    // Вспомогательные функции
    static Vector3 computeNormal(const Vector3& v1,
        const Vector3& v2,
        const Vector3& v3);

    static float clamp(float value, float min = 0.0f, float max = 1.0f);
};