
export module IShader;

import IGAPI;
import Result;
import IMeshGPU;
import IShader_DirectX;

export namespace zzz
{
	export class IShader :
		public IShader_DirectX
	{
	public:
		IShader() = delete;
		explicit IShader(const std::shared_ptr<IGAPI> gapi, const std::shared_ptr<IMeshGPU> mesh, std::wstring&& name);

		virtual ~IShader() = default;
		[[nodiscard]] inline std::wstring GetName() const noexcept { return m_Name; }

		virtual Result<> InitializeByText(std::string&& srcVS, std::string&& srcPS) = 0;

	protected:
		const std::shared_ptr<IGAPI> m_GAPI;
		std::wstring m_Name;

	private:
		const std::shared_ptr<IMeshGPU> m_Mesh;
	};

	IShader::IShader(const std::shared_ptr<IGAPI> gapi, const std::shared_ptr<IMeshGPU> mesh, std::wstring&& name) :
		m_GAPI{ gapi },
		m_Mesh{ mesh },
		m_Name{ std::move(name) }
	{
		ensure(m_Mesh != nullptr, "Mesh pointer cannot be null.");
		ensure(!m_Name.empty(), "Shader name cannot be empty.");
	}
}