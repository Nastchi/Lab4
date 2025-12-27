#pragma once

#include "config_reader.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class LightingModel {
public:
    enum Model {
        LAMBERT = 0,
        PHONG_BLINN = 1,
        TORRANCE_SPARROW = 2
    };

    Vector3 calculateColor(const Vector3& normal,
        const Vector3& lightDir,
        const Vector3& viewDir,
        const Vector3& surfaceColor,
        Model model,
        float lightIntensity,
        float shininess = 32.0f,
        float roughness = 0.3f,
        float reflectance = 0.5f) const;

private:
    Vector3 lambert(const Vector3& normal,
        const Vector3& lightDir,
        const Vector3& surfaceColor,
        float intensity) const;

    Vector3 phongBlinn(const Vector3& normal,
        const Vector3& lightDir,
        const Vector3& viewDir,
        const Vector3& surfaceColor,
        float intensity,
        float shininess) const;

    Vector3 torranceSparrow(const Vector3& normal,
        const Vector3& lightDir,
        const Vector3& viewDir,
        const Vector3& surfaceColor,
        float intensity,
        float roughness,
        float reflectance) const;
};