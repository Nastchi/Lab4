#ifndef OPENGL_VISUALIZER_H
#define OPENGL_VISUALIZER_H

#include "config_reader.h"
#include <vector>

class OpenGLVisualizer {
public:
    static void initialize(int argc, char** argv, const std::vector<std::vector<double>>& depthData, const Config& config);
    static void run();

private:
    static Config currentConfig;
    static std::vector<std::vector<double>> depthData;

    static float rotationX, rotationY;
    static float zoom;
    static bool showAxes;
    static bool wireframeMode;

    // Параметры камеры как в Python примере
    static float cameraX, cameraY, cameraZ;
    static float lookAtX, lookAtY, lookAtZ;
    static float upX, upY, upZ;

    static int windowWidth, windowHeight;

    // Функции отрисовки
    static void display();
    static void drawAxes();
    static void drawDepthMapAsQuads();
    static void calculateQuadNormal(float x1, float y1, float z1,
        float x2, float y2, float z2,
        float x3, float y3, float z3,
        float x4, float y4, float z4,
        float& nx, float& ny, float& nz);

    // Функции управления
    static void keyboard(unsigned char key, int x, int y);
    static void specialKeys(int key, int x, int y);
    static void mouse(int button, int state, int x, int y);
    static void motion(int x, int y);
    static void reshape(int width, int height);

    // Вспомогательные функции
    static void setupLighting();
    static void setupMaterial();
    static void resetView();
    static void printControls();
};

#endif