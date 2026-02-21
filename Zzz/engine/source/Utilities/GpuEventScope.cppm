
export module GpuEventScope;

import IGAPI;
import Ensure;
import StrConvert;

//import RootSignature;

#if defined(ZRENDER_API_D3D12)
import DXAPI;
using namespace zzz::dx;
#elif defined(ZRENDER_API_VULKAN)
import VKAPI;
using namespace zzz::vk;
#endif

namespace zzz
{
	template<class T> struct always_false : std::false_type {};

	export class GpuEventScope
	{
	public:
#if defined(_DEBUG)
		GpuEventScope(std::shared_ptr<IGAPI> igapi, const char* name, const std::array<float, 4>& color = { 1,1,1,1 })
		{
			init(igapi, name, color);
		}

		~GpuEventScope()
		{
			shutdown();
		}
#else
		GpuEventScope(std::shared_ptr<IGAPI>, const char*, const std::array<float, 4> & = { 1,1,1,1 }) {}
#endif

	private:
#if defined(ZRENDER_API_D3D12) // ==== Direct3D12 / PIX ====
		ComPtr<ID3D12GraphicsCommandList> m_cmdList = nullptr;

		void init(std::shared_ptr<IGAPI> igapi, const char* name, const std::array<float, 4>&)
		{
			auto m_DXAPI = std::dynamic_pointer_cast<DXAPI>(igapi);
			ensure(m_DXAPI);

			m_cmdList = m_DXAPI->GetCommandListUpdate();
			ensure(m_cmdList);

			auto wname = string_to_wstring(name);
			if (!wname)
				throw_runtime_error("String_to_wstring failed.");

			//PIXBeginEvent(m_cmdList.Get(), 0, wname.value().c_str());
		}

		void shutdown()
		{
			//if (m_cmdList)
			//	PIXEndEvent(m_cmdList.Get());
		}

#elif defined(ZRENDER_API_VULKAN) // ==== Vulkan ====
		VkCommandBuffer m_cmdBuf{};

		void init(std::shared_ptr<IGAPI> igapi, const char* name, const std::array<float, 4>& color)
		{
			std::shared_ptr<VKAPI> m_VKAPI = std::dynamic_pointer_cast<VKAPI>(igapi);
			ensure(m_VKAPI, "Failed to cast IGAPI to VKAPI.");

			//m_cmdBuf = m_VKAPI->GetCommandBuffer();
			//if (!m_cmdBuf) {
			//	throw_runtime_error(L"Invalid command buffer.");
			//}

			VkDebugUtilsLabelEXT labelInfo{};
			labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
			labelInfo.pLabelName = name;
			std::copy(color.begin(), color.end(), labelInfo.color);

			//if (vkCmdBeginDebugUtilsLabelEXT) {
			//	vkCmdBeginDebugUtilsLabelEXT(m_cmdBuf, &labelInfo);
			//}
		}

		void shutdown()
		{
			//if (vkCmdEndDebugUtilsLabelEXT && m_cmdBuf) {
			//	vkCmdEndDebugUtilsLabelEXT(m_cmdBuf);
			//}
		}

#elif defined(ZRENDER_API_METAL) // ==== Metal ====
		id<MTLCommandBuffer> m_cmdBuf = nil;

		void init(std::shared_ptr<IGAPI> igapi, const char* name, const std::array<float, 4>&)
		{
			auto m_MTLAPI = std::dynamic_pointer_cast<MTLAPI>(igapi);
			ensure(m_MTLAPI, L"Failed to cast IGAPI to MTLAPI.");

			m_cmdBuf = m_MTLAPI->GetCommandBuffer();
			if (!m_cmdBuf) {
				throw_runtime_error(L"Invalid command buffer.");
			}

			NSString* nsName = [NSString stringWithUTF8String : name];
			[m_cmdBuf pushDebugGroup : nsName] ;
		}

		void shutdown()
		{
			if (m_cmdBuf) {
				[m_cmdBuf popDebugGroup] ;
			}
		}

#else
		template<typename T>
		void init(T, const char*, const std::array<float, 4>&)
		{
			static_assert(always_false<T>::value, "GpuEventScope: unsupported graphics API");
		}

		void shutdown() {}
#endif
	};
}