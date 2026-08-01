#pragma once

#include <string>
#include <vector>
#include <memory>
#include <common/Common.h>

namespace zzz::script
{
	class Script;
}

namespace zzz
{
	class GameObject : public std::enable_shared_from_this<GameObject>
	{
	public:
		explicit GameObject(std::string name) : m_Name(std::move(name)) {}
		~GameObject() = default;

		const std::string& GetName() const { return m_Name; }
		void SetName(std::string name) { m_Name = std::move(name); }

		void AddScript(std::shared_ptr<script::Script> script) {
			m_Scripts.push_back(script);
		}

		void RemoveScript(const std::shared_ptr<script::Script>& script)
		{
			std::erase(m_Scripts, script);
		}

		void RemoveAllScripts() {
			m_Scripts.clear();
		}

		const std::vector<std::shared_ptr<script::Script>>& GetScripts() const
		{
			return m_Scripts;
		}

	private:
		std::string m_Name;
		std::vector<std::shared_ptr<script::Script>> m_Scripts;
	};
}
