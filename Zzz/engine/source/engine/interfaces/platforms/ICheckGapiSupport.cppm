
#include "pch.h"
export module ICheckGapiSupport;

export namespace zzz::platforms
{
	export class ICheckGapiSupport
	{
	public:
		ICheckGapiSupport();
		ICheckGapiSupport(const ICheckGapiSupport&) = delete;
		ICheckGapiSupport(ICheckGapiSupport&&) = delete;
		ICheckGapiSupport& operator=(const ICheckGapiSupport&) = delete;
		ICheckGapiSupport& operator=(ICheckGapiSupport&&) = delete;
		virtual ~ICheckGapiSupport() = default;

		[[nodiscard]] inline bool SupportsRayTracing() const noexcept { return m_supportsRayTracing; }
		[[nodiscard]] inline bool SupportsVariableRateShading() const noexcept { return m_supportsVariableRateShading; }
		[[nodiscard]] inline bool SupportsMeshShaders() const noexcept { return m_supportsMeshShaders; }
		[[nodiscard]] inline bool SupportsSamplerFeedback() const noexcept { return m_supportsSamplerFeedback; }
		[[nodiscard]] inline bool SupportsCopyQueue() const noexcept { return m_supportsCopyQueue; }
		[[nodiscard]] inline bool SupportsDedicatedDMA() const noexcept { return m_supportsDedicatedDMA; }

		[[nodiscard]] virtual std::string GetHighestShaderModelAsString(ShaderType shaderType) const = 0;

	protected:
		virtual void CheckSupported() = 0;

		bool m_supportsRayTracing;			// DXR
		bool m_supportsVariableRateShading;	// VRS
		bool m_supportsMeshShaders;			// Mesh Shaders  
		bool m_supportsSamplerFeedback;		// Sampler Feedback
		bool m_supportsCopyQueue;			// CopyQueue
		bool m_supportsDedicatedDMA;		// Dedicated DMA

		std::string m_strSupportHiShaderModel;
	};

	ICheckGapiSupport::ICheckGapiSupport() :
		m_supportsRayTracing{ false },
		m_supportsVariableRateShading{ false },
		m_supportsMeshShaders{ false },
		m_supportsSamplerFeedback{ false },
		m_supportsCopyQueue{ false },
		m_supportsDedicatedDMA{ false },
		m_strSupportHiShaderModel{}
	{
	}
}