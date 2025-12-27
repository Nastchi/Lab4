#include "config_reader.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cmath>

Config ConfigReader::readConfig(const std::string& filename) {
    Config config;
    // Установим значения по умолчанию
    config.depth_map_file = "DepthMap_10.dat";
    config.output_formats = { "obj", "stl", "ply" };
    config.output_dir = "output";
    config.light_direction = Vector3(1.0f, 1.0f, 1.0f);
    config.light_intensity = 1.0f;
    config.light_color = Vector3(1.0f, 1.0f, 1.0f);
    config.camera_position = Vector3(0.0f, 0.0f, 5.0f);
    config.camera_target = Vector3(0.0f, 0.0f, 0.0f);
    config.camera_up = Vector3(0.0f, 1.0f, 0.0f);
    config.fov = 45.0f;
    config.projection_type = "perspective";
    config.reflection_model = 1; 
    config.material_shininess = 32.0f;
    config.material_color = Vector3(0.7f, 0.7f, 0.8f);
    config.image_output = "output/rendered_image.bmp";
    config.image_width = 1024;
    config.image_height = 768;
    config.scale = 1.0f;
    config.show_axes = true;
    config.wireframe_mode = false;

    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Предупреждение: Не удалось открыть файл конфигурации: "
            << filename << ". Использую значения по умолчанию." << std::endl;
        return config; 
    }

    std::string line;
    while (std::getline(file, line)) {

        size_t comment_pos = line.find("//");
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }

        if (line.empty()) continue;

        
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);


        size_t separator_pos = line.find(":");
        if (separator_pos == std::string::npos) {
            separator_pos = line.find("=");
        }
        if (separator_pos == std::string::npos) {
            continue; // Пропускаем строки без разделителя
        }

        std::string key = line.substr(0, separator_pos);
        std::string value = line.substr(separator_pos + 1);

        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);

        if (key == "depth_map_file") {
            config.depth_map_file = value;
        }
        else if (key == "output_formats") {
            config.output_formats.clear();
            std::stringstream ss(value);
            std::string format;
            while (std::getline(ss, format, ',')) {

                format.erase(0, format.find_first_not_of(" \t"));
                format.erase(format.find_last_not_of(" \t") + 1);
                if (!format.empty()) {
                    config.output_formats.push_back(format);
                }
            }
        }
        else if (key == "output_dir") {
            config.output_dir = value;
        }
        else if (key == "light_direction") {
            config.light_direction = parseVector(value);
            float len = sqrt(config.light_direction.x * config.light_direction.x +
                config.light_direction.y * config.light_direction.y +
                config.light_direction.z * config.light_direction.z);
            if (len > 0.0001f) {
                config.light_direction.x /= len;
                config.light_direction.y /= len;
                config.light_direction.z /= len;
            }
        }
        else if (key == "light_intensity") {
            try {
                config.light_intensity = std::stof(value);
            }
            catch (...) {
                std::cerr << "Ошибка парсинга light_intensity, использую значение по умолчанию" << std::endl;
            }
        }
        else if (key == "light_color") {
            config.light_color = parseVector(value);
        }
        else if (key == "camera_position") {
            config.camera_position = parseVector(value);
        }
        else if (key == "camera_target") {
            config.camera_target = parseVector(value);
        }
        else if (key == "camera_up") {
            config.camera_up = parseVector(value);
        }
        else if (key == "fov") {
            try {
                config.fov = std::stof(value);
            }
            catch (...) {
                std::cerr << "Ошибка парсинга fov, использую значение по умолчанию" << std::endl;
            }
        }
        else if (key == "projection_type") {
            config.projection_type = value;
        }
        else if (key == "reflection_model") {
            try {
                config.reflection_model = std::stoi(value);
            }
            catch (...) {
                std::cerr << "Ошибка парсинга reflection_model, использую значение по умолчанию" << std::endl;
            }
        }
        else if (key == "material_shininess") {
            try {
                config.material_shininess = std::stof(value);
            }
            catch (...) {
                std::cerr << "Ошибка парсинга material_shininess, использую значение по умолчанию" << std::endl;
            }
        }
        else if (key == "material_color") {
            config.material_color = parseVector(value);
        }
        else if (key == "image_output") {
            config.image_output = value;
        }
        else if (key == "image_width") {
            try {
                config.image_width = std::stoi(value);
            }
            catch (...) {
                std::cerr << "Ошибка парсинга image_width, использую значение по умолчанию" << std::endl;
            }
        }
        else if (key == "image_height") {
            try {
                config.image_height = std::stoi(value);
            }
            catch (...) {
                std::cerr << "Ошибка парсинга image_height, использую значение по умолчанию" << std::endl;
            }
        }
        else if (key == "scale") {
            try {
                config.scale = std::stof(value);
            }
            catch (...) {
                std::cerr << "Ошибка парсинга scale, использую значение по умолчанию" << std::endl;
            }
        }
        else if (key == "show_axes") {
            config.show_axes = (value == "true" || value == "1" || value == "yes");
        }
        else if (key == "wireframe_mode") {
            config.wireframe_mode = (value == "true" || value == "1" || value == "yes");
        }
    }

    file.close();
    return config;
}

Vector3 ConfigReader::parseVector(const std::string& str) {
    Vector3 result(0.0f, 0.0f, 0.0f);

    if (str.empty()) {
        return Vector3(1.0f, 1.0f, 1.0f); 
    }

    std::string s = str;
    if (s.front() == '(' || s.front() == '[' || s.front() == '{') {
        s = s.substr(1);
    }
    if (s.back() == ')' || s.back() == ']' || s.back() == '}') {
        s.pop_back();
    }

    std::stringstream ss(s);
    char delimiter;
    if (ss >> result.x) {
        if (ss >> delimiter >> result.y >> delimiter >> result.z) {
            return result;
        }
    }

    // Формат 2: x,y,z 
    size_t pos1 = s.find(',');
    if (pos1 != std::string::npos) {
        size_t pos2 = s.find(',', pos1 + 1);
        if (pos2 != std::string::npos) {
            try {
                result.x = std::stof(s.substr(0, pos1));
                result.y = std::stof(s.substr(pos1 + 1, pos2 - pos1 - 1));
                result.z = std::stof(s.substr(pos2 + 1));
                return result;
            }
            catch (...) {
                // Ошибка парсинга
            }
        }
    }

    // Формат 3: x y z 
    std::stringstream ss2(s);
    if (ss2 >> result.x >> result.y >> result.z) {
        return result;
    }

    // Если ничего не получилось, возвращаем значение по умолчанию
    return Vector3(1.0f, 1.0f, 1.0f);
}

void ConfigReader::printConfig(const Config& config) {
    std::cout << "\n=== Конфигурация программы ===\n";
    std::cout << "Файл карты глубины: " << config.depth_map_file << "\n";
    std::cout << "Выходные форматы: ";

    if (config.output_formats.empty()) {
        std::cout << "нет";
    }
    else {
        for (size_t i = 0; i < config.output_formats.size(); ++i) {
            std::cout << config.output_formats[i];
            if (i < config.output_formats.size() - 1) std::cout << ", ";
        }
    }
    std::cout << "\n";

    std::cout << "Директория выходных файлов: " << config.output_dir << "\n";
    std::cout << "Направление света: (" << config.light_direction.x << ", "
        << config.light_direction.y << ", " << config.light_direction.z << ")\n";
    std::cout << "Интенсивность света: " << config.light_intensity << "\n";
    std::cout << "Цвет света: (" << config.light_color.x << ", "
        << config.light_color.y << ", " << config.light_color.z << ")\n";
    std::cout << "Позиция камеры: (" << config.camera_position.x << ", "
        << config.camera_position.y << ", " << config.camera_position.z << ")\n";
    std::cout << "Цель камеры: (" << config.camera_target.x << ", "
        << config.camera_target.y << ", " << config.camera_target.z << ")\n";
    std::cout << "Вектор вверх камеры: (" << config.camera_up.x << ", "
        << config.camera_up.y << ", " << config.camera_up.z << ")\n";
    std::cout << "Поле зрения (FOV): " << config.fov << "\n";
    std::cout << "Тип проекции: " << config.projection_type << "\n";

    std::cout << "Модель отражения: ";
    switch (config.reflection_model) {
    case 0: std::cout << "Ламберт\n"; break;
    case 1: std::cout << "Фонга-Блинна\n"; break;
    case 2: std::cout << "Торенса-Сперроу\n"; break;
    default: std::cout << "Неизвестно (" << config.reflection_model << ")\n"; break;
    }

    std::cout << "Блеск материала: " << config.material_shininess << "\n";
    std::cout << "Цвет материала: (" << config.material_color.x << ", "
        << config.material_color.y << ", " << config.material_color.z << ")\n";
    std::cout << "Выходное изображение: " << config.image_output << "\n";
    std::cout << "Размер изображения: " << config.image_width << "x" << config.image_height << "\n";
    std::cout << "Масштаб: " << config.scale << "\n";
    std::cout << "Показать оси: " << (config.show_axes ? "да" : "нет") << "\n";
    std::cout << "Режим каркаса: " << (config.wireframe_mode ? "да" : "нет") << "\n";
    std::cout << "================================\n\n";
}