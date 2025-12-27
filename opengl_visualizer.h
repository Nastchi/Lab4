#ifndef OPENGL_VISUALIZER_H
#define OPENGL_VISUALIZER_H

#ifdef _WIN32
#ifdef FREEGLUT_STATIC
#define FREEGLUT_LIB_PRAGMAS 1
#endif
#endif
#include "config_reader.h"
#include "reflection_models.h"
#include <vector>

class OpenGLVisualizer {
public:
    static void initialize(int argc, char** argv, const std::vector<std::vector<double>>& depthData);
    static void run();
    static void initialize(int argc, char** argv,
        const std::vector<std::vector<double>>& depthData,
        const Config& config);

private:
    static Config currentConfig;

    // Модели отражения
    static void setupLambertLighting();
    static void setupPhongBlinnLighting();
    static void setupTorranceSparrowLighting();
    static void applyReflectionModel();

    // Сохранение изображения
    static void saveScreenshot();
    static std::vector<std::vector<double>> depthData;
    static float rotationX, rotationY;
    static float zoom;
    static bool showAxes;
    static bool wireframeMode;

    // Функции отрисовки
    static void display();
    static void drawAxes();
    static void drawDepthMapMesh();
    static void drawWireframe();
    static void drawSolid();

    // Функции управления
    static void keyboard(unsigned char key, int x, int y);
    static void specialKeys(int key, int x, int y);
    static void reshape(int width, int height);
    static void setupLighting();
    static void setupMaterial();

    // Вспомогательные функции
    static void calculateNormals(std::vector<std::vector<float>>& normals);
    static void calculateTriangleNormal(int i1, int j1, int i2, int j2, int i3, int j3,
        float& nx, float& ny, float& nz);
    static void resetView();
    static void printControls();
};

#endif