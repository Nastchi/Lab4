#pragma once
#ifndef OBJ_WRITER_H
#define OBJ_WRITER_H

#include <vector>
#include <string>

class OBJWriter {
public:
    OBJWriter();
    bool writeToOBJ(const std::string& filename,
        const std::vector<std::vector<double>>& depthData,
        double scale = 1.0);
    void setGenerateGround(bool generate);

private:
    bool generateGround;
    void writeHeader(std::ofstream& file);
    void writeVertices(std::ofstream& file,
        const std::vector<std::vector<double>>& depthData,
        double scale);
    void writeFaces(std::ofstream& file,
        const std::vector<std::vector<double>>& depthData);
};

#endif