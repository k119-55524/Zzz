
#include "pch.h"

export module Ray;

import Vector4;
import Matrix4x4;

export namespace zzz::math
{
	export class Ray
	{
	public:
		// Конструкторы
		Ray() noexcept : origin(0.0f, 0.0f, 0.0f, 1.0f), direction(0.0f, 0.0f, 1.0f, 0.0f) {}
		Ray(const Vector4& orig, const Vector4& dir) noexcept : origin(orig), direction(dir.normalized()) {}

		inline Vector4 getOrigin() const noexcept { return origin; }
		inline Vector4 getDirection() const noexcept { return direction; }

		// Вычисление точки на луче
		inline Vector4 at(float t) const noexcept { return origin + direction * t; }

		Vector4 point_at(float distance) const noexcept { return at(distance); }

		// Трансформация луча
		Ray transform(const Matrix4x4& matrix) const noexcept
		{
			Vector4 newOrigin = matrix * origin;
			// Направление трансформируем без трансляции (w=0)
			Vector4 newDir = matrix * Vector4(direction[0], direction[1], direction[2], 0.0f);

			return Ray(newOrigin, newDir);
		}

		// Расстояние от точки до луча
		float distance_to_point(const Vector4& point) const noexcept
		{
			Vector4 v = point - origin;
			float t = v.dot(direction);

			if (t < 0.0f)
				return v.length(); // Расстояние до origin

			Vector4 projection = origin + direction * t;
			return (point - projection).length();
		}

		// Ближайшая точка на луче к заданной точке
		Vector4 closest_point(const Vector4& point) const noexcept
		{
			Vector4 v = point - origin;
			float t = v.dot(direction);

			if (t < 0.0f)
				return origin;

			return at(t);
		}

#pragma region createtion rays from screen coords
		// Вариант 1: Из пиксельных координат экрана
		static Ray from_screen_pixels(
			float screenX, float screenY,
			float screenWidth, float screenHeight,
			const Matrix4x4& viewMatrix,
			const Matrix4x4& projectionMatrix) noexcept
		{
			// Конвертируем в NDC
			float ndcX = (2.0f * screenX) / screenWidth - 1.0f;
			float ndcY = 1.0f - (2.0f * screenY) / screenHeight;

			return from_ndc(ndcX, ndcY, viewMatrix, projectionMatrix);
		}

		// Вариант 2: Из Normalized Device Coordinates (Нормализованные Координаты Устройства)
		static Ray from_ndc(
			float ndcX, float ndcY,
			const Matrix4x4& viewMatrix,
			const Matrix4x4& projectionMatrix) noexcept
		{
			// Near и far плоскости в NDC
			Vector4 rayNDC_near(ndcX, ndcY, -1.0f, 1.0f);  // Near plane
			Vector4 rayNDC_far(ndcX, ndcY, 1.0f, 1.0f);    // Far plane

			// Обратная трансформация
			Matrix4x4 invProj = projectionMatrix.inverted();
			Matrix4x4 invView = viewMatrix.inverted();

			// Из NDC в view space
			Vector4 rayView_near = invProj * rayNDC_near;
			Vector4 rayView_far = invProj * rayNDC_far;

			// Perspective divide
			if (std::abs(rayView_near[3]) > 1e-6f)
			{
				rayView_near = rayView_near / rayView_near[3];
				rayView_far = rayView_far / rayView_far[3];
			}

			// Из view space в world space
			Vector4 rayWorld_near = invView * rayView_near;
			Vector4 rayWorld_far = invView * rayView_far;

			Vector4 origin = rayWorld_near;
			Vector4 direction = (rayWorld_far - rayWorld_near).normalized();

			return Ray(origin, direction);
		}

		// Вариант 3: Оптимизированный для перспективной проекции
		static Ray from_ndc_perspective(
			float ndcX, float ndcY,
			const Vector4& cameraPosition,
			const Matrix4x4& viewMatrix,
			float fovY,
			float aspectRatio) noexcept
		{
			Ray ray;
			ray.origin = cameraPosition;

			// Обратная трансформация из NDC в view space
			float tanHalfFov = std::tan(fovY / 2.0f);
			float x = ndcX * aspectRatio * tanHalfFov;
			float y = ndcY * tanHalfFov;

			// Направление в view space (Z смотрит вперёд)
#if defined(ZRENDER_API_D3D12)
			Vector4 directionView(x, y, 1.0f, 0.0f);  // Left-handed
#else
			Vector4 directionView(x, y, -1.0f, 0.0f); // Right-handed
#endif

			// Трансформируем в world space
			Matrix4x4 viewInv = viewMatrix.inverted();
			ray.direction = (viewInv * directionView).normalized();

			return ray;
		}

		// Вариант 4: Для ортографической проекции
		static Ray from_ndc_orthographic(
			float ndcX, float ndcY,
			const Matrix4x4& viewMatrix,
			float left, float right,
			float bottom, float top) noexcept
		{
			Ray ray;

			// В ортографической проекции лучи параллельны
			float x = left + (ndcX + 1.0f) * 0.5f * (right - left);
			float y = bottom + (ndcY + 1.0f) * 0.5f * (top - bottom);

			Matrix4x4 viewInv = viewMatrix.inverted();

			// Origin на near plane
			ray.origin = viewInv * Vector4(x, y, 0.0f, 1.0f);

			// Direction - forward вектор камеры
#if defined(ZRENDER_API_D3D12)
			Vector4 forwardView(0.0f, 0.0f, 1.0f, 0.0f);  // Left-handed
#else
			Vector4 forwardView(0.0f, 0.0f, -1.0f, 0.0f); // Right-handed
#endif

			ray.direction = (viewInv * forwardView).normalized();

			return ray;
		}
#pragma endregion

		// Строковое представление
		std::string to_string() const { return "ray { origin: " + origin.to_string() + ", direction: " + direction.to_string() + " }"; }

		private:
			Vector4 origin;
			Vector4 direction; // Должен быть нормализован
	};
}