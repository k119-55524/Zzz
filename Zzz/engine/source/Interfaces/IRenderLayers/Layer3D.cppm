
export module Layer3D;

import Scene;
import Size2D;
import Result;
import IRenderLayer;
import ScenesManager;

namespace zzz
{
	export interface Layer3D final : public IRenderLayer
	{
	public:
		Layer3D() = delete;
		Layer3D(const std::shared_ptr<ScenesManager> _scenesManager, eAspectType aspectPreset, Size2D<zF32>& size, zF32 minDepth, zF32 maxDepth) noexcept :
			IRenderLayer{ aspectPreset, size, minDepth, maxDepth },
			m_ScenesManager{ _scenesManager }
		{
			ensure(m_ScenesManager, ">>>>> [Layer3D::Layer3D()]. Scenes manager cannot be null.");
		}
		virtual ~Layer3D() = default;

		[[nodiscard]] Result<std::shared_ptr<Scene>> AddScene() noexcept;
		[[nodiscard]] std::shared_ptr<Scene> GetScene() const override { return m_Scene; };

	private:
		std::shared_ptr<Scene> m_Scene;
		const std::shared_ptr<ScenesManager> m_ScenesManager;
	};

	Result<std::shared_ptr<Scene>> Layer3D::AddScene() noexcept
	{
		auto resScene = m_ScenesManager->GetDefaultScene();
		if (!resScene)
			Unexpected(eResult::failure, L">>>>> [Layer3D::AddScene()]. Failed to create new scene.");

		// TODO: 1 сцена на слой, потом можно будет расширить
		m_Scene = resScene.value();

		return m_Scene;
	}
}