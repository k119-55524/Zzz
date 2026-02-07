
export module SceneEntityFactory;

import Input;
import Result;
import Material;
import StrConvert;
import SceneEntity;
import GPUResManager;

using namespace zzz::input;

namespace zzz
{
	export class SceneEntityFactory final
	{
		Z_NO_COPY_MOVE(SceneEntityFactory);

	public:
		explicit SceneEntityFactory(const std::shared_ptr<GPUResManager> resGPU);
		~SceneEntityFactory() = default;

		Result<std::shared_ptr<SceneEntity>> GetColorBox(const std::shared_ptr<Input> input) noexcept;

	private:
		const std::shared_ptr<GPUResManager> m_ResGPU;
	};

	SceneEntityFactory::SceneEntityFactory(const std::shared_ptr<GPUResManager> resGPU) :
		m_ResGPU{ resGPU }
	{
		ensure(m_ResGPU, ">>>>> [SceneEntityFactory::SceneEntityFactory()]. Resource system GPU cannot be null.");
	}

	Result<std::shared_ptr<SceneEntity>> SceneEntityFactory::GetColorBox(const std::shared_ptr<Input> input) noexcept
	{
		std::shared_ptr<SceneEntity> entity;

		try
		{
			Result<std::shared_ptr<IMeshGPU>> meshGPU = m_ResGPU->GetGenericMesh(MeshType::Box);
			if (!meshGPU)
				return meshGPU.error();

			Result<std::shared_ptr<Material>> material = m_ResGPU->GetGenericMaterial(meshGPU.value());
			if (!material)
				return material.error();

			entity = safe_make_shared<SceneEntity>(input, meshGPU.value(), material.value());
		}
		catch (const std::exception& e)
		{
			// TODO: в будущем обработать удаление созданных ресурсов(внутри try блока)
			return Unexpected(eResult::exception, string_to_wstring(e.what()).value_or(L"Unknown exception occurred."));
		}

		return entity;
	}
}