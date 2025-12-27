#pragma once

#include <string>
#include <vector>

struct Vector3 {
    float x, y, z;

    Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}


    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }

    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }

    Vector3 operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }
};

struct Config {
    // Входные данные
    std::string depth_map_file;

    // Выходные форматы
    std::vector<std::string> output_formats;
    std::string output_dir;

    // Параметры освещения
    Vector3 light_direction;
    float light_intensity;
    Vector3 light_color;

    // Параметры камеры
    Vector3 camera_position;
    Vector3 camera_target;
    Vector3 camera_up;
    float fov;
    std::string projection_type; 

    // Модель отражения
    int reflection_model; // 0=Ламберт, 1=Фонга-Блинна, 2=Торенса-Сперроу
    float material_shininess;
    Vector3 material_color;

  
    std::string image_output;
    int image_width;
    int image_height;

    float scale;
    bool show_axes;
    bool wireframe_mode;
};

class ConfigReader {
public:
    static Config readConfig(const std::string& filename);
    static void printConfig(const Config& config);

private:
    static Vector3 parseVector(const std::string& str);
};
