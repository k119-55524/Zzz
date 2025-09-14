#include "pch.h"
export module IShader;

import result;

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
		explicit IShader(std::wstring&& name);

		virtual ~IShader() = default;
		[[nodiscard]] inline std::wstring GetName() const noexcept { return m_Name; }

		virtual result<> InitializeByText(std::string&& srcVS, std::string&& srcPS) = 0;

	private:
		std::wstring m_Name;
	};

	IShader::IShader(std::wstring&& name) :
		m_Name{ std::move(name) }
	{
		ensure(!m_Name.empty(), ">>>>> [IShader::IShader( ... )]. Shader name cannot be empty.");
	}
}