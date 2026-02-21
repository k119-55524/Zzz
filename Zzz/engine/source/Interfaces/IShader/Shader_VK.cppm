
export module Shader_VK;

#if defined(ZRENDER_API_VULKAN)

import IGAPI;
import Result;
import IShader;
import IMeshGPU;

namespace zzz::vk
{
	export class Shader_VK final : public IShader
	{
	public:
		Shader_VK(std::shared_ptr<IGAPI> igapi, const std::shared_ptr<IMeshGPU> mesh, std::wstring&& name);
		virtual ~Shader_VK() override = default;

		Result<> InitializeByText(std::string&& srcVS, std::string&& srcPS) override;
	};

	Shader_VK::Shader_VK(const std::shared_ptr<IGAPI> gapi, const std::shared_ptr<IMeshGPU> mesh, std::wstring&& name) :
		IShader(gapi, mesh, std::move(name))
	{
	}

	Result<> Shader_VK::InitializeByText(std::string&&, std::string&&)
	{
		return {};
	}
}
#endif // ZRENDER_API_VULKAN