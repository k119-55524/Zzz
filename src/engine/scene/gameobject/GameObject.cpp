#include "engine/scene/gameobject/GameObject.h"
#include "core/userscripts/base_script/Script.h"
#include <algorithm>

namespace zzz
{
	GameObject::GameObject(std::string name)
		: m_Guid{}
		, m_Name(std::move(name))
		, m_Transform(*this)
	{
	}

	GameObject::GameObject(::zzz::core::Guid guid, std::string name)
		: m_Guid(guid)
		, m_Name(std::move(name))
		, m_Transform(*this)
	{
	}

	void GameObject::SetParent(GameObject* newParent, bool keepWorldTransform) noexcept
	{
		if (m_Parent == newParent || newParent == this)
		{
			return;
		}

		// Запоминаем текущие мировые параметры, если требуется их сохранить
		::zzz::math::Vec3<zF32> oldWorldPos{};
		::zzz::math::Quat<zF32> oldWorldRot{};
		::zzz::math::Vec3<zF32> oldWorldScale{};
		if (keepWorldTransform)
		{
			oldWorldPos = m_Transform.GetWorldPosition();
			oldWorldRot = m_Transform.GetWorldRotation();
			oldWorldScale = m_Transform.GetWorldScale();
		}

		// Отвязываем от текущего родителя
		if (m_Parent != nullptr)
		{
			std::erase(m_Parent->m_Children, this);
		}

		m_Parent = newParent;

		// Привязываем к новому родителю
		if (m_Parent != nullptr)
		{
			m_Parent->m_Children.push_back(this);
		}

		// Пересчитываем локальные координаты относительно нового родителя
		if (keepWorldTransform)
		{
			m_Transform.SetWorldPosition(oldWorldPos);
			m_Transform.SetWorldRotation(oldWorldRot);
			if (m_Parent != nullptr)
			{
				const auto parentScale = m_Parent->GetTransform().GetWorldScale();
				m_Transform.SetLocalScale(::zzz::math::Vec3<zF32>{
					parentScale.x > 1e-6f ? oldWorldScale.x / parentScale.x : oldWorldScale.x,
					parentScale.y > 1e-6f ? oldWorldScale.y / parentScale.y : oldWorldScale.y,
					parentScale.z > 1e-6f ? oldWorldScale.z / parentScale.z : oldWorldScale.z
				});
			}
			else
			{
				m_Transform.SetLocalScale(oldWorldScale);
			}
		}

		m_Transform.SetDirty();
	}

	GameObject* GameObject::GetChild(size_t index) const noexcept
	{
		if (index < m_Children.size())
		{
			return m_Children[index];
		}
		return nullptr;
	}

	void GameObject::AddScript(std::shared_ptr<::zzz::core::Script> script)
	{
		if (script != nullptr)
		{
			m_Scripts.push_back(std::move(script));
		}
	}

	void GameObject::RemoveScript(const std::shared_ptr<::zzz::core::Script>& script)
	{
		std::erase(m_Scripts, script);
	}

	void GameObject::RemoveAllScripts()
	{
		m_Scripts.clear();
	}
}
