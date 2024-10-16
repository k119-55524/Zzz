#pragma once

#include "zSerialize.h"
#include "../Helpers/zColor.h"
#include "../Helpers/zResult.h"
#include "../Helpers/zVersion.h"
#include "../Platforms/IO/IIO.h"

using namespace Zzz::Platforms;
using namespace std::filesystem;

namespace Zzz::Core
{
	enum e_TypeClear : zU32
	{
		eUnclear,
		Color
	};

	class Scene : public zSerialize
	{
	public:
		Scene() = delete;
		Scene(shared_ptr<IIO> _platformIO);

		zResult Load(const zStr& filename);

		inline void SetClearColor(const zColor& color, bool IsSetCCtype = true) noexcept
		{
			clearColor = color;

			if (IsSetCCtype)
				typeClear = e_TypeClear::Color;
		};
		inline const zColor& GetClearColor() const noexcept { return clearColor; };

		void Update();

#ifdef _SERVICES
	public:
#else
	private:
#endif
		zResult Save(const zStr& filename);

	private:
		shared_ptr<IIO> platformIO;

		zVersion version;
		e_TypeClear typeClear;
		zColor clearColor;

		void SerializeToBuffer(stringstream& buffer) const;
		zResult DeSerializeInBuffer(istringstream& buffer);
	};
}