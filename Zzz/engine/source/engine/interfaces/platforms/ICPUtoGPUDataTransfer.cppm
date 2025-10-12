#include "pch.h"
export module ICPUtoGPUDataTransfer;

import result;
import QueueArray;
import ThreadPool;
import BaseCPUtoGPUDataTransfer;

using namespace zzz::templates;
using namespace zzz::platforms;

export namespace zzz
{
	export class ICPUtoGPUDataTransfer
	{
	public:
		ICPUtoGPUDataTransfer();
		ICPUtoGPUDataTransfer(ICPUtoGPUDataTransfer&) = delete;
		ICPUtoGPUDataTransfer(ICPUtoGPUDataTransfer&&) = delete;
		ICPUtoGPUDataTransfer& operator=(const ICPUtoGPUDataTransfer&) = delete;
		ICPUtoGPUDataTransfer& operator=(ICPUtoGPUDataTransfer&&) = delete;

		virtual void TransferResourceToGPU() = 0;

		virtual ~ICPUtoGPUDataTransfer() = default;

		void AddTransferResource(FillCallback fillCallback, PreparedCallback preparedCallback, CompleteCallback completeCallback);
		inline bool HasResourcesToUpload() const noexcept
		{
			std::lock_guard<std::mutex> lock(hasMutex);
			return m_TransferCallbacks[0].Size() > 0 || m_TransferCallbacks[1].Size() > 0;
		}

	protected:
		mutable std::mutex hasMutex;

		zU8 m_TransferIndex;
		QueueArray<std::shared_ptr<sInTransfersCallbacks>> m_TransferCallbacks[2];
	};

	ICPUtoGPUDataTransfer::ICPUtoGPUDataTransfer() :
		m_TransferIndex{ 0 },
		m_TransferCallbacks{ QueueArray<std::shared_ptr<sInTransfersCallbacks>>(100), QueueArray<std::shared_ptr<sInTransfersCallbacks>>(100) }
	{
	}
	
	// Накапливаем список для отправки
	void ICPUtoGPUDataTransfer::AddTransferResource(FillCallback fillCallback, PreparedCallback preparedCallback, CompleteCallback completeCallback)
	{
		std::lock_guard<std::mutex> lock(hasMutex);
		std::shared_ptr<sInTransfersCallbacks> callbacks = safe_make_shared<sInTransfersCallbacks>(fillCallback, preparedCallback, completeCallback);
		m_TransferCallbacks[1 - m_TransferIndex].PushBack(callbacks);
	}
}