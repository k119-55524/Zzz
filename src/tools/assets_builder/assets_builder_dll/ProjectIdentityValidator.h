#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

namespace zzz::builder
{
	/**
	 * @enum GuidOwnerKind
	 * @brief Категория сущности-владельца GUID в проекте.
	 */
	enum class GuidOwnerKind : uint8_t
	{
		Project,
		Scene,
		View,
		Script,
		Mesh,
		Material,
		Shader,
		Texture,
		Audio,
		Prefab,
		Layer,
		Object
	};

	[[nodiscard]] std::string_view ToString(GuidOwnerKind kind) noexcept;

	/**
	 * @class ProjectIdentityValidator
	 * @brief Нативный валидатор единого глобального пространства GUID и ссылочной целостности (Single Source of Truth).
	 *
	 * Выполняет двухпроходную проверку:
	 *   - Проход 1: регистрация всех владельцев GUID (.meta, layers[].guid, objects[].guid с рекурсией) и проверка уникальности.
	 *   - Проход 2: типизированная проверка всех GUID-ссылок (project.json, платформенные конфиги, .zview, .zscene и все children).
	 */
	class ProjectIdentityValidator
	{
	public:
		/**
		 * @brief Проверяет идентичность и связи проекта.
		 * @param projectDir Каталог проекта.
		 * @param errorBuffer Выходной буфер для текста ошибки (caller-owned).
		 * @param bufferSize Размер выходного буфера.
		 * @return true при отсутствии ошибок; false при ошибке (текст ошибки пишется в errorBuffer).
		 */
		[[nodiscard]] static bool Validate(
			const std::filesystem::path& projectDir,
			char* errorBuffer,
			uint32_t bufferSize,
			const std::string& platformConfigFile = "") noexcept;
	};
}
