#pragma once

#include "engine/EngineIncludes.h"

namespace zzz::engine
{
	class Engine;
	class IMonitorProvider;
	
	class Platform final
	{
	public:
		Platform() = delete;
		Platform(std::shared_ptr<NativeAppData> nativeData, const ProjectPlatformData& platformData);
		~Platform();

		[[nodiscard]] inline std::shared_ptr<NativeAppData> GetNativeData() const noexcept { return m_NativeData; }
		[[nodiscard]] inline const ProjectPlatformData& GetProjectPlatformData() const noexcept { return m_PlatformData; }
		[[nodiscard]] inline const HardwareState& GetHardwareState() const noexcept { return m_HardwareState; }
		[[nodiscard]] const IMonitorProvider& GetMonitorProvider() const noexcept;

#if Z_APPLE
		static constexpr bool c_AsyncRunLoop = true;
#else
		static constexpr bool c_AsyncRunLoop = false;
#endif

	private:
		void Initialize();
		void InitializePlatformSpecific();
		void ShutdownPlatformSpecific();
		[[nodiscard]] HardwareState GatherHardwareState() const;

		std::shared_ptr<NativeAppData> m_NativeData;
		ProjectPlatformData m_PlatformData;
		std::shared_ptr<IMonitorProvider> m_MonitorProvider;
		HardwareState m_HardwareState;
	};
}
