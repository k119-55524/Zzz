
#include "pch.h"
export module AABB;

import ray;
import Vector4;

namespace zzz::math
{
	export class AABB
	{
	public:
		Vector4 min; // Минимальная точка бокса
		Vector4 max; // Максимальная точка бокса

		// Конструктор по умолчанию
		AABB() : min(Vector4()), max(Vector4()) {}

		// Конструктор с заданными границами
		AABB(const Vector4& minimum, const Vector4& maximum)
			: min(minimum), max(maximum) {
		}

		// Проверка пересечения луча с AABB
		// Использует алгоритм slab method
		bool intersect(const ray& r, float tMin, float tMax) const
		{
			Vector4 origin;
			Vector4 direction;
			for (int i = 0; i < 3; i++)
			{
				origin = r.getOrigin();
				direction = r.getDirection();
				float invD = 1.0f / direction[i];
				float t0 = (min[i] - origin[i]) * invD;
				float t1 = (max[i] - origin[i]) * invD;

				if (invD < 0.0f)
					std::swap(t0, t1);

				tMin = t0 > tMin ? t0 : tMin;
				tMax = t1 < tMax ? t1 : tMax;

				if (tMax <= tMin)
					return false;
			}

			return true;
		}

		// Проверка пересечения луча с AABB (упрощенная версия)
		bool intersect(const ray& r) const { return intersect(r, 0.0f, std::numeric_limits<float>::infinity()); }

		// Проверка, содержится ли точка внутри AABB
		bool contains(const Vector4& point) const
		{
			return (
				point.x() >= min.x() && point.x() <= max.x() &&
				point.y() >= min.y() && point.y() <= max.y() &&
				point.z() >= min.z() && point.z() <= max.z());
		}

		// Объединение двух AABB
		static AABB merge(const AABB& a, const AABB& b)
		{
			Vector4 newMin(
				std::min(a.min.x(), b.min.x()),
				std::min(a.min.y(), b.min.y()),
				std::min(a.min.z(), b.min.z()),
				0.0f);
			Vector4 newMax(
				std::max(a.max.x(), b.max.x()),
				std::max(a.max.y(), b.max.y()),
				std::max(a.max.z(), b.max.z()),
				0.0f);

			return AABB(newMin, newMax);
		}

		// Расширение AABB для включения точки
		void expand(const Vector4& point)
		{
			min.set_x(std::min(min.x(), point.x()));
			min.set_y(std::min(min.y(), point.y()));
			min.set_z(std::min(min.z(), point.z()));

			max.set_x(std::max(max.x(), point.x()));
			max.set_y(std::max(max.y(), point.y()));
			max.set_z(std::max(max.z(), point.z()));
		}

		// Получение центра AABB
		inline Vector4 center() const { return (min + max) * 0.5f; }

		// Получение размеров AABB
		Vector4 size() const { return max - min; }

		// Получение площади поверхности AABB
		float surfaceArea() const
		{
			Vector4 d = size();
			return 2.0f * (d.x() * d.y() + d.y() * d.z() + d.z() * d.x());
		}

		// Получение объема AABB
		float volume() const
		{
			Vector4 d = size();
			return d.x() * d.y() * d.z();
		}

		// Проверка пересечения двух AABB
		bool intersects(const AABB& other) const {
			return (
				min.x() <= other.max.x() && max.x() >= other.min.x() &&
				min.y() <= other.max.y() && max.y() >= other.min.y() &&
				min.z() <= other.max.z() && max.z() >= other.min.z());
		}
	};
}