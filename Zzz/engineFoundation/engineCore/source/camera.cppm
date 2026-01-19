
export module Camera;

export import Math;
export import Size2D;

using namespace zzz::math;

export namespace zzz
{
	export class Camera
	{
	public:
		Camera() noexcept :
			m_Position(0.0f, 0.0f, 5.0f, 1.0f),
			m_Target(0.0f, 0.0f, 0.0f, 1.0f),
			m_Up(0.0f, 1.0f, 0.0f, 0.0f),
			m_FovY(45.0f * 3.14159265f / 180.0f),
			m_AspectPreset(eAspectType::Ratio_16x9),
			m_NearPlane(0.1f),
			m_FarPlane(1000.0f),
			m_Left(-10.0f),
			m_Right(10.0f),
			m_Bottom(-10.0f),
			m_Top(10.0f),
			m_ProjectionType(eProjType::Perspective),
			m_ViewMatrixDirty(true),
			m_ProjectionMatrixDirty(true)
		{
			m_AspectRatio = GetAspect(m_AspectPreset);
		}

		// Перспективная камера
		//Camera(const Vector4& position, const Vector4& target, const Vector4& up,
		//	float fovYRadians, float aspectRatio, float nearPlane, float farPlane) noexcept :
		//	m_Position(position),
		//	m_Target(target),
		//	m_Up(up.normalized()),
		//	m_FovY(fovYRadians),
		//	m_AspectPreset(eAspectType::Custom),
		//	m_NearPlane(nearPlane),
		//	m_FarPlane(farPlane),
		//	m_Left(-10.0f),
		//	m_Right(10.0f),
		//	m_Bottom(-10.0f),
		//	m_Top(10.0f),
		//	m_ProjectionType(eProjType::Perspective),
		//	m_ViewMatrixDirty(true),
		//	m_ProjectionMatrixDirty(true),
		//{
		//	m_AspectRatio = aspectRatio;
		//}

		// Ортографическая камера
		//Camera(const Vector4& position, const Vector4& target, const Vector4& up,
		//	float left, float right, float bottom, float top,
		//	float nearPlane, float farPlane) noexcept :
		//	m_Position(position),
		//	m_Target(target),
		//	m_Up(up.normalized()),
		//	m_FovY(45.0f * 3.14159265f / 180.0f),
		//	m_AspectPreset(eAspectType::Ratio_16_9),
		//	m_NearPlane(nearPlane),
		//	m_FarPlane(farPlane),
		//	m_Left(left),
		//	m_Right(right),
		//	m_Bottom(bottom),
		//	m_Top(top),
		//	m_ProjectionType(eProjType::Orthographic),
		//	m_ViewMatrixDirty(true),
		//	m_ProjectionMatrixDirty(true)
		//{
		//	m_AspectRatio = GetAspect(m_AspectPreset);
		//}

		inline void SetPosition(const Vector3& position) noexcept { m_Position = position; m_ViewMatrixDirty = true; }
		inline const Vector3& GetPosition() const noexcept { return m_Position; }

		inline void SetTarget(const Vector3& target) noexcept { m_Target = target; m_ViewMatrixDirty = true; }
		inline const Vector3& GetTarget() const noexcept { return m_Target; }

		inline void SetUp(const Vector3& up) noexcept { m_Up = up.normalized(); m_ViewMatrixDirty = true; }
		inline const Vector3& GetUp() const noexcept { return m_Up; }

		inline Vector3 GetForward() const noexcept { return (m_Target - m_Position).normalized(); }
		inline Vector3 GetRight() const noexcept { return GetForward().cross3(m_Up).normalized(); }
		inline Vector3 GetActualUp() const noexcept { return GetRight().cross3(GetForward()).normalized(); }

		inline void Move(const Vector3& offset) noexcept { m_Position += offset; m_Target += offset; m_ViewMatrixDirty = true; }
		inline void MoveForward(float distance) noexcept { Move(GetForward() * distance); }
		inline void MoveRight(float distance) noexcept { Move(GetRight() * distance); }
		inline void MoveUp(float distance) noexcept { Move(m_Up * distance); }

		//inline void RotateAroundTarget(float yawRadians, float pitchRadians) noexcept
		//{
		//	Vector3 offset = m_Position - m_Target;
		//	float radius = offset.length();

		//	Matrix4x4 yawRotation = Matrix4x4::rotationY(yawRadians);
		//	offset = offset * yawRotation;

		//	Vector4 right = (m_Target - m_Position).normalized().cross3(m_Up).normalized();
		//	Matrix4x4 pitchRotation = Matrix4x4::rotation(right, pitchRadians);
		//	offset = offset * pitchRotation;

		//	m_Position = m_Target + offset.normalized() * radius;
		//	m_ViewMatrixDirty = true;
		//}

		inline void LookAt(const Vector3& target) noexcept { m_Target = target; m_ViewMatrixDirty = true; }

		//inline void Rotate(float yawRadians, float pitchRadians) noexcept
		//{
		//	Vector4 forward = GetForward();
		//	Vector4 right = GetRight();

		//	Matrix4x4 yawRotation = Matrix4x4::rotationY(yawRadians);
		//	forward = forward * yawRotation;

		//	Matrix4x4 pitchRotation = Matrix4x4::rotation(right, pitchRadians);
		//	forward = forward * pitchRotation;

		//	m_Target = m_Position + forward;
		//	m_ViewMatrixDirty = true;
		//}

		// TODO: Возможно, эти методы не нужны, так как есть SetFovY и SetAspectRatio
		//inline void SetPerspective(float fovYRadians, float aspectRatio, float nearPlane, float farPlane) noexcept
		//{
		//	m_FovY = fovYRadians;
		//	m_AspectRatio = aspectRatio;
		//	m_AspectPreset = eAspectType::Custom;
		//	m_NearPlane = nearPlane;
		//	m_FarPlane = farPlane;
		//	m_ProjectionType = eProjType::Perspective;
		//	m_ProjectionMatrixDirty = true;
		//}

		//inline void SetOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) noexcept
		//{
		//	m_Left = left;
		//	m_Right = right;
		//	m_Bottom = bottom;
		//	m_Top = top;
		//	m_NearPlane = nearPlane;
		//	m_FarPlane = farPlane;
		//	m_ProjectionType = eProjType::Orthographic;
		//	m_ProjectionMatrixDirty = true;
		//}

		inline const Matrix4x4& GetViewMatrix() const noexcept
		{
			if (m_ViewMatrixDirty)
			{
				m_ViewMatrix = Matrix4x4::lookAt(m_Position, m_Target, m_Up);
				m_ViewMatrixDirty = false;
			}

			return m_ViewMatrix;
		}

		inline const Matrix4x4& GetProjectionMatrix(Size2D<>& surfSize) const noexcept
		{
			float currentAspect = m_AspectRatio;
			if (m_AspectPreset == eAspectType::FullWindow)
				currentAspect = static_cast<float>(surfSize.width) / static_cast<float>(surfSize.height);

			//if (m_ProjectionMatrixDirty)
			{
				if (m_ProjectionType == eProjType::Perspective)
					m_ProjectionMatrix = Matrix4x4::perspective(m_FovY, currentAspect, m_NearPlane, m_FarPlane);
				else
					m_ProjectionMatrix = Matrix4x4::orthographic(m_Left, m_Right, m_Bottom, m_Top, m_NearPlane, m_FarPlane);
				m_ProjectionMatrixDirty = false;
			}

			return m_ProjectionMatrix;
		}

		inline Matrix4x4 GetProjectionViewMatrix(Size2D<>& surfSize) const noexcept { return GetViewMatrix() * GetProjectionMatrix(surfSize); }

		inline float GetFovY() const noexcept { return m_FovY; }
		inline void SetFovY(float fovYRadians) noexcept { m_FovY = fovYRadians; m_ProjectionMatrixDirty = true; }

		//inline float GetAspectRatio() const noexcept { return m_AspectRatio; }

		inline void SetAspectRatio(eAspectType type, float aspect = 0.0f) noexcept
		{
			m_AspectPreset = type;
			switch (type)
			{
			case eAspectType::Custom:
				if (aspect <= 0.0f)
					throw_runtime_error(">>>>> [Camera::SetAspectRatioPreset( ... )]. Custom aspect ratio must be greater than zero.");

				m_AspectRatio = aspect;
				break;
			case eAspectType::FullWindow:
				break;
			default:
				m_AspectRatio = GetAspect(type);
			}

			m_ProjectionMatrixDirty = true;
		}

		inline eAspectType GetAspectPreset() const noexcept { return m_AspectPreset; }

		inline float GetNearPlane() const noexcept { return m_NearPlane; }
		inline void SetNearPlane(float nearPlane) noexcept { m_NearPlane = nearPlane; m_ProjectionMatrixDirty = true; }

		inline float GetFarPlane() const noexcept { return m_FarPlane; }
		inline void SetFarPlane(float farPlane) noexcept { m_FarPlane = farPlane; m_ProjectionMatrixDirty = true; }

		inline eProjType GetProjectionType() const noexcept { return m_ProjectionType; }

		// Преобразование экранных координат (normalized device coords) в луч в мировом пространстве
		//inline Ray ScreenPointToRay(float ndcX, float ndcY) const noexcept
		//{
		//	if (m_ProjectionType == eProjType::Perspective)
		//	{
		//		return Ray::from_ndc_perspective(
		//			ndcX, ndcY,
		//			m_Position,
		//			GetViewMatrix(),
		//			m_FovY,
		//			m_AspectRatio);
		//	}
		//	else // Orthographic
		//	{
		//		return Ray::from_ndc_orthographic(
		//			ndcX, ndcY,
		//			GetViewMatrix(),
		//			m_Left, m_Right,
		//			m_Bottom, m_Top);
		//	}
		//}

		// Дополнительно: из пиксельных координат
		//inline Ray ScreenPixelsToRay(float screenX, float screenY,
		//	float screenWidth, float screenHeight) const noexcept
		//{
		//	// Конвертируем в NDC
		//	float ndcX = (2.0f * screenX) / screenWidth - 1.0f;
		//	float ndcY = 1.0f - (2.0f * screenY) / screenHeight;

		//	return ScreenPointToRay(ndcX, ndcY);
		//}

		// Frustum culling (базовая версия)
		struct Frustum
		{
			Vector4 planes[6]; // Left, Right, Bottom, Top, Near, Far (normal + distance)
		};

		//inline Frustum GetFrustum() const noexcept
		//{
		//	Frustum frustum;
		//	Matrix4x4 vp = GetProjectionViewMatrix();

		//	// Извлекаем плоскости из матрицы VP
		//	// Left plane
		//	frustum.planes[0] = Vector4(
		//		vp.at(0, 3) + vp.at(0, 0),
		//		vp.at(1, 3) + vp.at(1, 0),
		//		vp.at(2, 3) + vp.at(2, 0),
		//		vp.at(3, 3) + vp.at(3, 0)
		//	).normalized();

		//	// Right plane
		//	frustum.planes[1] = Vector4(
		//		vp.at(0, 3) - vp.at(0, 0),
		//		vp.at(1, 3) - vp.at(1, 0),
		//		vp.at(2, 3) - vp.at(2, 0),
		//		vp.at(3, 3) - vp.at(3, 0)
		//	).normalized();

		//	// Bottom plane
		//	frustum.planes[2] = Vector4(
		//		vp.at(0, 3) + vp.at(0, 1),
		//		vp.at(1, 3) + vp.at(1, 1),
		//		vp.at(2, 3) + vp.at(2, 1),
		//		vp.at(3, 3) + vp.at(3, 1)
		//	).normalized();

		//	// Top plane
		//	frustum.planes[3] = Vector4(
		//		vp.at(0, 3) - vp.at(0, 1),
		//		vp.at(1, 3) - vp.at(1, 1),
		//		vp.at(2, 3) - vp.at(2, 1),
		//		vp.at(3, 3) - vp.at(3, 1)
		//	).normalized();

		//	// Near plane
		//	frustum.planes[4] = Vector4(
		//		vp.at(0, 3) + vp.at(0, 2),
		//		vp.at(1, 3) + vp.at(1, 2),
		//		vp.at(2, 3) + vp.at(2, 2),
		//		vp.at(3, 3) + vp.at(3, 2)
		//	).normalized();

		//	// Far plane
		//	frustum.planes[5] = Vector4(
		//		vp.at(0, 3) - vp.at(0, 2),
		//		vp.at(1, 3) - vp.at(1, 2),
		//		vp.at(2, 3) - vp.at(2, 2),
		//		vp.at(3, 3) - vp.at(3, 2)
		//	).normalized();

		//	return frustum;
		//}

		// Проверка точки внутри frustum
		//inline bool isPointInFrustum(const Vector4& point) const noexcept
		//{
		//	Frustum frustum = GetFrustum();
		//	for (int i = 0; i < 6; ++i)
		//	{
		//		// Distance from point to plane
		//		float distance = point.dot(frustum.planes[i]) + frustum.planes[i][3];
		//		if (distance < 0.0f)
		//			return false;
		//	}
		//	return true;
		//}

		// Проверка сферы внутри frustum
		//inline bool isSphereInFrustum(const Vector4& center, float radius) const noexcept
		//{
		//	Frustum frustum = GetFrustum();
		//	for (int i = 0; i < 6; ++i)
		//	{
		//		float distance = center.dot(frustum.planes[i]) + frustum.planes[i][3];
		//		if (distance < -radius)
		//			return false;
		//	}
		//	return true;
		//}

		// Zoom
		//inline void Zoom(float factor) noexcept
		//{
		//	if (m_ProjectionType == eProjType::Perspective)
		//	{
		//		m_FovY *= factor;
		//		m_FovY = std::max(0.1f, std::min(m_FovY, 3.14f)); // Clamp [~6°, 180°]
		//		m_ProjectionMatrixDirty = true;
		//	}
		//	else
		//	{
		//		float width = m_Right - m_Left;
		//		float height = m_Top - m_Bottom;
		//		float centerX = (m_Left + m_Right) * 0.5f;
		//		float centerY = (m_Bottom + m_Top) * 0.5f;

		//		width *= factor;
		//		height *= factor;

		//		m_Left = centerX - width * 0.5f;
		//		m_Right = centerX + width * 0.5f;
		//		m_Bottom = centerY - height * 0.5f;
		//		m_Top = centerY + height * 0.5f;
		//		m_ProjectionMatrixDirty = true;
		//	}
		//}

	private:
		Vector3 m_Position;
		Vector3 m_Target;
		Vector3 m_Up;

		float m_FovY;
		float m_NearPlane;
		float m_FarPlane;

		eAspectType m_AspectPreset;
		float m_AspectRatio;

		float m_Left;
		float m_Right;
		float m_Bottom;
		float m_Top;

		eProjType m_ProjectionType;

		mutable Matrix4x4 m_ViewMatrix;
		mutable Matrix4x4 m_ProjectionMatrix;
		mutable bool m_ViewMatrixDirty;
		mutable bool m_ProjectionMatrixDirty;
	};
}
