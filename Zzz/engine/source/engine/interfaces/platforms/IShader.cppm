#include "pch.h"
export module IShader;

import result;
import IMeshGPU;

export namespace zzz
{
	//enum class ShaderType
	//{
	//	Vertex,    // Вершинный шейдер
	//	Pixel,     // Пиксельный шейдер (ранее назывался фрагментный)
	//	Geometry,  // Геометрический шейдер
	//	Hull,      // Халловский шейдер (шейдер корпуса)
	//	Domain,    // Домейн шейдер (шейдер области)
	//	Compute    // Вычислительный шейдер
	//};

	export class IShader
	{
	public:
		IShader() = delete;
		explicit IShader(const std::shared_ptr<IMeshGPU> mesh, std::wstring&& name);

		virtual ~IShader() = default;
		[[nodiscard]] inline std::wstring GetName() const noexcept { return m_Name; }

		virtual result<> InitializeByText(std::string&& srcVS, std::string&& srcPS) = 0;

#if defined(RENDER_API_D3D12)

#elif defined(RENDER_API_VULKAN)
#elif defined(RENDER_API_METAL)
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

	private:
		const std::shared_ptr<IMeshGPU> m_Mesh;
		std::wstring m_Name;
	};

	IShader::IShader(const std::shared_ptr<IMeshGPU> mesh, std::wstring&& name) :
		m_Mesh{ mesh },
		m_Name{ std::move(name) }
	{
		ensure(m_Mesh != nullptr, ">>>>> [IShader::IShader( ... )]. Mesh pointer cannot be null.");
		ensure(!m_Name.empty(), ">>>>> [IShader::IShader( ... )]. Shader name cannot be empty.");
	}
}