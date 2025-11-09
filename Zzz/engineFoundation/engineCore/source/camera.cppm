#include "pch.h"

export module camera;
import vector4;
import matrix4x4;

using namespace zzz::math;

namespace zzz::engineCore
{
	export enum class ProjectionType
	{
		Perspective,
		Orthographic
	};

	export class camera
	{
	public:
		// --- Конструкторы ---
		camera() noexcept
			: m_position(0.0f, 0.0f, 5.0f, 1.0f)
			, m_target(0.0f, 0.0f, 0.0f, 1.0f)
			, m_up(0.0f, 1.0f, 0.0f, 0.0f)
			, m_fovY(45.0f * 3.14159265f / 180.0f)  // 45 градусов в радианах
			, m_aspectRatio(16.0f / 9.0f)
			, m_nearPlane(0.1f)
			, m_farPlane(1000.0f)
			, m_left(-10.0f)
			, m_right(10.0f)
			, m_bottom(-10.0f)
			, m_top(10.0f)
			, m_projectionType(ProjectionType::Perspective)
			, m_viewMatrixDirty(true)
			, m_projectionMatrixDirty(true)
		{}

		// Перспективная камера
		camera(const vector4& position, const vector4& target, const vector4& up,
			float fovYRadians, float aspectRatio, float nearPlane, float farPlane) noexcept
			: m_position(position)
			, m_target(target)
			, m_up(up.normalized())
			, m_fovY(fovYRadians)
			, m_aspectRatio(aspectRatio)
			, m_nearPlane(nearPlane)
			, m_farPlane(farPlane)
			, m_left(-10.0f)
			, m_right(10.0f)
			, m_bottom(-10.0f)
			, m_top(10.0f)
			, m_projectionType(ProjectionType::Perspective)
			, m_viewMatrixDirty(true)
			, m_projectionMatrixDirty(true)
		{}

		// Ортографическая камера
		camera(const vector4& position, const vector4& target, const vector4& up,
			float left, float right, float bottom, float top,
			float nearPlane, float farPlane) noexcept
			: m_position(position)
			, m_target(target)
			, m_up(up.normalized())
			, m_fovY(45.0f * 3.14159265f / 180.0f)
			, m_aspectRatio(16.0f / 9.0f)
			, m_nearPlane(nearPlane)
			, m_farPlane(farPlane)
			, m_left(left)
			, m_right(right)
			, m_bottom(bottom)
			, m_top(top)
			, m_projectionType(ProjectionType::Orthographic)
			, m_viewMatrixDirty(true)
			, m_projectionMatrixDirty(true)
		{}

		// Позиция и ориентация
		void setPosition(const vector4& position) noexcept
		{
			m_position = position;
			m_viewMatrixDirty = true;
		}

		const vector4& getPosition() const noexcept { return m_position; }

		void setTarget(const vector4& target) noexcept
		{
			m_target = target;
			m_viewMatrixDirty = true;
		}

		const vector4& getTarget() const noexcept { return m_target; }

		void setUp(const vector4& up) noexcept
		{
			m_up = up.normalized();
			m_viewMatrixDirty = true;
		}

		const vector4& getUp() const noexcept { return m_up; }

		// Векторы локальной системы координат камеры
		vector4 getForward() const noexcept
		{
			return (m_target - m_position).normalized();
		}

		vector4 getRight() const noexcept
		{
			vector4 forward = getForward();
			return forward.cross3(m_up).normalized();
		}

		vector4 getActualUp() const noexcept
		{
			vector4 forward = getForward();
			vector4 right = getRight();
			return right.cross3(forward).normalized();
		}

		// Движение камеры
		void move(const vector4& offset) noexcept
		{
			m_position += offset;
			m_target += offset;
			m_viewMatrixDirty = true;
		}

		void moveForward(float distance) noexcept
		{
			vector4 forward = getForward();
			move(forward * distance);
		}

		void moveRight(float distance) noexcept
		{
			vector4 right = getRight();
			move(right * distance);
		}

		void moveUp(float distance) noexcept
		{
			move(m_up * distance);
		}

		// Вращение камеры
		void rotateAroundTarget(float yawRadians, float pitchRadians) noexcept
		{
			vector4 offset = m_position - m_target;
			float radius = offset.length();

			// Вращение по Yaw (вокруг мировой оси Y)
			matrix4x4 yawRotation = matrix4x4::rotationY(yawRadians);
			offset = yawRotation * offset;

			// Вращение по Pitch (вокруг локальной оси Right)
			vector4 right = (m_target - m_position).normalized().cross3(m_up).normalized();
			matrix4x4 pitchRotation = matrix4x4::rotation(right, pitchRadians);
			offset = pitchRotation * offset;

			m_position = m_target + offset.normalized() * radius;
			m_viewMatrixDirty = true;
		}

		void lookAt(const vector4& target) noexcept
		{
			m_target = target;
			m_viewMatrixDirty = true;
		}

		void rotate(float yawRadians, float pitchRadians) noexcept
		{
			// Вращение направления взгляда
			vector4 forward = getForward();
			vector4 right = getRight();

			// Yaw (поворот влево-вправо)
			matrix4x4 yawRotation = matrix4x4::rotationY(yawRadians);
			forward = yawRotation * forward;

			// Pitch (поворот вверх-вниз)
			matrix4x4 pitchRotation = matrix4x4::rotation(right, pitchRadians);
			forward = pitchRotation * forward;

			m_target = m_position + forward;
			m_viewMatrixDirty = true;
		}

		// Перспективная проекция
		void setPerspective(float fovYRadians, float aspectRatio,
			float nearPlane, float farPlane) noexcept
		{
			m_fovY = fovYRadians;
			m_aspectRatio = aspectRatio;
			m_nearPlane = nearPlane;
			m_farPlane = farPlane;
			m_projectionType = ProjectionType::Perspective;
			m_projectionMatrixDirty = true;
		}

		// Ортографическая проекция
		void setOrthographic(float left, float right, float bottom, float top,
			float nearPlane, float farPlane) noexcept
		{
			m_left = left;
			m_right = right;
			m_bottom = bottom;
			m_top = top;
			m_nearPlane = nearPlane;
			m_farPlane = farPlane;
			m_projectionType = ProjectionType::Orthographic;
			m_projectionMatrixDirty = true;
		}

		// Получение матриц
		const matrix4x4& getViewMatrix() const noexcept
		{
			if (m_viewMatrixDirty)
			{
				m_viewMatrix = matrix4x4::lookAt(m_position, m_target, m_up);
				m_viewMatrixDirty = false;
			}
			return m_viewMatrix;
		}

		const matrix4x4& getProjectionMatrix() const noexcept
		{
			if (m_projectionMatrixDirty)
			{
				if (m_projectionType == ProjectionType::Perspective)
				{
					m_projectionMatrix = matrix4x4::perspective(
						m_fovY, m_aspectRatio, m_nearPlane, m_farPlane
					);
				}
				else
				{
					m_projectionMatrix = matrix4x4::orthographic(
						m_left, m_right, m_bottom, m_top, m_nearPlane, m_farPlane
					);
				}
				m_projectionMatrixDirty = false;
			}
			return m_projectionMatrix;
		}

		matrix4x4 getViewProjectionMatrix() const noexcept
		{
			return getProjectionMatrix() * getViewMatrix();
		}

		// Параметры
		float getFovY() const noexcept { return m_fovY; }
		void setFovY(float fovYRadians) noexcept
		{
			m_fovY = fovYRadians;
			m_projectionMatrixDirty = true;
		}

		float getAspectRatio() const noexcept { return m_aspectRatio; }
		void setAspectRatio(float aspectRatio) noexcept
		{
			m_aspectRatio = aspectRatio;
			m_projectionMatrixDirty = true;
		}

		float getNearPlane() const noexcept { return m_nearPlane; }
		void setNearPlane(float nearPlane) noexcept
		{
			m_nearPlane = nearPlane;
			m_projectionMatrixDirty = true;
		}

		float getFarPlane() const noexcept { return m_farPlane; }
		void setFarPlane(float farPlane) noexcept
		{
			m_farPlane = farPlane;
			m_projectionMatrixDirty = true;
		}

		ProjectionType getProjectionType() const noexcept { return m_projectionType; }

		// Ray casting (для picking)
		struct Ray
		{
			vector4 origin;
			vector4 direction;
		};

		// Преобразование экранных координат (normalized device coords) в луч в мировом пространстве
		Ray screenPointToRay(float ndcX, float ndcY) const noexcept
		{
			// NDC: x, y в диапазоне [-1, 1]
			Ray ray;
			ray.origin = m_position;

			if (m_projectionType == ProjectionType::Perspective)
			{
				// Обратная трансформация из NDC в view space
				float tanHalfFov = std::tan(m_fovY / 2.0f);
				float x = ndcX * m_aspectRatio * tanHalfFov;
				float y = ndcY * tanHalfFov;

				// Направление в view space
				vector4 directionView(x, y, -1.0f, 0.0f);

				// Трансформируем в world space
				matrix4x4 viewInv = getViewMatrix().inverted();
				ray.direction = (viewInv * directionView).normalized();
			}
			else // Orthographic
			{
				// В ортографической проекции лучи параллельны
				float x = m_left + (ndcX + 1.0f) * 0.5f * (m_right - m_left);
				float y = m_bottom + (ndcY + 1.0f) * 0.5f * (m_top - m_bottom);

				matrix4x4 viewInv = getViewMatrix().inverted();
				ray.origin = viewInv * vector4(x, y, 0.0f, 1.0f);
				ray.direction = getForward();
			}

			return ray;
		}

		// Frustum culling (базовая версия)
		struct Frustum
		{
			vector4 planes[6]; // Left, Right, Bottom, Top, Near, Far (normal + distance)
		};

		Frustum getFrustum() const noexcept
		{
			Frustum frustum;
			matrix4x4 vp = getViewProjectionMatrix();

			// Извлекаем плоскости из матрицы VP
			// Left plane
			frustum.planes[0] = vector4(
				vp.at(0, 3) + vp.at(0, 0),
				vp.at(1, 3) + vp.at(1, 0),
				vp.at(2, 3) + vp.at(2, 0),
				vp.at(3, 3) + vp.at(3, 0)
			).normalized();

			// Right plane
			frustum.planes[1] = vector4(
				vp.at(0, 3) - vp.at(0, 0),
				vp.at(1, 3) - vp.at(1, 0),
				vp.at(2, 3) - vp.at(2, 0),
				vp.at(3, 3) - vp.at(3, 0)
			).normalized();

			// Bottom plane
			frustum.planes[2] = vector4(
				vp.at(0, 3) + vp.at(0, 1),
				vp.at(1, 3) + vp.at(1, 1),
				vp.at(2, 3) + vp.at(2, 1),
				vp.at(3, 3) + vp.at(3, 1)
			).normalized();

			// Top plane
			frustum.planes[3] = vector4(
				vp.at(0, 3) - vp.at(0, 1),
				vp.at(1, 3) - vp.at(1, 1),
				vp.at(2, 3) - vp.at(2, 1),
				vp.at(3, 3) - vp.at(3, 1)
			).normalized();

			// Near plane
			frustum.planes[4] = vector4(
				vp.at(0, 3) + vp.at(0, 2),
				vp.at(1, 3) + vp.at(1, 2),
				vp.at(2, 3) + vp.at(2, 2),
				vp.at(3, 3) + vp.at(3, 2)
			).normalized();

			// Far plane
			frustum.planes[5] = vector4(
				vp.at(0, 3) - vp.at(0, 2),
				vp.at(1, 3) - vp.at(1, 2),
				vp.at(2, 3) - vp.at(2, 2),
				vp.at(3, 3) - vp.at(3, 2)
			).normalized();

			return frustum;
		}

		// Проверка точки внутри frustum
		bool isPointInFrustum(const vector4& point) const noexcept
		{
			Frustum frustum = getFrustum();
			for (int i = 0; i < 6; ++i)
			{
				// Distance from point to plane
				float distance = point.dot(frustum.planes[i]) + frustum.planes[i][3];
				if (distance < 0.0f)
					return false;
			}
			return true;
		}

		// Проверка сферы внутри frustum
		bool isSphereInFrustum(const vector4& center, float radius) const noexcept
		{
			Frustum frustum = getFrustum();
			for (int i = 0; i < 6; ++i)
			{
				float distance = center.dot(frustum.planes[i]) + frustum.planes[i][3];
				if (distance < -radius)
					return false;
			}
			return true;
		}

		// Zoom
		void zoom(float factor) noexcept
		{
			if (m_projectionType == ProjectionType::Perspective)
			{
				m_fovY *= factor;
				m_fovY = std::max(0.1f, std::min(m_fovY, 3.14f)); // Clamp [~6°, 180°]
				m_projectionMatrixDirty = true;
			}
			else
			{
				float width = m_right - m_left;
				float height = m_top - m_bottom;
				float centerX = (m_left + m_right) * 0.5f;
				float centerY = (m_bottom + m_top) * 0.5f;

				width *= factor;
				height *= factor;

				m_left = centerX - width * 0.5f;
				m_right = centerX + width * 0.5f;
				m_bottom = centerY - height * 0.5f;
				m_top = centerY + height * 0.5f;
				m_projectionMatrixDirty = true;
			}
		}

	private:
		// Позиция и ориентация
		vector4 m_position;
		vector4 m_target;
		vector4 m_up;

		// Параметры перспективной проекции
		float m_fovY;
		float m_aspectRatio;
		float m_nearPlane;
		float m_farPlane;

		// Параметры ортографической проекции
		float m_left;
		float m_right;
		float m_bottom;
		float m_top;

		// Тип проекции
		ProjectionType m_projectionType;

		// Кэшированные матрицы (mutable для ленивого вычисления в const методах)
		mutable matrix4x4 m_viewMatrix;
		mutable matrix4x4 m_projectionMatrix;
		mutable bool m_viewMatrixDirty;
		mutable bool m_projectionMatrixDirty;

		//// Вспомогательная функция для cross product
		//static vector4 cross3(const vector4& a, const vector4& b) noexcept
		//{
		//	return vector4(
		//		a[1] * b[2] - a[2] * b[1],
		//		a[2] * b[0] - a[0] * b[2],
		//		a[0] * b[1] - a[1] * b[0],
		//		0.0f);
		//}
	};
}