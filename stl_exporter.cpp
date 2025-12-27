#include "mesh_exporter.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <cstddef>  

bool STLExporter::exportMesh(const std::vector<std::vector<double>>& depthData,
    const std::string& filename,
    float scale) {
    if (depthData.empty()) {
        std::cerr << "Ошибка: Пустые данные глубины" << std::endl;
        return false;
    }

    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка: Не удалось создать файл " << filename << std::endl;
        return false;
    }

    int height = static_cast<int>(depthData.size());
    if (height < 2) return false;

    int width = static_cast<int>(depthData[0].size());
    if (width < 2) return false;

    std::vector<Triangle> triangles;

    // Собираем треугольники
    for (int i = 0; i < height - 1; i++) {
        for (int j = 0; j < width - 1; j++) {
            // Пропускаем фон
            if (depthData[i][j] <= 0.0 || depthData[i][j + 1] <= 0.0 ||
                depthData[i + 1][j] <= 0.0 || depthData[i + 1][j + 1] <= 0.0) {
                continue;
            }

            Triangle tri1, tri2;

            // ИСПРАВЛЕНО: явное приведение double к float
            float x1 = static_cast<float>((j - width / 2.0) * scale);
            float y1 = static_cast<float>(depthData[i][j] * scale);
            float z1 = static_cast<float>((i - height / 2.0) * scale);

            float x2 = static_cast<float>((j + 1 - width / 2.0) * scale);
            float y2 = static_cast<float>(depthData[i][j + 1] * scale);
            float z2 = static_cast<float>((i - height / 2.0) * scale);

            float x3 = static_cast<float>((j - width / 2.0) * scale);
            float y3 = static_cast<float>(depthData[i + 1][j] * scale);
            float z3 = static_cast<float>((i + 1 - height / 2.0) * scale);

            float x4 = static_cast<float>((j + 1 - width / 2.0) * scale);
            float y4 = static_cast<float>(depthData[i + 1][j + 1] * scale);
            float z4 = static_cast<float>((i + 1 - height / 2.0) * scale);

            // Первый треугольник
            tri1.v1[0] = x1; tri1.v1[1] = y1; tri1.v1[2] = z1;
            tri1.v2[0] = x2; tri1.v2[1] = y2; tri1.v2[2] = z2;
            tri1.v3[0] = x3; tri1.v3[1] = y3; tri1.v3[2] = z3;

            computeNormal(tri1.normal[0], tri1.normal[1], tri1.normal[2],
                tri1.v1, tri1.v2, tri1.v3);

            // Второй треугольник
            tri2.v1[0] = x2; tri2.v1[1] = y2; tri2.v1[2] = z2;
            tri2.v2[0] = x4; tri2.v2[1] = y4; tri2.v2[2] = z4;
            tri2.v3[0] = x3; tri2.v3[1] = y3; tri2.v3[2] = z3;

            computeNormal(tri2.normal[0], tri2.normal[1], tri2.normal[2],
                tri2.v1, tri2.v2, tri2.v3);

            triangles.push_back(tri1);
            triangles.push_back(tri2);
        }
    }

    // Запись STL файла
    file << "solid 3D_Model\n";

    for (const auto& tri : triangles) {
        file << "facet normal "
            << tri.normal[0] << " "
            << tri.normal[1] << " "
            << tri.normal[2] << "\n";
        file << "  outer loop\n";
        file << "    vertex "
            << tri.v1[0] << " " << tri.v1[1] << " " << tri.v1[2] << "\n";
        file << "    vertex "
            << tri.v2[0] << " " << tri.v2[1] << " " << tri.v2[2] << "\n";
        file << "    vertex "
            << tri.v3[0] << " " << tri.v3[1] << " " << tri.v3[2] << "\n";
        file << "  endloop\n";
        file << "endfacet\n";
    }

    file << "endsolid 3D_Model\n";
    file.close();

    std::cout << "STL файл сохранен: " << filename
        << " (треугольников: " << triangles.size() << ")" << std::endl;
    return true;
}

void STLExporter::computeNormal(float& nx, float& ny, float& nz,
    const float v1[3], const float v2[3], const float v3[3]) {
    // Вектор 1: v2 - v1
    float ux = v2[0] - v1[0];
    float uy = v2[1] - v1[1];
    float uz = v2[2] - v1[2];

    // Вектор 2: v3 - v1
    float vx = v3[0] - v1[0];
    float vy = v3[1] - v1[1];
    float vz = v3[2] - v1[2];

    // Векторное произведение (нормаль)
    nx = uy * vz - uz * vy;
    ny = uz * vx - ux * vz;
    nz = ux * vy - uy * vx;

    // Нормализация
    float length = sqrtf(nx * nx + ny * ny + nz * nz);
    if (length > 0.0001f) {
        nx /= length;
        ny /= length;
        nz /= length;
    }
    else {
        nx = 0.0f;
        ny = 1.0f;
        nz = 0.0f;
    }
}