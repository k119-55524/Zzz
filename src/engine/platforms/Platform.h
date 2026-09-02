#pragma once

#include "engine/EngineIncludes.h"
#include "core/hardware/HardwareState.h"

namespace zzz::engine
{
	class Engine;
	class IMonitorProvider;
	class HardwareManager;

	class Platform final
	{
	public:
		Platform() = delete;
		Platform(std::shared_ptr<NativeAppData> nativeData, const ProjectPlatformData& platformData);
		~Platform();

		[[nodiscard]] inline std::shared_ptr<NativeAppData> GetNativeData() const noexcept { return m_NativeData; }
		[[nodiscard]] inline const ProjectPlatformData& GetProjectPlatformData() const noexcept { return m_PlatformData; }
		[[nodiscard]] const HardwareState& GetHardwareState() const noexcept;
		[[nodiscard]] const IMonitorProvider& GetMonitorProvider() const noexcept;

#if defined(Z_APPLE)
		static constexpr bool c_AsyncRunLoop = true;
#else
		static constexpr bool c_AsyncRunLoop = false;
#endif

	private:
		void Initialize();
		void InitializePlatformSpecific();
		void ShutdownPlatformSpecific();

		std::shared_ptr<NativeAppData> m_NativeData;
		ProjectPlatformData m_PlatformData;
		std::shared_ptr<IMonitorProvider> m_MonitorProvider;
		std::unique_ptr<HardwareManager> m_HardwareManager;   // заменяет m_HardwareState; unique, т.к. единственный владелец (см. stage_06)
	};
}
