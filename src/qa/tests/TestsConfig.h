#pragma once

// =============================================================================
// Конфигурация запуска наборов тестов EngineTests
// Закомментируйте / раскомментируйте нужные дефайны для включения групп тестов
// =============================================================================

// --- Математический модуль (math/) ---
#define Z_TEST_MATH_VECTORS         // Векторы Vec2, Vec3, Vec4
#define Z_TEST_MATH_POINT2D         // Point2D (открытые x, y, data, operator[])
#define Z_TEST_MATH_SIZE2D          // Size2D (открытые width, height)
#define Z_TEST_MATH_RECT2D          // Rect2D (position, size, bounding rects)
#define Z_TEST_MATH_MAT3            // Матрицы Mat3 (Пункт 2)
#define Z_TEST_MATH_MAT4            // Матрицы Mat4 (Пункт 2)
#define Z_TEST_MATH_QUAT            // Кватернионы Quat (Пункт 3)

// --- Ядро и сериализация (core/) ---
#define Z_TEST_CORE_SERIALIZATION   // Бинарная сериализация векторов и геометрии
#define Z_TEST_CORE_TEMPLATES       // Color, Event, DoubleBufferedVector
#define Z_TEST_CORE_ENUMS_STRUCTS   // Базовые перечисления, форматы, ConverterGAPITypes, Vertex3D, AttributeRange, AlignUp (Пункт 4)
