#pragma once

#include "engine/package/UserSettingsManager.h"
#include "engine/gapi/selectors/gpu/GpuRatingEvaluator.h"

namespace zzz::engine
{
	using namespace zzz::core;

	/**
	 * @brief Абстрактный селектор видеокарт для графического API.
	 */
	class IGpuSelector
	{
	public:
		IGpuSelector() = delete;
		explicit IGpuSelector(const std::shared_ptr<UserSettingsManager>& userSettings)
			: m_UserSettings(userSettings)
		{}
		virtual ~IGpuSelector() = default;

	protected:
		std::shared_ptr<UserSettingsManager> m_UserSettings;
	};
}
