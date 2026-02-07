
export module Layer3D;

import Input;
import Scene;
import Size2D;
import Result;
import StrConvert;
import IRenderLayer;
import SceneEntityFactory;

using namespace zzz::input;

namespace zzz
{
	export interface Layer3D final : public IRenderLayer
	{
	public:
		Layer3D() = delete;
		virtual ~Layer3D() = default;
		Layer3D(
			const std::shared_ptr<Input> input,
			const std::shared_ptr<SceneEntityFactory> entityFactory,
			eAspectType aspectPreset,
			Size2D<zF32>& size,
			zF32 minDepth,
			zF32 maxDepth) noexcept :
			IRenderLayer{ input, entityFactory, aspectPreset, size, minDepth, maxDepth }
		{
		}

		[[nodiscard]] Result<std::shared_ptr<Scene>> AddScene() noexcept;
	};

	Result<std::shared_ptr<Scene>> Layer3D::AddScene() noexcept
	{
		try
		{
			Result<std::shared_ptr<Scene>> scene = safe_make_shared<Scene>(m_Input, m_EntityFactory);
			if (!scene)
				return scene.error();

			// TODO: 1 сцена на слой, потом можно будет расширить
			m_Scene = scene.value();
		}
		catch (const std::exception& e)
		{
			return Unexpected(eResult::exception, std::format(L">>>>> [Layer3D::AddScene()]. Exception: {}.", string_to_wstring(e.what()).value_or(L"Unknown exception")));
		}

		return m_Scene;
	}
}