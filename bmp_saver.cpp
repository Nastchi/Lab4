#include "bmp_saver.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>

// Структура BMP заголовка
#pragma pack(push, 1)
struct BMPFileHeader {
    uint16_t file_type{ 0x4D42 };          // "BM"
    uint32_t file_size{ 0 };               // Размер файла в байтах
    uint16_t reserved1{ 0 };
    uint16_t reserved2{ 0 };
    uint32_t offset_data{ 54 };           // Смещение до данных пикселей
};

struct BMPInfoHeader {
    uint32_t size{ 40 };                  // Размер этого заголовка
    int32_t width{ 0 };                   // Ширина в пикселях
    int32_t height{ 0 };                  // Высота в пикселях
    uint16_t planes{ 1 };                 // Количество плоскостей
    uint16_t bit_count{ 24 };             // Бит на пиксель (24 = RGB)
    uint32_t compression{ 0 };            // Сжатие (0 = BI_RGB)
    uint32_t size_image{ 0 };             // Размер изображения в байтах
    int32_t x_pixels_per_meter{ 0 };
    int32_t y_pixels_per_meter{ 0 };
    uint32_t colors_used{ 0 };            // Количество используемых цветов
    uint32_t colors_important{ 0 };       // Количество важных цветов
};
#pragma pack(pop)

bool BMPSaver::saveFrameBuffer(const std::string& filename,
    int width, int height) {
    // В этой реализации просто сохраняем заглушку
    std::cout << "[INFO] BMPSaver::saveFrameBuffer - заглушка\n";

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось создать файл " << filename << std::endl;
        return false;
    }

    // Простой BMP с черным цветом
    BMPFileHeader file_header;
    BMPInfoHeader info_header;

    info_header.width = width;
    info_header.height = height;
    info_header.size_image = width * height * 3;
    file_header.file_size = sizeof(file_header) + sizeof(info_header) + info_header.size_image;

    // Записываем заголовки
    file.write(reinterpret_cast<char*>(&file_header), sizeof(file_header));
    file.write(reinterpret_cast<char*>(&info_header), sizeof(info_header));

    // Записываем черные пиксели
    std::vector<uint8_t> pixels(width * height * 3, 0);
    file.write(reinterpret_cast<char*>(pixels.data()), pixels.size());

    file.close();
    std::cout << "Создан BMP файл: " << filename
        << " (" << width << "x" << height << ")" << std::endl;

    return true;
}

bool BMPSaver::saveDepthMapAsBMP(const std::vector<std::vector<double>>& depthData,
    const std::string& filename) {
    if (depthData.empty()) {
        std::cerr << "Ошибка: пустые данные глубины" << std::endl;
        return false;
    }

    int height = static_cast<int>(depthData.size());
    if (height == 0) return false;

    int width = static_cast<int>(depthData[0].size());
    if (width == 0) return false;

    std::cout << "Сохранение карты глубины как BMP: "
        << filename << " (" << width << "x" << height << ")" << std::endl;

    // Находим минимальное и максимальное значение глубины
    double minDepth = depthData[0][0];
    double maxDepth = depthData[0][0];

    for (const auto& row : depthData) {
        for (double depth : row) {
            if (depth > 0.0) { // Игнорируем фон (0 или отрицательные значения)
                if (depth < minDepth) minDepth = depth;
                if (depth > maxDepth) maxDepth = depth;
            }
        }
    }

    std::cout << "Диапазон глубины: " << minDepth << " - " << maxDepth << std::endl;

    // Создаем BMP файл
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось создать файл " << filename << std::endl;
        return false;
    }

    // Подготавливаем заголовки BMP
    BMPFileHeader file_header;
    BMPInfoHeader info_header;

    // Размеры изображения
    info_header.width = width;
    info_header.height = height;

    // Выравнивание строк по 4 байта
    int row_stride = width * 3;
    int padding = (4 - (row_stride % 4)) % 4;
    info_header.size_image = (row_stride + padding) * height;

    file_header.file_size = sizeof(file_header) + sizeof(info_header) + info_header.size_image;
    file_header.offset_data = sizeof(file_header) + sizeof(info_header);

    // Записываем заголовки
    file.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));
    file.write(reinterpret_cast<const char*>(&info_header), sizeof(info_header));

    // Записываем данные пикселей (снизу вверх, как требует BMP)
    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            double depth = depthData[y][x];

            // Нормализуем глубину в диапазон 0-255
            uint8_t intensity = 0;
            if (depth > 0.0 && maxDepth > minDepth) {
                double normalized = (depth - minDepth) / (maxDepth - minDepth);
                intensity = static_cast<uint8_t>(normalized * 255);
            }

            // BMP хранит цвета в порядке BGR
            uint8_t pixel[3] = { intensity, intensity, intensity }; // grayscale

            // Если хотим цветовое кодирование глубины:
            // uint8_t pixel[3];
            // if (depth <= 0.0) {
            //     pixel[0] = 0; pixel[1] = 0; pixel[2] = 0; // черный для фона
            // } else {
            //     // Цветовое кодирование от синего (близко) к красному (далеко)
            //     double normalized = (depth - minDepth) / (maxDepth - minDepth);
            //     if (normalized < 0.5) {
            //         pixel[0] = static_cast<uint8_t>(normalized * 2 * 255); // blue
            //         pixel[1] = 0;
            //         pixel[2] = static_cast<uint8_t>((1 - normalized * 2) * 255); // red
            //     } else {
            //         pixel[0] = 255;
            //         pixel[1] = static_cast<uint8_t>((normalized - 0.5) * 2 * 255); // green
            //         pixel[2] = 0;
            //     }
            // }

            file.write(reinterpret_cast<const char*>(pixel), 3);
        }

        // Добавляем выравнивание
        uint8_t padding_byte[3] = { 0, 0, 0 };
        file.write(reinterpret_cast<const char*>(padding_byte), padding);
    }

    file.close();

    std::cout << "Карта глубины сохранена как BMP: " << filename << std::endl;
    return true;
}

void BMPSaver::normalizeDepthData(const std::vector<std::vector<double>>& depthData,
    std::vector<uint8_t>& pixels) {
    if (depthData.empty()) return;

    int height = static_cast<int>(depthData.size());
    int width = static_cast<int>(depthData[0].size());

    // Находим мин/макс глубину (игнорируя фон)
    double minVal = std::numeric_limits<double>::max();
    double maxVal = std::numeric_limits<double>::lowest();

    for (const auto& row : depthData) {
        for (double val : row) {
            if (val > 0.0) { // игнорируем фон
                if (val < minVal) minVal = val;
                if (val > maxVal) maxVal = val;
            }
        }
    }

    // Если все значения фоновые
    if (minVal > maxVal) {
        minVal = 0.0;
        maxVal = 1.0;
    }

    // Нормализуем и конвертируем в байты
    pixels.resize(width * height);

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            double val = depthData[i][j];
            if (val <= 0.0) {
                pixels[i * width + j] = 0; // черный для фона
            }
            else {
                double normalized = (val - minVal) / (maxVal - minVal);
                pixels[i * width + j] = static_cast<uint8_t>(normalized * 255);
            }
        }
    }
}