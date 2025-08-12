#include "pch.h"
export module ISurfaceAppWin;

import IGAPI;
import result;
import IAppWin;
import zSize2D;
import zViewSettings;

using namespace zzz::platforms;

export namespace zzz
{
	export class ISurfaceAppWin abstract
	{
	public:
		ISurfaceAppWin() = delete;
		ISurfaceAppWin(const ISurfaceAppWin&) = delete;
		ISurfaceAppWin(ISurfaceAppWin&&) = delete;
		ISurfaceAppWin& operator=(const ISurfaceAppWin&) = delete;
		ISurfaceAppWin& operator=(ISurfaceAppWin&&) = delete;

		explicit ISurfaceAppWin(
			std::shared_ptr<zViewSettings> _settings,
			std::shared_ptr<IAppWin> _iAppWin,
			std::shared_ptr<IGAPI> _iGAPI);

		virtual ~ISurfaceAppWin() = default;

		[[nodiscard]] virtual result<> Initialize() = 0;
		[[nodiscard]] virtual void OnRender() = 0;
		virtual void OnResize(const zSize2D<>& size) = 0;

	protected:
		std::shared_ptr<zViewSettings> settings;
		std::shared_ptr<IAppWin> iAppWin;
		std::shared_ptr<IGAPI> iGAPI;
	};

	ISurfaceAppWin::ISurfaceAppWin(
		std::shared_ptr<zViewSettings> _settings,
		std::shared_ptr<IAppWin> _iAppWin,
		std::shared_ptr<IGAPI> _iGAPI) :
		settings{ _settings },
		iAppWin{ _iAppWin },
		iGAPI{ _iGAPI }
	{
		ensure(settings, ">>>>> [ISurfaceAppWin::ISurfaceAppWin()]. Settings cannot be null.");
		ensure(iAppWin, ">>>>> [ISurfaceAppWin::ISurfaceAppWin()]. Application window cannot be null.");
		ensure(iGAPI, ">>>>> [ISurfaceAppWin::ISurfaceAppWin()]. GAPI cannot be null.");
	}
}