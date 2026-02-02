
export module Transform;

export import Math;

using namespace zzz::math;

namespace zzz
{
	export class Transform
	{
	public:
		Transform() :
			m_Position{ 0, 0, 0 },
			m_Scale{ 1, 1, 1 },
			m_Rotation{ Quaternion::identity() },
			m_IsDirty{ true }
		{
		}

		Transform(const Transform& other) :
			m_Position(other.m_Position),
			m_Rotation(other.m_Rotation),
			m_Scale(other.m_Scale),
			m_IsDirty(other.m_IsDirty),
			m_WorldMatrix(other.m_WorldMatrix)
		{
		}

		Transform(Transform&& other) noexcept :
			m_Position(std::move(other.m_Position)),
			m_Rotation(std::move(other.m_Rotation)),
			m_Scale(std::move(other.m_Scale)),
			m_IsDirty(other.m_IsDirty),
			m_WorldMatrix(std::move(other.m_WorldMatrix))
		{
		}

		Transform& operator=(const Transform& other)
		{
			if (this != &other)
			{
				m_Position = other.m_Position;
				m_Rotation = other.m_Rotation;
				m_Scale = other.m_Scale;
				m_IsDirty = other.m_IsDirty;
				m_WorldMatrix = other.m_WorldMatrix;
			}
			return *this;
		}

		Transform& operator=(Transform&& other) noexcept
		{
			if (this != &other)
			{
				m_Position = std::move(other.m_Position);
				m_Rotation = std::move(other.m_Rotation);
				m_Scale = std::move(other.m_Scale);
				m_IsDirty = other.m_IsDirty;
				m_WorldMatrix = std::move(other.m_WorldMatrix);
			}
			return *this;
		}

		virtual ~Transform() = default;

		inline void SetPosition(float x, float y, float z) noexcept
		{
			m_Position = Vector3(x, y, z);
			m_IsDirty = true;
		}

		inline void SetPosition(const Vector3& position) noexcept
		{
			m_Position = position;
			m_IsDirty = true;
		}

		inline void Move(const Vector3& delta) noexcept
		{
			m_Position += delta;
			m_IsDirty = true;
		}

		inline void Move(float dx, float dy, float dz) noexcept
		{
			Move(Vector3(dx, dy, dz));
		}

		inline void SetRotation(const Quaternion& rotation) noexcept
		{
			m_Rotation = rotation;
			m_IsDirty = true;
		}

		inline void SetScale(const Vector3& scale) noexcept
		{
			m_Scale = scale;
			m_IsDirty = true;
		}

		[[nodiscard]] inline const Vector3& GetPosition() const noexcept { return m_Position; }
		[[nodiscard]] inline const Quaternion& GetRotation() const noexcept { return m_Rotation; }
		[[nodiscard]] inline const Vector3& GetScale() const noexcept { return m_Scale; }
		[[nodiscard]] inline const Matrix4x4& GetWorldMatrix() const noexcept
		{
			if (m_IsDirty)
			{
				Matrix4x4 translationMatrix = Matrix4x4::translation(m_Position);
				Matrix4x4 rotationMatrix = m_Rotation.toMatrix4x4();
				Matrix4x4 scaleMatrix = Matrix4x4::scale(m_Scale);
				m_WorldMatrix = translationMatrix * rotationMatrix * scaleMatrix;
				m_IsDirty = false;
			}

			return m_WorldMatrix;
		}

	protected:
		Vector3 m_Position;
		Quaternion m_Rotation;
		Vector3 m_Scale;

		mutable bool m_IsDirty;
		mutable Matrix4x4 m_WorldMatrix;
	};
}