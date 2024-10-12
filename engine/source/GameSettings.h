#pragma once

#include "Platforms/IO/IIO.h"
#include "Helpers/zResult.h"
#include "SceneEntities/zSerialize.h"

using namespace Zzz::Platforms;

namespace Zzz
{
	class GameSettings : zSerialize
	{
	public:
		GameSettings() = delete;
		GameSettings(shared_ptr<IIO> _platformIO);

		zResult Save();
		zResult Load();

	private:
		shared_ptr<IIO> platformIO;

		void SerializeToBuffer(stringstream& buffer) const;
		zResult DeSerializeInBuffer(istringstream& buffer);

		void SetDefault() const noexcept;
	};
}
