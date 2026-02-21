
export module IDeviceCapabilities;

namespace zzz
{
	export class IDeviceCapabilities
	{
	public:
		static constexpr const wchar_t* cDefaultDeviceName = L"Unknown GPU";

		IDeviceCapabilities();
		IDeviceCapabilities(const IDeviceCapabilities&) = delete;
		IDeviceCapabilities(IDeviceCapabilities&&) = delete;
		IDeviceCapabilities& operator=(const IDeviceCapabilities&) = delete;
		IDeviceCapabilities& operator=(IDeviceCapabilities&&) = delete;
		virtual ~IDeviceCapabilities() = default;

		[[nodiscard]] inline bool IsSupportsRayTracing() const noexcept { return m_RayTracingSupported; }
		[[nodiscard]] inline bool IsSupportsVariableRateShading() const noexcept { return m_VariableRateShadingSupported; }
		[[nodiscard]] inline bool IsSupportsMeshShaders() const noexcept { return m_MeshShadersSupported; }
		[[nodiscard]] inline bool IsSupportsSamplerFeedback() const noexcept { return m_SamplerFeedbackSupported; }
		[[nodiscard]] inline bool IsSupportsCopyQueue() const noexcept { return m_CopyQueueSupported; }
		[[nodiscard]] inline bool IsSupportsDedicatedDMA() const noexcept { return m_DedicatedDMASupported; }

		[[nodiscard]] inline const std::wstring GetGPUName() const noexcept { return m_GPUName; };

		// TODO: Ќадо будет пересмотреть решение по "максимальной шейдерной модели" дл€ разных API. Ќо пока так.
		[[nodiscard]] virtual std::wstring GetHighestShaderModelAsString(eShaderType eShaderType) const noexcept = 0;

	protected:
		virtual void CheckSupported() = 0;

		bool m_RayTracingSupported;			// DXR
		bool m_VariableRateShadingSupported;// VRS
		bool m_MeshShadersSupported;		// Mesh Shaders  
		bool m_SamplerFeedbackSupported;	// Sampler Feedback
		bool m_CopyQueueSupported;			// CopyQueue
		bool m_DedicatedDMASupported;		// Dedicated DMA

		std::wstring m_GPUName;
		std::wstring m_HighestShaderModel;
	};

	IDeviceCapabilities::IDeviceCapabilities() :
		m_RayTracingSupported{ false },
		m_VariableRateShadingSupported{ false },
		m_MeshShadersSupported{ false },
		m_SamplerFeedbackSupported{ false },
		m_CopyQueueSupported{ false },
		m_DedicatedDMASupported{ false },
		m_GPUName{ cDefaultDeviceName },
		m_HighestShaderModel{}
	{
	}
}