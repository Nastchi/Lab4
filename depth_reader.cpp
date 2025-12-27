#include "depth_reader.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

DepthReader::DepthReader() : width(0), height(0) {}


bool DepthReader::readDepthMap(const std::string& filename) {
    setlocale(LC_ALL, "Russian");

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Ошибка: Не удалось открыть файл " << filename << std::endl;
        return false;
    }
    double readHeight, readWidth;
    file.read(reinterpret_cast<char*>(&readHeight), sizeof(double));
    file.read(reinterpret_cast<char*>(&readWidth), sizeof(double));

    height = static_cast<int>(std::round(readHeight));
    width = static_cast<int>(std::round(readWidth));

    

    if (height <= 0 || width <= 0) {
        std::cerr << "Некорректные размеры карты глубины: " << width << " x " << height << std::endl;
        return false;
    }

    std::vector<double> flatData(width * height);
    file.read(reinterpret_cast<char*>(flatData.data()), width * height * sizeof(double));

    if (!file) {
        std::cerr << "Ошибка чтения данных из файла" << std::endl;
        return false;
    }

    // Конвертируем в 2D массив
    depthData.resize(height, std::vector<double>(width));
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            depthData[i][j] = flatData[i * width + j];
        }
    }

    file.close();
    return true;
}

const std::vector<std::vector<double>>& DepthReader::getDepthData() const {
    return depthData;
}

int DepthReader::getWidth() const { return width; }
int DepthReader::getHeight() const { return height; }

void DepthReader::printInfo() const {
    

    if (!depthData.empty()) {
        double minDepth = depthData[0][0];
        double maxDepth = depthData[0][0];

        for (const auto& row : depthData) {
            for (double depth : row) {
                if (depth < minDepth) minDepth = depth;
                if (depth > maxDepth) maxDepth = depth;
            }
        }

        
    }
}