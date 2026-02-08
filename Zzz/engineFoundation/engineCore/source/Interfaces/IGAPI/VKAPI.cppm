
export module VKAPI;

#if defined(ZRENDER_API_VULKAN)

import IGAPI;
import Result;

namespace zzz::vk
{
	export class VKAPI final : public IGAPI
	{
		Z_NO_COPY_MOVE(VKAPI);

	public:
		explicit VKAPI();
		virtual ~VKAPI() override;

	protected:
		[[nodiscard]] Result<> Init() override;
		void WaitForGpu() override;

		void SubmitCommandLists() override;
		void BeginRender() override;
		void EndRender() override;

	private:
	};

	VKAPI::VKAPI() :
		IGAPI(eGAPIType::Vulkan)
	{
	}

	VKAPI::~VKAPI()
	{
	}

	[[nodiscard]] Result<> VKAPI::Init()
	{
		return {};
	}

	void VKAPI::WaitForGpu()
	{
	}

	void VKAPI::SubmitCommandLists()
	{
	}

	void VKAPI::BeginRender()
	{
	}

	void VKAPI::EndRender()
	{
	}
}
#endif // ZRENDER_API_VULKAN