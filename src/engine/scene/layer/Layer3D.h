#pragma once

#include "engine/scene/layer/ILayer.h"
#include "engine/scene/ObjectWorld.h"
#include "engine/scene/storage/DefaultSceneStorage.h"
#include <memory>

namespace zzz
{
	/**
	 * @class Layer3D
	 * @brief Слой 3D игрового мира (GameObject, Transform, меши, материалы, освещение).
	 */
	class Layer3D final : public ILayer
	{
	public:
		explicit Layer3D(std::string name = "Layer3D");
		~Layer3D() override = default;

		Z_NO_COPY_MOVE(Layer3D);

		[[nodiscard]] const std::string& GetName() const noexcept override { return m_Name; }
		[[nodiscard]] eLayerType GetType() const noexcept override { return eLayerType::Layer3D; }

		[[nodiscard]] bool IsVisible() const noexcept override { return m_IsVisible; }
		void SetVisible(bool visible) noexcept override { m_IsVisible = visible; }

		[[nodiscard]] bool IsEnabled() const noexcept override { return m_IsEnabled; }
		void SetEnabled(bool enabled) noexcept override { m_IsEnabled = enabled; }

		void Update(float dt) override;

		[[nodiscard]] ISceneStorage* GetStorage() noexcept override { return m_Storage.get(); }
		[[nodiscard]] ObjectWorld& GetObjectWorld() noexcept { return m_ObjectWorld; }
		[[nodiscard]] const ObjectWorld& GetObjectWorld() const noexcept { return m_ObjectWorld; }

	private:
		std::string m_Name;
		bool m_IsVisible{ true };
		bool m_IsEnabled{ true };

		std::unique_ptr<ISceneStorage> m_Storage;
		ObjectWorld m_ObjectWorld;
	};
}
