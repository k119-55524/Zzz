#pragma once

#include "engine/EngineIncludes.h"
#include "engine/package/UserSettingsManager.h"

namespace zzz::engine
{
	using namespace zzz::core;

	class Engine;

	class IGAPI
	{
		friend class Engine;

	public:
		explicit IGAPI() = default;
		virtual ~IGAPI() = default;

		[[nodiscard]] static constexpr eGAPIType GetGAPIType() noexcept
		{
#if defined(Z_VULKAN)
			return eGAPIType::Vulkan;
#elif defined(Z_D3D12)
			return eGAPIType::DirectX12;
#elif defined(Z_METAL)
			return eGAPIType::Metal;
#endif
		}

		[[nodiscard]] inline bool IsCanDisableVSync() const noexcept { return m_IsCanDisableVSync; }

		virtual void WaitForGpu() = 0;

	protected:
		bool m_IsCanDisableVSync{ false };

	private:
		virtual void Initialize(std::shared_ptr<UserSettingsManager> userSettings) = 0;
	};
}
