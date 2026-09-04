# Этап 08: Бинарные форматы ресурсов (`MeshData`, `TextureData`, `MaterialData`, `ShaderData`) и упаковка в `PackagePacker`

## 1. Контекст и цели этапа
- **Номер пункта:** **Пункт 8** (Уровень 2: GAPI-ресурсы, содержимое куба и сквозной рендер).
- **Цель:** Заложить основу ассетов движка перед созданием `ResourceManager`. Спроектировать канонические сериализуемые структуры данных для ключевых типов ресурсов (`MeshData`, `TextureData`, `MaterialData`, `ShaderData`) в `src/core/io/package/`. Расширить `PackagePacker` для упаковки этих форматов из исходников папки `Assets/` в бинарный архив `package.dat`. Создать минимальные исходные ассеты 3D-куба, чтобы на следующем шаге `ResourceManager` загружал настоящие файлы из `package.dat` без моков и процедурных заглушек.
- **Статус:** `⏳ Не начато`.
- **Зависимости:** `src/core/serialize/Serializer.h`, `src/core/enums/eResourceType.h`, `src/core/enums/ePackage.h`, `src/core/io/package/PackageEntry.h`, `src/tools/assets_builder/assets_builder_dll/PackagePacker.h`.

---

## 2. Архитектурные принципы

1. **Честный конвейер ассетов с первого шага (No Procedural Hacks):**
   - Никакого «кустарного» процедурного создания вершин в коде движка.
   - Меш куба, его текстура, материал и шейдер с самого начала являются полноправными ассетами в `Assets/`, упаковываются утилитой `PackagePacker` в `package.dat` и считываются движком через `PackageManager`.
2. **Единый бинарный протокол (`ISerializable`):**
   - Все структуры ресурсов наследуют `ISerializable` и используют наш кроссплатформенный `Serializer` (`core/serialize/Serializer.h`), гарантирующий строгий порядок байт (Little-Endian) и платформонезависимость.
3. **Мета-файлы с GUID (`<filename>.meta`):**
   - У каждого исходного файла в `Assets/` есть парный `.meta` файл, содержащий уникальный 128-битный `Guid`:
     ```json
     {
       "guid": "018f3a2b-7c1e-7d8a-9e2f-4a5b6c7d8e9f"
     }
     ```
   - `PackagePacker` считывает этот `Guid` и прописывает его в заголовок `PackageEntry`.
4. **Связи через GUID в материале:**
   - Исходный файл материала `cube.zmat` хранится как читаемый JSON:
     ```json
     {
       "shaderGuid": "00000000-0000-0000-0000-000000000010",
       "textures": {
         "MainTex": "00000000-0000-0000-0000-000000000011"
       },
       "properties": {
         "tintColor": [1.0, 1.0, 1.0, 1.0],
         "roughness": 0.5
       }
     }
     ```
   - `PackagePacker` парсит его и упаковывает в бинарный `MaterialData`.

---

## 3. Архитектурная спецификация

### 3.1. Структура геометрии: `src/core/io/package/MeshData.h`

```cpp
#pragma once

#include <vector>
#include <cstddef>
#include "core/serialize/Serializer.h"
#include "core/enums/eIndexFormat.h"

namespace zzz::core
{
	/**
	 * @class MeshData
	 * @brief Сериализуемый бинарный контейнер геометрии меша в package.dat.
	 */
	class MeshData final : public ISerializable
	{
	public:
		MeshData() = default;
		MeshData(
			uint32_t vertexCount,
			uint32_t vertexStride,
			std::vector<std::byte> vertexData,
			uint32_t indexCount,
			eIndexFormat indexFormat,
			std::vector<std::byte> indexData);

		[[nodiscard]] uint32_t GetVertexCount() const noexcept { return m_VertexCount; }
		[[nodiscard]] uint32_t GetVertexStride() const noexcept { return m_VertexStride; }
		[[nodiscard]] const std::vector<std::byte>& GetVertexData() const noexcept { return m_VertexData; }

		[[nodiscard]] uint32_t GetIndexCount() const noexcept { return m_IndexCount; }
		[[nodiscard]] eIndexFormat GetIndexFormat() const noexcept { return m_IndexFormat; }
		[[nodiscard]] const std::vector<std::byte>& GetIndexData() const noexcept { return m_IndexData; }

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override;

	private:
		uint32_t m_VertexCount{ 0 };
		uint32_t m_VertexStride{ 0 };
		std::vector<std::byte> m_VertexData;

		uint32_t m_IndexCount{ 0 };
		eIndexFormat m_IndexFormat{ eIndexFormat::Index16 };
		std::vector<std::byte> m_IndexData;
	};
}
```

---

### 3.2. Структура текстуры: `src/core/io/package/TextureData.h`

```cpp
#pragma once

#include <vector>
#include <cstddef>
#include "core/serialize/Serializer.h"
#include "core/enums/ePixelFormat.h"

namespace zzz::core
{
	/**
	 * @class TextureData
	 * @brief Сериализуемый бинарный контейнер 2D-текстуры в package.dat.
	 */
	class TextureData final : public ISerializable
	{
	public:
		TextureData() = default;
		TextureData(
			uint32_t width,
			uint32_t height,
			uint32_t mipLevels,
			ePixelFormat format,
			std::vector<std::byte> pixelData);

		[[nodiscard]] uint32_t GetWidth() const noexcept { return m_Width; }
		[[nodiscard]] uint32_t GetHeight() const noexcept { return m_Height; }
		[[nodiscard]] uint32_t GetMipLevels() const noexcept { return m_MipLevels; }
		[[nodiscard]] ePixelFormat GetFormat() const noexcept { return m_Format; }
		[[nodiscard]] const std::vector<std::byte>& GetPixelData() const noexcept { return m_PixelData; }

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override;

	private:
		uint32_t m_Width{ 0 };
		uint32_t m_Height{ 0 };
		uint32_t m_MipLevels{ 1 };
		ePixelFormat m_Format{ ePixelFormat::R8G8B8A8_UNORM };
		std::vector<std::byte> m_PixelData;
	};
}
```

---

### 3.3. Структура материала: `src/core/io/package/MaterialData.h`

```cpp
#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include "core/serialize/Serializer.h"
#include "core/utils/Guid.h"

namespace zzz::core
{
	/**
	 * @class MaterialData
	 * @brief Сериализуемый бинарный контейнер свойств материала в package.dat.
	 */
	class MaterialData final : public ISerializable
	{
	public:
		MaterialData() = default;
		MaterialData(
			Guid shaderGuid,
			std::unordered_map<std::string, Guid> textureGuids,
			std::vector<std::byte> properties);

		[[nodiscard]] const Guid& GetShaderGuid() const noexcept { return m_ShaderGuid; }
		[[nodiscard]] const std::unordered_map<std::string, Guid>& GetTextureGuids() const noexcept { return m_TextureGuids; }
		[[nodiscard]] const std::vector<std::byte>& GetProperties() const noexcept { return m_Properties; }

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override;

	private:
		Guid m_ShaderGuid{};
		std::unordered_map<std::string, Guid> m_TextureGuids;
		std::vector<std::byte> m_Properties;
	};
}
```

---

### 3.4. Структура шейдера: `src/core/io/package/ShaderData.h`

```cpp
#pragma once

#include <vector>
#include <cstddef>
#include "core/serialize/Serializer.h"
#include "core/enums/eGAPIType.h"

namespace zzz::core
{
	/**
	 * @class ShaderData
	 * @brief Сериализуемый бинарный контейнер скомпилированного байткода шейдера.
	 */
	class ShaderData final : public ISerializable
	{
	public:
		ShaderData() = default;
		ShaderData(
			eGAPIType gapiType,
			std::vector<std::byte> vertexShaderBytecode,
			std::vector<std::byte> pixelShaderBytecode);

		[[nodiscard]] eGAPIType GetGAPIType() const noexcept { return m_GAPIType; }
		[[nodiscard]] const std::vector<std::byte>& GetVertexShaderBytecode() const noexcept { return m_VertexShaderBytecode; }
		[[nodiscard]] const std::vector<std::byte>& GetPixelShaderBytecode() const noexcept { return m_PixelShaderBytecode; }

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override;

	private:
		eGAPIType m_GAPIType{ eGAPIType::DirectX12 };
		std::vector<std::byte> m_VertexShaderBytecode;
		std::vector<std::byte> m_PixelShaderBytecode;
	};
}
```

---

### 3.5. Расширение `PackagePacker` (`src/tools/assets_builder/assets_builder_dll/`)

В функцию сканирования ассетов в `PackagePacker.cpp` добавляются обработчики расширений:
- `.zmsh` $\to$ парсинг в `MeshData`
- `.ztx` / `.png` $\to$ чтение пикселей в `TextureData`
- `.zmat` $\to$ чтение JSON в `MaterialData`
- `.zshd` / `.hlsl` $\to$ упаковка в `ShaderData`

Для каждого найденного файла читается парный `.meta` (для извлечения `Guid`), после чего файл сериализуется через `Serializer` и упаковывается в `package.dat` с флагом `ePackage::BinaryAsset` и гранулярным `eResourceType`.

---

### 3.6. Минимальные тестовые ассеты 3D-куба

В директории проекта `Assets/` создаются исходные файлы:
1. `cube.zmsh` + `cube.zmsh.meta`:
   - 24 вершины (6 граней $\times$ 4 вершины с позицией, нормалью, UV) и 36 индексов (12 треугольников).
2. `default.ztx` + `default.ztx.meta`:
   - Минимальная RGBA8 текстура шахматки 4x4.
3. `cube.zmat` + `cube.zmat.meta`:
   - Описание материала с привязкой `shaderGuid` и `MainTex` $\to$ `default.ztx`.
4. `BasicTextured.zshd` + `BasicTextured.zshd.meta`:
   - Тестовый блок байткода шейдера.

---

## 4. План верификации

1. **Компиляция под MSVC x64 + Ninja:**
   - Сборка целей `core`, `assets_builder_dll`, `EngineTests`, `game_win` без ошибок.
2. **Сборка `package.dat`:**
   - Запуск `PackagePacker::PackProject()`, успешная генерация `package.dat` со всеми 4 типами ассетов куба.
3. **Проверка в `PackageManager`:**
   - При запуске `game_win.exe` `PackageManager` находит записи `cube`, `default`, `cube.zmat`, `BasicTextured` по их `Guid` и успешно их десериализует.
   - Код выхода 0.

---

## 5. Чек-лист Definition of Done (DoD)

- [ ] Создать `src/core/io/package/MeshData.h` и `MeshData.cpp`
- [ ] Создать `src/core/io/package/TextureData.h` и `TextureData.cpp`
- [ ] Создать `src/core/io/package/MaterialData.h` и `MaterialData.cpp`
- [ ] Создать `src/core/io/package/ShaderData.h` и `ShaderData.cpp`
- [ ] Зарегистрировать новые файлы в `src/core/CMakeLists.txt`
- [ ] Расширить `PackagePacker.cpp` для упаковки `.zmsh`, `.ztx`/`.png`, `.zmat`, `.zshd`
- [ ] Создать исходные ассеты куба с `.meta` файлами в проекте
- [ ] Собрать `package.dat` и проверить десериализацию записей через `PackageManager`
- [ ] Собрать и запустить `game_win.exe` (чистый запуск, чтение пакета и завершение с кодом 0)
- [ ] Запросить утверждение у пользователя
- [ ] Зафиксировать Git-коммит: `feat(package): completed stage 08 - binary resource formats and package packing`
- [ ] Обновить статус Пункта 8 в `general_plan.md` на `✅ Выполнено`
