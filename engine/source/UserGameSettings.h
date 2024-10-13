#pragma once

#include "Structs.h"
#include "Platforms/IO/IIO.h"
#include "Helpers/zResult.h"
#include "SceneEntities/zSerialize.h"

using namespace Zzz::Platforms;

namespace Zzz
{
	// Класс работает с файлом настроек игры.
	// Файл предполагается использовать как хранящий текущие настройки игры пользователем
	class UserGameSettings : protected zSerialize
	{
	public:
		UserGameSettings() = delete;
		UserGameSettings(shared_ptr<IIO> _platformIO);

		zResult Save();
		zResult Load();

		inline const zSize& GetSetupAppWinSize() const noexcept { return winSize; };
		inline const zStr& GetMSWinClassName() const noexcept { return msWinClassName; };
		inline const zU64& GetMSWinIcoID() const noexcept { return msWinIcoID; };
		inline const zStr& GetWinCaption() const noexcept { return winCaption; };

		void SetDefault() noexcept;

	private:
		shared_ptr<IIO> platformIO;

		void SerializeToBuffer(stringstream& buffer) const;
		zResult DeSerializeInBuffer(istringstream& buffer);
		zResult TestParameters() const;

		zVersion version;
		zSize winSize;
		bool isFullScreen;
		zStr startScene;

		zStr msWinClassName;
		zU64 msWinIcoID;
		zStr winCaption;
	};
}
