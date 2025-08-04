#include "pch.h"
export module DXAPI;

#if defined(_WIN64)
import IGAPI;
import result;

using namespace zzz::result;

export namespace zzz::platforms
{
	class DXAPI final : public IGAPI
	{
	public:
		DXAPI() = delete;
		DXAPI(DXAPI&) = delete;
		DXAPI(DXAPI&&) = delete;

		DXAPI& operator=(const DXAPI&) = delete;

		explicit DXAPI(const std::shared_ptr<ISuperWidget> _appWin);
		virtual ~DXAPI() override;

	protected:
		virtual zResult<> Initialize() override;

		void OnUpdate() override;
		void OnRender() override;
		void OnResize(const zSize2D<>& size) override;
	};

	DXAPI::DXAPI(const std::shared_ptr<ISuperWidget> appWin) :
		IGAPI(appWin, eGAPIType::DirectX12)//,
		//m_frameIndex{ 0 },
		//m_fenceEvent{ 0 },
		//m_fenceValue{ 0 },
		////m_aspectRatio{ 0 },
		//m_RtvDescrSize{ 0 },
		//m_DsvDescrSize{ 0 },
		//m_CbvSrvDescrSize{ 0 }
	{
	}

	DXAPI::~DXAPI()
	{
		//WaitForPreviousFrame();
		//CloseHandle(m_fenceEvent);
	}

	zResult<> DXAPI::Initialize()
	{
		return {};
	}

	void DXAPI::OnUpdate()
	{
	}

	void DXAPI::OnRender()
	{
	}

	void DXAPI::OnResize(const zSize2D<>& size)
	{
	}
}
#endif // _WIN64