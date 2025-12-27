#ifdef _WIN32
#include <windows.h>
#endif

#include "opengl_visualizer.h"
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <locale>
#include <fstream>
#include <string>
#include <cstdint>

using namespace std;

// Инициализация статических переменных
Config OpenGLVisualizer::currentConfig;
vector<vector<double>> OpenGLVisualizer::depthData;
float OpenGLVisualizer::rotationX = 0.0f;
float OpenGLVisualizer::rotationY = 0.0f;
float OpenGLVisualizer::zoom = 1.0f;
bool OpenGLVisualizer::showAxes = true;
bool OpenGLVisualizer::wireframeMode = false;

void OpenGLVisualizer::display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(0, 3, 2 * zoom,
        0, 0, 0,
        0, 2, 0);

    glRotatef(rotationX, 1, 0, 0);
    glRotatef(rotationY, 0, 1, 0);
    glScalef(0.1f, 0.1f, 0.1f);

    if (showAxes) {
        drawAxes();
    }

    if (!depthData.empty()) {
        drawDepthMapMesh();
    }

    glutSwapBuffers();
}

void OpenGLVisualizer::drawAxes() {
    glDisable(GL_LIGHTING);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glColor3f(1, 0, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(2, 0, 0);
    glColor3f(0, 1, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 2, 0);
    glColor3f(0, 0, 1);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 2);
    glEnd();

    glColor3f(1, 1, 1);
    glRasterPos3f(2.2f, 0, 0);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'X');
    glRasterPos3f(0, 2.2f, 0);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'Y');
    glRasterPos3f(0, 0, 2.2f);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'Z');

    glEnable(GL_LIGHTING);
}

void OpenGLVisualizer::drawDepthMapMesh() {
    if (wireframeMode) {
        drawWireframe();
    }
    else {
        drawSolid();
    }
}

void OpenGLVisualizer::drawSolid() {
    if (depthData.empty() || depthData[0].empty()) {
        return;
    }

    int height = static_cast<int>(depthData.size());
    int width = static_cast<int>(depthData[0].size());

    if (height < 2 || width < 2) {
        return;
    }

    vector<vector<float>> normals(height * width, vector<float>(3, 0.0f));
    calculateNormals(normals);

    // Используем конфигурацию для цвета материала
    GLfloat materialColor[] = {
        currentConfig.material_color.x,
        currentConfig.material_color.y,
        currentConfig.material_color.z,
        1.0f
    };

    GLfloat materialSpecular[] = { 0.3f, 0.3f, 0.3f, 1.0f };

    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialColor);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    GLfloat shininess[] = { currentConfig.phong_shininess };
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

    // Рисуем треугольники
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < height - 1; i++) {
        for (int j = 0; j < width - 1; j++) {
            // Пропускаем треугольники, содержащие точки с нулевой глубиной (фон)
            if (depthData[i][j] <= 0.0 || depthData[i][j + 1] <= 0.0 ||
                depthData[i + 1][j] <= 0.0 || depthData[i + 1][j + 1] <= 0.0) {
                continue;
            }

            // Проверяем границы для нормалей
            int idx1 = i * width + j;
            int idx2 = i * width + j + 1;
            int idx3 = (i + 1) * width + j;
            int idx4 = (i + 1) * width + j + 1;

            if (idx1 >= normals.size() || idx2 >= normals.size() ||
                idx3 >= normals.size() || idx4 >= normals.size()) {
                continue;
            }

            // Координаты вершин
            double x1 = (j - width / 2.0) * 0.1 * currentConfig.scale;
            double z1 = (i - height / 2.0) * 0.1 * currentConfig.scale;
            double y1 = depthData[i][j] * 0.05 * currentConfig.scale;

            double x2 = (j + 1 - width / 2.0) * 0.1 * currentConfig.scale;
            double z2 = (i - height / 2.0) * 0.1 * currentConfig.scale;
            double y2 = depthData[i][j + 1] * 0.05 * currentConfig.scale;

            double x3 = (j - width / 2.0) * 0.1 * currentConfig.scale;
            double z3 = (i + 1 - height / 2.0) * 0.1 * currentConfig.scale;
            double y3 = depthData[i + 1][j] * 0.05 * currentConfig.scale;

            double x4 = (j + 1 - width / 2.0) * 0.1 * currentConfig.scale;
            double z4 = (i + 1 - height / 2.0) * 0.1 * currentConfig.scale;
            double y4 = depthData[i + 1][j + 1] * 0.05 * currentConfig.scale;

            // Первый треугольник
            glNormal3f(normals[idx1][0], normals[idx1][1], normals[idx1][2]);
            glVertex3f(x1, y1, z1);

            glNormal3f(normals[idx2][0], normals[idx2][1], normals[idx2][2]);
            glVertex3f(x2, y2, z2);

            glNormal3f(normals[idx3][0], normals[idx3][1], normals[idx3][2]);
            glVertex3f(x3, y3, z3);

            // Второй треугольник
            glNormal3f(normals[idx2][0], normals[idx2][1], normals[idx2][2]);
            glVertex3f(x2, y2, z2);

            glNormal3f(normals[idx4][0], normals[idx4][1], normals[idx4][2]);
            glVertex3f(x4, y4, z4);

            glNormal3f(normals[idx3][0], normals[idx3][1], normals[idx3][2]);
            glVertex3f(x3, y3, z3);
        }
    }
    glEnd();
}

void OpenGLVisualizer::drawWireframe() {
    if (depthData.empty() || depthData[0].empty()) {
        return;
    }

    int height = depthData.size();
    int width = depthData[0].size();

    glDisable(GL_LIGHTING);
    glColor3f(0.3f, 0.6f, 1.0f);
    glLineWidth(1.0f);

    glBegin(GL_LINES);
    for (int i = 0; i < height - 1; i++) {
        for (int j = 0; j < width - 1; j++) {
            // Пропускаем линии, содержащие точки с нулевой глубиной
            if (depthData[i][j] <= 0.0 || depthData[i][j + 1] <= 0.0) {
                continue;
            }

            double x1 = (j - width / 2.0) * 0.1 * currentConfig.scale;
            double z1 = (i - height / 2.0) * 0.1 * currentConfig.scale;
            double y1 = depthData[i][j] * 0.05 * currentConfig.scale;

            double x2 = (j + 1 - width / 2.0) * 0.1 * currentConfig.scale;
            double z2 = (i - height / 2.0) * 0.1 * currentConfig.scale;
            double y2 = depthData[i][j + 1] * 0.05 * currentConfig.scale;

            glVertex3f(x1, y1, z1);
            glVertex3f(x2, y2, z2);

            // Пропускаем вертикальные линии с нулевой глубиной
            if (depthData[i][j] <= 0.0 || depthData[i + 1][j] <= 0.0) {
                continue;
            }

            double x3 = (j - width / 2.0) * 0.1 * currentConfig.scale;
            double z3 = (i + 1 - height / 2.0) * 0.1 * currentConfig.scale;
            double y3 = depthData[i + 1][j] * 0.05 * currentConfig.scale;

            glVertex3f(x1, y1, z1);
            glVertex3f(x3, y3, z3);
        }
    }
    glEnd();

    glEnable(GL_LIGHTING);
}

void OpenGLVisualizer::calculateNormals(vector<vector<float>>& normals) {
    if (depthData.empty() || depthData[0].empty()) {
        return;
    }

    int height = depthData.size();
    int width = depthData[0].size();

    for (int i = 0; i < height * width; i++) {
        normals[i] = { 0.0f, 1.0f, 0.0f };
    }

    for (int i = 0; i < height - 1; i++) {
        for (int j = 0; j < width - 1; j++) {
            if (depthData[i][j] <= 0.0 && depthData[i][j + 1] <= 0.0 &&
                depthData[i + 1][j] <= 0.0 && depthData[i + 1][j + 1] <= 0.0) {
                continue;
            }

            // Рассчитываем нормали для каждого треугольника отдельно
            vector<vector<float>> triangleNormals;

            // Треугольник 1: (i,j) - (i,j+1) - (i+1,j)
            if (depthData[i][j] > 0.0 && depthData[i][j + 1] > 0.0 && depthData[i + 1][j] > 0.0) {
                float nx, ny, nz;
                calculateTriangleNormal(i, j, i, j + 1, i + 1, j, nx, ny, nz);
                triangleNormals.push_back({ nx, ny, nz });
            }

            // Треугольник 2: (i,j+1) - (i+1,j+1) - (i+1,j)
            if (depthData[i][j + 1] > 0.0 && depthData[i + 1][j + 1] > 0.0 && depthData[i + 1][j] > 0.0) {
                float nx, ny, nz;
                calculateTriangleNormal(i, j + 1, i + 1, j + 1, i + 1, j, nx, ny, nz);
                triangleNormals.push_back({ nx, ny, nz });
            }

            // Накопление нормалей для вершин
            for (const auto& normal : triangleNormals) {
                int idx1 = i * width + j;
                int idx2 = i * width + j + 1;
                int idx3 = (i + 1) * width + j;
                int idx4 = (i + 1) * width + j + 1;

                if (depthData[i][j] > 0.0 && idx1 < normals.size()) {
                    normals[idx1][0] += normal[0]; normals[idx1][1] += normal[1]; normals[idx1][2] += normal[2];
                }
                if (depthData[i][j + 1] > 0.0 && idx2 < normals.size()) {
                    normals[idx2][0] += normal[0]; normals[idx2][1] += normal[1]; normals[idx2][2] += normal[2];
                }
                if (depthData[i + 1][j] > 0.0 && idx3 < normals.size()) {
                    normals[idx3][0] += normal[0]; normals[idx3][1] += normal[1]; normals[idx3][2] += normal[2];
                }
                if (depthData[i + 1][j + 1] > 0.0 && idx4 < normals.size()) {
                    normals[idx4][0] += normal[0]; normals[idx4][1] += normal[1]; normals[idx4][2] += normal[2];
                }
            }
        }
    }

    // Нормализация накопленных нормалей
    for (int i = 0; i < height * width; i++) {
        if (depthData[i / width][i % width] > 0.0) {
            float length = sqrt(normals[i][0] * normals[i][0] +
                normals[i][1] * normals[i][1] +
                normals[i][2] * normals[i][2]);

            if (length > 0.0001f) {
                normals[i][0] /= length;
                normals[i][1] /= length;
                normals[i][2] /= length;
            }
            else {
                normals[i] = { 0.0f, 1.0f, 0.0f };
            }
        }
    }
}

void OpenGLVisualizer::calculateTriangleNormal(int i1, int j1, int i2, int j2, int i3, int j3,
    float& nx, float& ny, float& nz) {

    double x1 = (j1 - depthData[0].size() / 2.0) * 0.1;
    double y1 = depthData[i1][j1] * 0.05;
    double z1 = (i1 - depthData.size() / 2.0) * 0.1;

    double x2 = (j2 - depthData[0].size() / 2.0) * 0.1;
    double y2 = depthData[i2][j2] * 0.05;
    double z2 = (i2 - depthData.size() / 2.0) * 0.1;

    double x3 = (j3 - depthData[0].size() / 2.0) * 0.1;
    double y3 = depthData[i3][j3] * 0.05;
    double z3 = (i3 - depthData.size() / 2.0) * 0.1;

    float v1x = static_cast<float>(x2 - x1);
    float v1y = static_cast<float>(y2 - y1);
    float v1z = static_cast<float>(z2 - z1);

    float v2x = static_cast<float>(x3 - x1);
    float v2y = static_cast<float>(y3 - y1);
    float v2z = static_cast<float>(z3 - z1);

    nx = v1y * v2z - v1z * v2y;
    ny = v1z * v2x - v1x * v2z;
    nz = v1x * v2y - v1y * v2x;

    float length = sqrt(nx * nx + ny * ny + nz * nz);
    if (length > 0.0001f) {
        nx /= length;
        ny /= length;
        nz /= length;
    }
}

void OpenGLVisualizer::keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'w': case 'W': rotationX += 5.0f; break;
    case 's': case 'S': rotationX -= 5.0f; break;
    case 'a': case 'A': rotationY += 5.0f; break;
    case 'd': case 'D': rotationY -= 5.0f; break;
    case 'r': case 'R': resetView(); break;
    case 27: exit(0); break;
    }
    glutPostRedisplay();
}

void OpenGLVisualizer::specialKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP: rotationX += 5.0f; break;
    case GLUT_KEY_DOWN: rotationX -= 5.0f; break;
    case GLUT_KEY_LEFT: rotationY += 5.0f; break;
    case GLUT_KEY_RIGHT: rotationY -= 5.0f; break;
    case GLUT_KEY_HOME: resetView(); break;
    case GLUT_KEY_PAGE_UP: zoom *= 0.9f; break;
    case GLUT_KEY_PAGE_DOWN: zoom *= 1.1f; break;
    }
    glutPostRedisplay();
}

void OpenGLVisualizer::reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / height, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void OpenGLVisualizer::setupLighting() {
    GLfloat light0_position[] = { 3.0f, 10.0f, 3.0f, 1.0f };
    GLfloat light0_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat light0_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light0_specular[] = { 0.8f, 0.8f, 0.8f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);

    GLfloat light1_position[] = { -3.0f, 5.0f, -3.0f, 1.0f };
    GLfloat light1_diffuse[] = { 0.6f, 0.6f, 0.6f, 1.0f };
    GLfloat light1_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };

    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_NORMALIZE);
}

void OpenGLVisualizer::setupMaterial() {
    GLfloat material_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat material_diffuse[] = {
        currentConfig.material_color.x,
        currentConfig.material_color.y,
        currentConfig.material_color.z,
        1.0f
    };
    GLfloat material_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat material_shininess[] = { currentConfig.material_shininess };

    glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, material_shininess);
}

void OpenGLVisualizer::setupLightingForModel(int reflectionModel) {
    // Настройка освещения в зависимости от модели отражения
    switch (reflectionModel) {
    case 0: // Ламберт
    {
        GLfloat light_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
        glMaterialf(GL_FRONT, GL_SHININESS, 0.0f);
    }
    break;

    case 1: // Фонга-Блинна
    {
        GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
        glMaterialf(GL_FRONT, GL_SHININESS, currentConfig.material_shininess);
    }
    break;

    case 2: // Торренса-Сперроу
    {
        GLfloat light_specular[] = { 0.8f, 0.8f, 0.8f, 1.0f };
        glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
        glMaterialf(GL_FRONT, GL_SHININESS, currentConfig.material_shininess * 0.5f);
    }
    break;
    }
}

void OpenGLVisualizer::resetView() {
    rotationX = 0.0f;
    rotationY = 0.0f;
    zoom = 1.0f;
    showAxes = true;
    wireframeMode = false;
}

void OpenGLVisualizer::printControls() {
    setlocale(LC_ALL, "Russian");

    cout << "\n=== Управление в OpenGL ===" << endl;
    cout << "Стрелки Вверх/Вниз - вращение по X" << endl;
    cout << "Стрелки Влево/Вправо - вращение по Y" << endl;
    cout << "W/S/A/D - вращение модели" << endl;
    cout << "R - сброс вида" << endl;
    cout << "ESC - выход" << endl;
    cout << "===========================\n" << endl;
}

void OpenGLVisualizer::initialize(int argc, char** argv,
    const std::vector<std::vector<double>>& data,
    const Config& config) {

    cout << "OpenGL инициализация начата..." << endl;

    if (data.empty()) {
        cerr << "ОШИБКА: Нет данных для визуализации!" << endl;
        return;
    }

    depthData = data;

    cout << "Данные загружены: " << depthData.size() << " x " << depthData[0].size() << endl;

    // Инициализация GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 700);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("3D Depth Map Viewer - Лабораторная работа 4");

    // Настройка OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);

    // Цвет фона
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Сохраняем конфигурацию
    currentConfig = config;

    // Базовая настройка освещения
    setupLighting();
    setupMaterial();
    setupLightingForModel(config.reflection_model);

    // Устанавливаем режим каркаса если нужно
    wireframeMode = config.wireframe_mode;
    showAxes = config.show_axes;

    cout << "Модель отражения: ";
    switch (config.reflection_model) {
    case 0: cout << "Ламберт" << endl; break;
    case 1: cout << "Фонга-Блинна" << endl; break;
    case 2: cout << "Торенса-Сперроу" << endl; break;
    default: cout << "Неизвестно" << endl; break;
    }

    // Устанавливаем функции обратного вызова
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutReshapeFunc(reshape);

    cout << "OpenGL визуализация инициализирована!" << endl;
    cout << "Обработка пропусков для нулевых глубин активирована" << endl;
}

void OpenGLVisualizer::run() {
    setlocale(LC_ALL, "Russian");
    printControls();
    glutMainLoop();
}