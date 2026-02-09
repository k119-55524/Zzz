
export module IGPUUpload;

import Result;
import QueueArray;
import ThreadPool;
import GPUUploadCallbacks;

using namespace zzz::templates;

namespace zzz
{
	export class IGPUUpload
	{
		Z_NO_COPY_MOVE(IGPUUpload);

	public:
		IGPUUpload();
		virtual ~IGPUUpload() = default;

		virtual void TransferResourceToGPU() = 0;

		void AddTransferResource(FillCallback OnFill, PreparedCallback OnPrepared, CompleteCallback OnComplete);
		inline bool HasResourcesToUpload() const noexcept
		{
			std::lock_guard<std::mutex> lock(hasMutex);
			return m_TransferCallbacks[0].Size() > 0 || m_TransferCallbacks[1].Size() > 0;
		}

	protected:
		mutable std::mutex hasMutex;

		zU8 m_TransferIndex;
		QueueArray<std::shared_ptr<GPUUploadCB>> m_TransferCallbacks[2];
	};

	IGPUUpload::IGPUUpload() :
		m_TransferIndex{ 0 },
		m_TransferCallbacks{ QueueArray<std::shared_ptr<GPUUploadCB>>(100), QueueArray<std::shared_ptr<GPUUploadCB>>(100) }
	{
	}

	// Накапливаем список для отправки
	void IGPUUpload::AddTransferResource(FillCallback OnFill, PreparedCallback OnPrepared, CompleteCallback OnComplete)
	{
		std::lock_guard<std::mutex> lock(hasMutex);
		std::shared_ptr<GPUUploadCB> callbacks = safe_make_shared<GPUUploadCB>(OnFill, OnPrepared, OnComplete);
		m_TransferCallbacks[1 - m_TransferIndex].PushBack(callbacks);
	}
}