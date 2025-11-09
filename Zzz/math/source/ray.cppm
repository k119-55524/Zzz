
#include "pch.h"

export module ray;

import vector4;
import matrix4x4;

export namespace zzz::math
{
	export class ray
	{
	public:
		// Конструкторы
		ray() noexcept : origin(0.0f, 0.0f, 0.0f, 1.0f), direction(0.0f, 0.0f, 1.0f, 0.0f) {}
		ray(const vector4& orig, const vector4& dir) noexcept : origin(orig), direction(dir.normalized()) {}

		// Вычисление точки на луче
		vector4 at(float t) const noexcept
		{
			return origin + direction * t;
		}

		vector4 point_at(float distance) const noexcept
		{
			return at(distance);
		}

		// Трансформация луча
		ray transform(const matrix4x4& matrix) const noexcept
		{
			vector4 newOrigin = matrix * origin;
			// Направление трансформируем без трансляции (w=0)
			vector4 newDir = matrix * vector4(direction[0], direction[1], direction[2], 0.0f);
			return ray(newOrigin, newDir);
		}

		// Пересечение с плоскостью
		struct PlaneIntersection
		{
			bool hit;
			float t;
			vector4 point;
		};

		// Плоскость задаётся точкой и нормалью
		PlaneIntersection intersect_plane(const vector4& planePoint, const vector4& planeNormal) const noexcept
		{
			PlaneIntersection result{ false, 0.0f, vector4() };

			float denom = direction.dot(planeNormal);

			// Луч параллелен плоскости
			if (std::abs(denom) < 1e-6f)
				return result;

			vector4 diff = planePoint - origin;
			float t = diff.dot(planeNormal) / denom;

			// Пересечение позади луча
			if (t < 0.0f)
				return result;

			result.hit = true;
			result.t = t;
			result.point = at(t);
			return result;
		}

		// Пересечение со сферой
		struct SphereIntersection
		{
			bool hit;
			float t_near;	// Ближайшая точка
			float t_far;	// Дальняя точка
			vector4 point_near;
			vector4 point_far;
		};

		SphereIntersection intersect_sphere(const vector4& center, float radius) const noexcept
		{
			SphereIntersection result{ false, 0.0f, 0.0f, vector4(), vector4() };

			vector4 oc = origin - center;
			float a = direction.dot(direction);
			float b = 2.0f * oc.dot(direction);
			float c = oc.dot(oc) - radius * radius;
			float discriminant = b * b - 4.0f * a * c;

			// Нет пересечения
			if (discriminant < 0.0f)
				return result;

			float sqrtD = std::sqrt(discriminant);
			float t1 = (-b - sqrtD) / (2.0f * a);
			float t2 = (-b + sqrtD) / (2.0f * a);

			// Обе точки позади луча
			if (t2 < 0.0f)
				return result;

			result.hit = true;
			result.t_near = (t1 >= 0.0f) ? t1 : t2;
			result.t_far = t2;
			result.point_near = at(result.t_near);
			result.point_far = at(result.t_far);

			return result;
		}

		// Пересечение с AABB (Axis-Aligned Bounding Box)
		struct AABBIntersection
		{
			bool hit;
			float t_min;
			float t_max;
			vector4 point_entry;
			vector4 point_exit;
		};

		AABBIntersection intersect_aabb(const vector4& box_min, const vector4& box_max) const noexcept
		{
			AABBIntersection result{ false, 0.0f, 0.0f, vector4(), vector4() };

			float t_min = -std::numeric_limits<float>::infinity();
			float t_max = std::numeric_limits<float>::infinity();

			// Проверяем пересечение по каждой оси
			for (int i = 0; i < 3; ++i)
			{
				if (std::abs(direction[i]) < 1e-8f)
				{
					// Луч параллелен этой оси
					if (origin[i] < box_min[i] || origin[i] > box_max[i])
						return result; // Нет пересечения
				}
				else
				{
					float inv_d = 1.0f / direction[i];
					float t1 = (box_min[i] - origin[i]) * inv_d;
					float t2 = (box_max[i] - origin[i]) * inv_d;

					if (t1 > t2) std::swap(t1, t2);

					t_min = std::max(t_min, t1);
					t_max = std::min(t_max, t2);

					if (t_min > t_max)
						return result; // Нет пересечения
				}
			}

			// Пересечение позади луча
			if (t_max < 0.0f)
				return result;

			result.hit = true;
			result.t_min = (t_min >= 0.0f) ? t_min : 0.0f;
			result.t_max = t_max;
			result.point_entry = at(result.t_min);
			result.point_exit = at(result.t_max);

			return result;
		}

		// Пересечение с треугольником (Moller–Trumbore algorithm)
		struct TriangleIntersection
		{
			bool hit;
			float t;
			float u, v; // Барицентрические координаты
			vector4 point;
		};

		TriangleIntersection intersect_triangle(const vector4& v0, const vector4& v1, const vector4& v2) const noexcept
		{
			TriangleIntersection result{ false, 0.0f, 0.0f, 0.0f, vector4() };

			const float EPSILON = 1e-8f;

			vector4 edge1 = v1 - v0;
			vector4 edge2 = v2 - v0;

			// Вычисляем cross product через компоненты
			vector4 h(
				direction[1] * edge2[2] - direction[2] * edge2[1],
				direction[2] * edge2[0] - direction[0] * edge2[2],
				direction[0] * edge2[1] - direction[1] * edge2[0],
				0.0f);

			float a = edge1.dot(h);

			// Луч параллелен треугольнику
			if (std::abs(a) < EPSILON)
				return result;

			float f = 1.0f / a;
			vector4 s = origin - v0;
			float u = f * s.dot(h);

			if (u < 0.0f || u > 1.0f)
				return result;

			vector4 q(
				s[1] * edge1[2] - s[2] * edge1[1],
				s[2] * edge1[0] - s[0] * edge1[2],
				s[0] * edge1[1] - s[1] * edge1[0],
				0.0f);

			float v = f * direction.dot(q);

			if (v < 0.0f || u + v > 1.0f)
				return result;

			float t = f * edge2.dot(q);

			if (t < EPSILON)
				return result;

			result.hit = true;
			result.t = t;
			result.u = u;
			result.v = v;
			result.point = at(t);

			return result;
		}

		// Расстояние от точки до луча
		float distance_to_point(const vector4& point) const noexcept
		{
			vector4 v = point - origin;
			float t = v.dot(direction);

			if (t < 0.0f)
				return v.length(); // Расстояние до origin

			vector4 projection = origin + direction * t;
			return (point - projection).length();
		}

		// Ближайшая точка на луче к заданной точке
		vector4 closest_point(const vector4& point) const noexcept
		{
			vector4 v = point - origin;
			float t = v.dot(direction);

			if (t < 0.0f)
				return origin;

			return at(t);
		}

#pragma region createtion rays from screen coords
		// Вариант 1: Из пиксельных координат экрана
		static ray from_screen_pixels(
			float screenX, float screenY,
			float screenWidth, float screenHeight,
			const matrix4x4& viewMatrix,
			const matrix4x4& projectionMatrix) noexcept
		{
			// Конвертируем в NDC
			float ndcX = (2.0f * screenX) / screenWidth - 1.0f;
			float ndcY = 1.0f - (2.0f * screenY) / screenHeight;

			return from_ndc(ndcX, ndcY, viewMatrix, projectionMatrix);
		}

		// Вариант 2: Из Normalized Device Coordinates (Нормализованные Координаты Устройства)
		static ray from_ndc(
			float ndcX, float ndcY,
			const matrix4x4& viewMatrix,
			const matrix4x4& projectionMatrix) noexcept
		{
			// Near и far плоскости в NDC
			vector4 rayNDC_near(ndcX, ndcY, -1.0f, 1.0f);  // Near plane
			vector4 rayNDC_far(ndcX, ndcY, 1.0f, 1.0f);    // Far plane

			// Обратная трансформация
			matrix4x4 invProj = projectionMatrix.inverted();
			matrix4x4 invView = viewMatrix.inverted();

			// Из NDC в view space
			vector4 rayView_near = invProj * rayNDC_near;
			vector4 rayView_far = invProj * rayNDC_far;

			// Perspective divide
			if (std::abs(rayView_near[3]) > 1e-6f)
			{
				rayView_near = rayView_near / rayView_near[3];
				rayView_far = rayView_far / rayView_far[3];
			}

			// Из view space в world space
			vector4 rayWorld_near = invView * rayView_near;
			vector4 rayWorld_far = invView * rayView_far;

			vector4 origin = rayWorld_near;
			vector4 direction = (rayWorld_far - rayWorld_near).normalized();

			return ray(origin, direction);
		}

		// Вариант 3: Оптимизированный для перспективной проекции
		static ray from_ndc_perspective(
			float ndcX, float ndcY,
			const vector4& cameraPosition,
			const matrix4x4& viewMatrix,
			float fovY,
			float aspectRatio) noexcept
		{
			ray ray;
			ray.origin = cameraPosition;

			// Обратная трансформация из NDC в view space
			float tanHalfFov = std::tan(fovY / 2.0f);
			float x = ndcX * aspectRatio * tanHalfFov;
			float y = ndcY * tanHalfFov;

			// Направление в view space (Z смотрит вперёд)
#if defined(ZRENDER_API_D3D12)
			vector4 directionView(x, y, 1.0f, 0.0f);  // Left-handed
#else
			vector4 directionView(x, y, -1.0f, 0.0f); // Right-handed
#endif

			// Трансформируем в world space
			matrix4x4 viewInv = viewMatrix.inverted();
			ray.direction = (viewInv * directionView).normalized();

			return ray;
		}

		// Вариант 4: Для ортографической проекции
		static ray from_ndc_orthographic(
			float ndcX, float ndcY,
			const matrix4x4& viewMatrix,
			float left, float right,
			float bottom, float top) noexcept
		{
			ray ray;

			// В ортографической проекции лучи параллельны
			float x = left + (ndcX + 1.0f) * 0.5f * (right - left);
			float y = bottom + (ndcY + 1.0f) * 0.5f * (top - bottom);

			matrix4x4 viewInv = viewMatrix.inverted();

			// Origin на near plane
			ray.origin = viewInv * vector4(x, y, 0.0f, 1.0f);

			// Direction - forward вектор камеры
#if defined(ZRENDER_API_D3D12)
			vector4 forwardView(0.0f, 0.0f, 1.0f, 0.0f);  // Left-handed
#else
			vector4 forwardView(0.0f, 0.0f, -1.0f, 0.0f); // Right-handed
#endif

			ray.direction = (viewInv * forwardView).normalized();

			return ray;
		}
#pragma endregion

		// Строковое представление
		std::string to_string() const
		{
			return "ray { origin: " + origin.to_string() + ", direction: " + direction.to_string() + " }";
		}

		private:
			vector4 origin;
			vector4 direction; // Должен быть нормализован
	};
}