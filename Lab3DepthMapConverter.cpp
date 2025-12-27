#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>

#include "config_reader.h"
#include "depth_reader.h"
#include "mesh_exporter.h"
#include "opengl_visualizer.h"
#include "bmp_saver.h"

namespace fs = std::filesystem;

void createOutputDirectory(const std::string& dir) {
    if (!fs::exists(dir)) {
        if (fs::create_directories(dir)) {
            std::cout << "Создана директория: " << dir << std::endl;
        }
    }
}

int main(int argc, char** argv) {
    setlocale(LC_ALL, "Russian");

    std::cout << "========================================\n";
    std::cout << "Лабораторная работа №4\n";
    std::cout << "Формирование изображения трехмерной поверхности\n";
    std::cout << "Вариант 10: C++, модели: Ламберта, Фонга-Блинна, Торенса-Сперроу\n";
    std::cout << "========================================\n\n";

    // 1. Чтение конфигурации
    std::cout << "1. Чтение конфигурации...\n";
    Config config;
    try {
        config = ConfigReader::readConfig("resources/config.json");
        ConfigReader::printConfig(config);
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка чтения конфигурации: " << e.what() << std::endl;
        return -1;
    }

    // 2. Создание выходной директории
    createOutputDirectory(config.output_dir);

    // 3. Чтение карты глубины
    std::cout << "\n2. Чтение карты глубины...\n";
    DepthReader reader;
    if (!reader.readDepthMap(config.depth_map_file)) {
        std::cerr << "Ошибка при чтении карты глубины!" << std::endl;
        return -1;
    }

    auto depthData = reader.getDepthData();
    std::cout << "Размер карты глубины: " << reader.getWidth()
        << "x" << reader.getHeight() << std::endl;

    // 4. Сохранение карты глубины как BMP (для проверки)
    std::cout << "\n3. Сохранение карты глубины как изображения...\n";
    std::string depthBMP = config.output_dir + "/depth_map.bmp";
    if (BMPSaver::saveDepthMapAsBMP(depthData, depthBMP)) {
        std::cout << "Карта глубины сохранена: " << depthBMP << std::endl;
    }

    // 5. Экспорт в разные форматы
    std::cout << "\n4. Экспорт 3D модели...\n";

    std::vector<std::unique_ptr<MeshExporter>> exporters;

    for (const auto& format : config.output_formats) {
        if (format == "obj") {
            exporters.push_back(std::make_unique<OBJExporter>());
        }
        else if (format == "stl") {
            exporters.push_back(std::make_unique<STLExporter>());
        }
        else if (format == "ply") {
            exporters.push_back(std::make_unique<PLYExporter>());
        }
        else {
            std::cerr << "Неизвестный формат: " << format << std::endl;
        }
    }

    for (auto& exporter : exporters) {
        std::string outputFile = config.output_dir + "/model." + exporter->getFileExtension();
        std::cout << "Экспорт в " << exporter->getFormatName() << "...\n";

        if (exporter->exportMesh(depthData, outputFile, config.scale)) {
            std::cout << "  Успешно: " << outputFile << "\n";
        }
        else {
            std::cerr << "  Ошибка экспорта в " << exporter->getFormatName() << std::endl;
        }
    }

    // 6. Визуализация в OpenGL
    std::cout << "\n5. Запуск OpenGL визуализации...\n";
    std::cout << "Используется модель отражения: ";
    switch (config.reflection_model) {
    case 0: std::cout << "Ламберт\n"; break;
    case 1: std::cout << "Фонга-Блинна\n"; break;
    case 2: std::cout << "Торенса-Сперроу\n"; break;
    }

    // Инициализация OpenGL
    OpenGLVisualizer::initialize(argc, argv, depthData, config);

    // Запуск основного цикла
    OpenGLVisualizer::run();

    return 0;
}