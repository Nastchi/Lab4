﻿#ifdef _WIN32
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

// Новые переменные для управления видом
float OpenGLVisualizer::cameraX = 0.0f;
float OpenGLVisualizer::cameraY = 0.0f;
float OpenGLVisualizer::cameraZ = 5.0f;
float OpenGLVisualizer::lookAtX = 0.0f;
float OpenGLVisualizer::lookAtY = 0.0f;
float OpenGLVisualizer::lookAtZ = 0.0f;
float OpenGLVisualizer::upX = 0.0f;
float OpenGLVisualizer::upY = 1.0f;
float OpenGLVisualizer::upZ = 0.0f;

int OpenGLVisualizer::windowWidth = 1020;
int OpenGLVisualizer::windowHeight = 820;

void OpenGLVisualizer::display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Используем камеру как в Python примере
    gluLookAt(cameraX, cameraY, cameraZ,
        lookAtX, lookAtY, lookAtZ,
        upX, upY, upZ);

    glRotatef(rotationX, 1, 0, 0);
    glRotatef(rotationY, 0, 1, 0);

    if (showAxes) {
        drawAxes();
    }

    if (!depthData.empty()) {
        drawDepthMapAsQuads();
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

void OpenGLVisualizer::drawDepthMapAsQuads() {
    if (depthData.empty() || depthData[0].empty()) {
        return;
    }

    int height = static_cast<int>(depthData.size());
    int width = static_cast<int>(depthData[0].size());

    if (height < 2 || width < 2) {
        return;
    }

    // Находим максимальную глубину для масштабирования
    double maxDepth = 1.0;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (depthData[i][j] > maxDepth) {
                maxDepth = depthData[i][j];
            }
        }
    }
    if (maxDepth == 0) maxDepth = 1.0;

    float scaleFactor = 200.0f; // Как в Python примере

    if (wireframeMode) {
        glDisable(GL_LIGHTING);
        glColor3f(0.3f, 0.6f, 1.0f);
        glLineWidth(1.0f);
        glBegin(GL_LINE_LOOP);
    }
    else {
        glBegin(GL_QUADS); // Используем QUADS как в Python примере
    }

    for (int i = 0; i < height - 1; ++i) {
        for (int j = 0; j < width - 1; ++j) {
            // Пропускаем точки с нулевой глубиной (фон)
            if (depthData[i][j] <= 0.0 || depthData[i][j + 1] <= 0.0 ||
                depthData[i + 1][j] <= 0.0 || depthData[i + 1][j + 1] <= 0.0) {
                continue;
            }

            // Координаты вершин как в Python примере
            float v1x = (j - width / 2.0f) / width;
            float v1y = (height / 2.0f - i) / height;
            float v1z = -static_cast<float>(depthData[i][j]) / maxDepth;

            float v2x = ((j + 1) - width / 2.0f) / width;
            float v2y = (height / 2.0f - i) / height;
            float v2z = -static_cast<float>(depthData[i][j + 1]) / maxDepth;

            float v3x = ((j + 1) - width / 2.0f) / width;
            float v3y = (height / 2.0f - (i + 1)) / height;
            float v3z = -static_cast<float>(depthData[i + 1][j + 1]) / maxDepth;

            float v4x = (j - width / 2.0f) / width;
            float v4y = (height / 2.0f - (i + 1)) / height;
            float v4z = -static_cast<float>(depthData[i + 1][j]) / maxDepth;

            // Масштабируем как в Python примере
            v1x *= scaleFactor; v1y *= scaleFactor; v1z *= scaleFactor;
            v2x *= scaleFactor; v2y *= scaleFactor; v2z *= scaleFactor;
            v3x *= scaleFactor; v3y *= scaleFactor; v3z *= scaleFactor;
            v4x *= scaleFactor; v4y *= scaleFactor; v4z *= scaleFactor;

            // Вычисляем нормаль
            float normal[3];
            calculateQuadNormal(v1x, v1y, v1z, v2x, v2y, v2z,
                v3x, v3y, v3z, v4x, v4y, v4z,
                normal[0], normal[1], normal[2]);

            if (!wireframeMode) {
                // Устанавливаем материал
                GLfloat materialColor[] = {
                    currentConfig.material_color.x,
                    currentConfig.material_color.y,
                    currentConfig.material_color.z,
                    1.0f
                };
                glMaterialfv(GL_FRONT, GL_DIFFUSE, materialColor);

                // Нормаль для освещения
                glNormal3f(normal[0], normal[1], normal[2]);
            }

            // Рисуем вершины
            glVertex3f(v1x, v1y, v1z);
            glVertex3f(v2x, v2y, v2z);
            glVertex3f(v3x, v3y, v3z);
            glVertex3f(v4x, v4y, v4z);
        }
    }

    glEnd();

    if (wireframeMode) {
        glEnable(GL_LIGHTING);
    }
}

void OpenGLVisualizer::calculateQuadNormal(float x1, float y1, float z1,
    float x2, float y2, float z2,
    float x3, float y3, float z3,
    float x4, float y4, float z4,
    float& nx, float& ny, float& nz) {
    // Вычисляем нормаль для квада как среднее двух треугольников
    float nx1, ny1, nz1, nx2, ny2, nz2;

    // Нормаль первого треугольника (v1, v2, v3)
    float u1x = x2 - x1;
    float u1y = y2 - y1;
    float u1z = z2 - z1;

    float v1x = x3 - x1;
    float v1y = y3 - y1;
    float v1z = z3 - z1;

    nx1 = u1y * v1z - u1z * v1y;
    ny1 = u1z * v1x - u1x * v1z;
    nz1 = u1x * v1y - u1y * v1x;

    // Нормаль второго треугольника (v1, v3, v4)
    float u2x = x3 - x1;
    float u2y = y3 - y1;
    float u2z = z3 - z1;

    float v2x = x4 - x1;
    float v2y = y4 - y1;
    float v2z = z4 - z1;

    nx2 = u2y * v2z - u2z * v2y;
    ny2 = u2z * v2x - u2x * v2z;
    nz2 = u2x * v2y - u2y * v2x;

    // Среднее значение
    nx = (nx1 + nx2) / 2.0f;
    ny = (ny1 + ny2) / 2.0f;
    nz = (nz1 + nz2) / 2.0f;

    // Нормализация
    float length = sqrtf(nx * nx + ny * ny + nz * nz);
    if (length > 0.0001f) {
        nx /= length;
        ny /= length;
        nz /= length;
    }
    else {
        nx = 0.0f;
        ny = 1.0f;
        nz = 0.0f;
    }
}

void OpenGLVisualizer::keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'w': case 'W': cameraZ -= 0.5f; break; // Движение вперед
    case 's': case 'S': cameraZ += 0.5f; break; // Движение назад
    case 'a': case 'A': cameraX -= 0.5f; break; // Движение влево
    case 'd': case 'D': cameraX += 0.5f; break; // Движение вправо
    case 'q': case 'Q': cameraY += 0.5f; break; // Движение вверх
    case 'e': case 'E': cameraY -= 0.5f; break; // Движение вниз
    case 'r': case 'R': resetView(); break;
    case ' ': wireframeMode = !wireframeMode; break; // Переключение режима
    case 27: exit(0); break; // ESC для выхода
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

void OpenGLVisualizer::mouse(int button, int state, int x, int y) {
    // Будущая реализация вращения мышью
}

void OpenGLVisualizer::motion(int x, int y) {
    // Будущая реализация вращения мышью
}

void OpenGLVisualizer::reshape(int width, int height) {
    windowWidth = width;
    windowHeight = height;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / height, 0.1, 1000.0); // Как в Python примере
    glMatrixMode(GL_MODELVIEW);
}

void OpenGLVisualizer::setupLighting() {
    // Освещение на основе конфигурации
    GLfloat light_position[] = {
        currentConfig.light_direction.x * 10.0f,
        currentConfig.light_direction.y * 10.0f,
        currentConfig.light_direction.z * 10.0f,
        1.0f
    };

    GLfloat light_ambient[] = {
        currentConfig.ambient_intensity,
        currentConfig.ambient_intensity,
        currentConfig.ambient_intensity,
        1.0f
    };

    GLfloat light_diffuse[] = {
        currentConfig.light_intensity,
        currentConfig.light_intensity,
        currentConfig.light_intensity,
        1.0f
    };

    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
}

void OpenGLVisualizer::setupMaterial() {
    // Настройка материала на основе конфигурации
    GLfloat material_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat material_diffuse[] = {
        currentConfig.material_color.x,
        currentConfig.material_color.y,
        currentConfig.material_color.z,
        1.0f
    };

    // Разные настройки для разных моделей отражения
    GLfloat material_specular[4];
    GLfloat material_shininess[1];

    switch (currentConfig.reflection_model) {
    case 0: // Ламберт
        material_specular[0] = 0.0f; material_specular[1] = 0.0f;
        material_specular[2] = 0.0f; material_specular[3] = 1.0f;
        material_shininess[0] = 0.0f;
        break;
    case 1: // Фонга-Блинна
        material_specular[0] = 0.5f; material_specular[1] = 0.5f;
        material_specular[2] = 0.5f; material_specular[3] = 1.0f;
        material_shininess[0] = currentConfig.phong_shininess;
        break;
    case 2: // Торренса-Сперроу
        material_specular[0] = 0.8f; material_specular[1] = 0.8f;
        material_specular[2] = 0.8f; material_specular[3] = 1.0f;
        material_shininess[0] = currentConfig.material_shininess * 2.0f;
        break;
    default:
        material_specular[0] = 0.5f; material_specular[1] = 0.5f;
        material_specular[2] = 0.5f; material_specular[3] = 1.0f;
        material_shininess[0] = 32.0f;
    }

    glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, material_shininess);
}

void OpenGLVisualizer::resetView() {
    rotationX = 0.0f;
    rotationY = 0.0f;
    zoom = 1.0f;
    cameraX = 0.0f;
    cameraY = 0.0f;
    cameraZ = 5.0f; // Как в Python примере (камера на расстоянии 5)
    lookAtX = 0.0f;
    lookAtY = 0.0f;
    lookAtZ = 0.0f;
    upX = 0.0f;
    upY = 1.0f;
    upZ = 0.0f;
    showAxes = true;
    wireframeMode = false;
}

void OpenGLVisualizer::printControls() {
    setlocale(LC_ALL, "Russian");

    cout << "\n=== Управление в OpenGL ===" << endl;
    cout << "W/S - движение вперед/назад" << endl;
    cout << "A/D - движение влево/вправо" << endl;
    cout << "Q/E - движение вверх/вниз" << endl;
    cout << "Стрелки - вращение модели" << endl;
    cout << "Пробел - переключение каркасного режима" << endl;
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

    // Инициализация GLUT как в Python примере
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("3D Depth Map Visualization - Лабораторная работа 4");

    // Настройка OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);

    // Цвет фона черный как в Python примере
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Сохраняем конфигурацию
    currentConfig = config;

    // Настройка камеры на основе конфигурации
    cameraX = config.viewer_position.x;
    cameraY = config.viewer_position.y;
    cameraZ = config.viewer_position.z;
    lookAtX = 0.0f;
    lookAtY = 0.0f;
    lookAtZ = 0.0f;
    upX = 0.0f;
    upY = 1.0f;
    upZ = 0.0f;

    // Настройка освещения и материала
    setupLighting();
    setupMaterial();

    // Устанавливаем режим каркаса если нужно
    wireframeMode = config.wireframe_mode;
    showAxes = config.show_axes;

    cout << "Модель отражения: ";
    switch (config.reflection_model) {
    case 0: cout << "Ламберт" << endl; break;
    case 1: cout << "Фонга-Блинна" << endl; break;
    case 2: cout << "Торренса-Сперроу" << endl; break;
    default: cout << "Неизвестно" << endl; break;
    }

    // Устанавливаем функции обратного вызова
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);

    cout << "OpenGL визуализация инициализирована!" << endl;
    cout << "Размер окна: " << windowWidth << "x" << windowHeight << endl;
}

void OpenGLVisualizer::run() {
    setlocale(LC_ALL, "Russian");
    printControls();
    glutMainLoop();
}