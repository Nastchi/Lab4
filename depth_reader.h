#pragma once

#include <vector>
#include <string>

class DepthReader {
public:
    DepthReader();
    bool readDepthMap(const std::string& filename);
    const std::vector<std::vector<double>>& getDepthData() const;
    int getWidth() const;
    int getHeight() const;
    void printInfo() const;
    void normalizeData();
    std::vector<std::vector<double>> getNormalizedData() const;

    double getMinDepth() const { return minDepth; }
    double getMaxDepth() const { return maxDepth; }
    double getAverageDepth() const;

private:
    std::vector<std::vector<double>> depthData;
    double minDepth, maxDepth;
    int width, height;
};