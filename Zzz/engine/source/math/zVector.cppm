#include "pch.h"
export module zVector;

namespace zzz::zmath
{
	template<typename T, std::size_t N>
	struct zVector
	{
		T data[N];

		// Конструктор по умолчанию
		zVector()
		{
			for (std::size_t i = 0; i < N; ++i)
				data[i] = (i == 3 && N == 4) ? T(1) : T(0);  // w = 1 для Vec4
		}

		// Конструктор от значений
		template<typename... Args>
		explicit zVector(Args... args) : data{ T(args)... }
		{
			static_assert(sizeof...(args) == N, ">>>>> [zVector::zVector(Args... args)]. Неправильное количество аргументов");
		}

		// Доступ
		T& operator[](std::size_t i) { return data[i]; }
		const T& operator[](std::size_t i) const { return data[i]; }

		// Прямые преобразования: Vector ? API
#if defined(ZRENDER_API_D3D12)
		template<std::size_t M = N>
		operator std::enable_if_t<M == 4, DirectX::XMFLOAT4>() const
		{
			return DirectX::XMFLOAT4{ data[0], data[1], data[2], data[3] };
		}
		template<std::size_t M = N>
		operator std::enable_if_t<M == 3, DirectX::XMFLOAT3>() const
		{
			return DirectX::XMFLOAT3{ data[0], data[1], data[2] };
		}
		template<std::size_t M = N>
		operator std::enable_if_t<M == 2, DirectX::XMFLOAT2>() const
		{
			return DirectX::XMFLOAT2{ data[0], data[1] };
		}
#elif defined(ZRENDER_API_VULKAN)
		template<std::size_t M = N>
		operator std::enable_if_t<M == 4, glm::vec<4, T, glm::defaultp>>() const
		{
			return glm::vec<4, T, glm::defaultp>{data[0], data[1], data[2], data[3]};
		}
		template<std::size_t M = N>
		operator std::enable_if_t<M == 3, glm::vec<3, T, glm::defaultp>>() const
		{
			return glm::vec<3, T, glm::defaultp>{data[0], data[1], data[2]};
		}
		template<std::size_t M = N>
		operator std::enable_if_t<M == 2, glm::vec<2, T, glm::defaultp>>() const 
		{
			return glm::vec<2, T, glm::defaultp>{data[0], data[1]};
		}
#elif defined(ZRENDER_API_METAL)
		template<std::size_t M = N>
		operator std::enable_if_t<M == 4, vector<T, 4>>() const
		{
			return vector<T, 4>{data[0], data[1], data[2], data[3]};
		}
		template<std::size_t M = N>
		operator std::enable_if_t<M == 3, vector<T, 3>>() const
		{
			return vector<T, 3>{data[0], data[1], data[2]};
		}
		template<std::size_t M = N>
		operator std::enable_if_t<M == 2, vector<T, 2>>() const
		{
			return vector<T, 2>{data[0], data[1]};
		}
#endif

		// Обратные конструкторы: API ? Vector
#if defined(ZRENDER_API_D3D12)
		template<std::size_t M = N>
		zVector(const std::enable_if_t<M == 4, DirectX::XMFLOAT4>& v)
		{
			data[0] = v.x; data[1] = v.y; data[2] = v.z; data[3] = v.w;
		}
		template<std::size_t M = N>
		zVector(const std::enable_if_t<M == 3, DirectX::XMFLOAT3>& v)
		{
			data[0] = v.x; data[1] = v.y; data[2] = v.z;
		}
		template<std::size_t M = N>
		zVector(const std::enable_if_t<M == 2, DirectX::XMFLOAT2>& v)
		{
			data[0] = v.x; data[1] = v.y;
		}
#elif defined(ZRENDER_API_VULKAN)
		template<std::size_t M = N>
		zVector(const std::enable_if_t<M == 4, glm::vec<4, T, glm::defaultp>>& v)
		{
			data[0] = v.x; data[1] = v.y; data[2] = v.z; data[3] = v.w;
		}
		template<std::size_t M = N>
		zVector(const std::enable_if_t<M == 3, glm::vec<3, T, glm::defaultp>>& v)
		{
			data[0] = v.x; data[1] = v.y; data[2] = v.z;
		}
		template<std::size_t M = N>
		zVector(const std::enable_if_t<M == 2, glm::vec<2, T, glm::defaultp>>& v)
		{
			data[0] = v.x; data[1] = v.y;
		}
#elif defined(ZRENDER_API_METAL)
		template<std::size_t M = N>
		zVector(const std::enable_if_t<M == 4, vector<T, 4>>& v)
		{
			data[0] = v.x; data[1] = v.y; data[2] = v.z; data[3] = v.w;
		}
		template<std::size_t M = N>
		zVector(const std::enable_if_t<M == 3, vector<T, 3>>& v)
		{
			data[0] = v.x; data[1] = v.y; data[2] = v.z;
		}
		template<std::size_t M = N>
		zVector(const std::enable_if_t<M == 2, vector<T, 2>>& v)
		{
			data[0] = v.x; data[1] = v.y;
		}
#endif
	};

	// Типы-псевдонимы
	export using zVec2 = zVector<float, 2>;
	export using zVec3 = zVector<float, 3>;
	export using zVec4 = zVector<float, 4>;
}