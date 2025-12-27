#pragma once

#include <vector>
#include <string>
#include <fstream>

class MeshExporter {
public:
    virtual ~MeshExporter() = default;

    virtual bool exportMesh(const std::vector<std::vector<double>>& depthData,
        const std::string& filename,
        float scale = 1.0f) = 0;

    virtual std::string getFormatName() const = 0;
    virtual std::string getFileExtension() const = 0;
};

class OBJExporter : public MeshExporter {
public:
    bool exportMesh(const std::vector<std::vector<double>>& depthData,
        const std::string& filename,
        float scale = 1.0f) override;

    std::string getFormatName() const override { return "Wavefront OBJ"; }
    std::string getFileExtension() const override { return "obj"; }

private:
    void writeHeader(std::ofstream& file);
    void writeVertices(std::ofstream& file,
        const std::vector<std::vector<double>>& depthData,
        float scale);
    void writeNormals(std::ofstream& file,
        const std::vector<std::vector<double>>& depthData,
        float scale);
    void writeFaces(std::ofstream& file,
        const std::vector<std::vector<double>>& depthData);
};

class STLExporter : public MeshExporter {
public:
    bool exportMesh(const std::vector<std::vector<double>>& depthData,
        const std::string& filename,
        float scale = 1.0f) override;

    std::string getFormatName() const override { return "STL (ASCII)"; }
    std::string getFileExtension() const override { return "stl"; }

private:
    struct Triangle {
        float normal[3];
        float v1[3];
        float v2[3];
        float v3[3];
    };


    void computeNormal(float& nx, float& ny, float& nz,
        const float v1[3], const float v2[3], const float v3[3]);
};

class PLYExporter : public MeshExporter {
public:
    bool exportMesh(const std::vector<std::vector<double>>& depthData,
        const std::string& filename,
        float scale = 1.0f) override;

    std::string getFormatName() const override { return "PLY (ASCII)"; }
    std::string getFileExtension() const override { return "ply"; }

private:
    void writeHeader(std::ofstream& file, int vertexCount, int faceCount);
};