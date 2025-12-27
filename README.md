# Lab4
## Лабораторная работа №4: Формирование изображения трехмерной поверхности

## 📋 Описание проекта
Программа для визуализации трехмерных моделей на основе карт глубины с поддержкой различных моделей отражения света и экспортом в несколько форматов.

## 🎯 Вариант задания
**Вариант 10:**
- **Язык программирования:** C++
- **Модели отражения:** Ламберта, Фонга-Блинна, Торенса-Сперроу
- **Форматы экспорта:** OBJ, STL, PLY

## ⚙️ Конфигурация
Программа настраивается через файл `resources/config.json`:

```json
{
  "depthMap": {
    "filename": "DepthMap_10.dat"
  },
  "output": {
    "directory": "output",
    "formats": ["ply", "obj", "stl"],
    "exportFilename": "model.ply"
  },
  "light": {
    "direction": [0.5, 0.7, 0.3],
    "intensity": 2.0,
    "ambientIntensity": 0.2
  },
  "viewer": {
    "position": [100, 100, 100]
  },
  "reflectionModel": "torrance-sparrow",
  "material": {
    "color": [0.6, 0.7, 0.9],
    "shininess": 32.0
  },
  "mesh": {
    "scaleZ": 0.5
  },
  "image": {
    "output": "render.bmp",
    "width": 1024,
    "height": 768
  },
  "display": {
    "wireframeMode": false,
    "showAxes": true
  },
  "preset": "torrance_ceramic",
  
  "lambert": {
    "diffuse": 0.8
  },
  "phong": {
    "shininess": 32.0,
    "specularStrength": 0.5
  },
  "torrance": {
    "roughness": 0.3,
    "metalness": 0.0,
    "reflectance": 0.04,
    "ior": 1.5
  }
}
```
### Модели отражения:
0 - Модель Ламберта
1 - Модель Фонга-Блинна 
2 - Модель Торренса-Сперроу 

## 🚀 Сборка и запуск
Требования:
Visual Studio 2019/2022 с поддержкой C++17
Библиотека FreeGLUT (включена в проект)

Компиляция:
```
l /EHsc /std:c++17 /I. /I"freeglut/include" Lab3DepthMapConverter.cpp depth_reader.cpp config_reader.cpp opengl_visualizer.cpp bmp_saver.cpp lighting_model.cpp mesh_exporter.cpp obj_writer.cpp ply_exporter.cpp stl_exporter.cpp reflection_models.cpp /link "freeglut/lib/x64/freeglut.lib" opengl32.lib glu32.lib /out:Lab4Demo.exe
```
Запуск:
```
Lab4.exe
```
