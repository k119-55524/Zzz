#pragma once

#include "../../NativeAppData.h"

using namespace zzz::io;

namespace zzz::engine
{
	class Engine;
	
	class Platform final
	{
	public:
		Platform() = delete;
		Platform(std::shared_ptr<NativeAppData> nativeData);
		~Platform();

		[[nodiscard]] inline std::shared_ptr<NativeAppData> GetNativeData() const noexcept { return m_NativeData; }

#if Z_APPLE
		static constexpr bool c_AsyncRunLoop = true;
#else
		static constexpr bool c_AsyncRunLoop = false;
#endif

	private:
		void Initialize();
		void InitializePlatformSpecific();
		void ShutdownPlatformSpecific();

		std::shared_ptr<NativeAppData> m_NativeData;
	};
}
