#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

#include "config_reader.h"
#include "depth_reader.h"
#include "mesh_exporter.h"
#include "opengl_visualizer.h"
#include "bmp_saver.h"

namespace fs = std::filesystem;
namespace fs = std::filesystem;

void createOutputDirectory(const std::string& dir) {
    if (!fs::exists(dir)) {
        if (fs::create_directories(dir)) {
            std::cout << "Создана директория: " << dir << std::endl;
        }
    }
}

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    setlocale(LC_ALL, "Russian");

    std::cout << "========================================" << std::endl;
    std::cout << "Лабораторная работа №4" << std::endl;
    std::cout << "Формирование изображения трехмерной поверхности" << std::endl;
    std::cout << "Вариант 10: C++, модели: Ламберта, Фонга-Блинна, Торренса-Сперроу" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // 1. Проверка аргументов командной строки
    if (argc < 2) {
        std::cerr << "Ошибка: Не указан JSON файл конфигурации" << std::endl;
        std::cerr << "Использование: " << argv[0] << " <config.json>" << std::endl;
        std::cerr << "Пример: " << argv[0] << " config.json" << std::endl;
        std::cerr << "Пример: " << argv[0] << " config_lambert.json" << std::endl;
        std::cerr << "Пример: " << argv[0] << " config_phong.json" << std::endl;
        return -1;
    }

    // 2. Чтение конфигурации из JSON файла
    std::cout << "1. Чтение конфигурации из файла: " << argv[1] << std::endl;
    Config config;
    try {
        config = ConfigReader::readConfig(argv[1]); 
        ConfigReader::printConfig(config);  
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка чтения конфигурации: " << e.what() << std::endl;
        return -1;
    }

    // 3. Создание выходной директории
    createOutputDirectory(config.output_dir);

    // 4. Чтение карты глубины
    std::cout << "\n2. Чтение карты глубины..." << std::endl;
    DepthReader reader;
    if (!reader.readDepthMap(config.depth_map_file)) {
        std::cerr << "Ошибка при чтении карты глубины!" << std::endl;
        return -1;
    }

    auto depthData = reader.getDepthData();
    std::cout << "Размер карты глубины: " << reader.getWidth()
        << "x" << reader.getHeight() << std::endl;

    // 5. Сохранение карты глубины как BMP
    std::cout << "\n3. Сохранение карты глубины как изображения..." << std::endl;
    std::string depthBMP = config.output_dir + "/depth_map.bmp";
    if (BMPSaver::saveDepthMapAsBMP(depthData, depthBMP)) {
        std::cout << "Карта глубины сохранена: " << depthBMP << std::endl;
    }

    // 6. Экспорт в разные форматы
    std::cout << "\n4. Экспорт 3D модели..." << std::endl;

    for (const auto& format : config.output_formats) {
        std::unique_ptr<MeshExporter> exporter;

        if (format == "obj" || format == "OBJ") {
            exporter = std::make_unique<OBJExporter>();
        }
        else if (format == "stl" || format == "STL") {
            exporter = std::make_unique<STLExporter>();
        }
        else if (format == "ply" || format == "PLY") {
            exporter = std::make_unique<PLYExporter>();
        }
        else {
            std::cerr << "Неизвестный формат: " << format << std::endl;
            continue;
        }

        std::string outputFile = config.output_dir + "/model." + exporter->getFileExtension();
        std::cout << "Экспорт в " << exporter->getFormatName() << "..." << std::endl;

        if (exporter->exportMesh(depthData, outputFile, config.scale)) {
            std::cout << "  Успешно: " << outputFile << std::endl;
        }
        else {
            std::cerr << "  Ошибка экспорта в " << exporter->getFormatName() << std::endl;
        }
    }

    // 7. Визуализация в OpenGL
    std::cout << "\n5. Запуск OpenGL визуализации..." << std::endl;
    std::cout << "Используется модель отражения: ";
    switch (config.reflection_model) {
    case 0: std::cout << "Ламберт" << std::endl; break;
    case 1: std::cout << "Фонга-Блинн" << std::endl; break;
    case 2: std::cout << "Торренса-Сперроу" << std::endl; break;
    }

    // Инициализация OpenGL
    OpenGLVisualizer::initialize(argc, argv, depthData, config);

    // Запуск основного цикла
    OpenGLVisualizer::run();

    return 0;
}