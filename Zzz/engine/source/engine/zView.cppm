#include "pch.h"
export module zView;

import IGAPI;
import result;
import IAppWin;
import zSize2D;
import IWinSurface;
import zViewFactory;
import zViewSettings;

using namespace zzz::platforms;

namespace zzz
{
	class engine;
}

namespace zzz
{
	export class zView final
	{
	public:
		zView() = delete;
		zView(const zView&) = delete;
		zView(zView&&) = delete;
		zView& operator=(const zView&) = delete;
		zView& operator=(zView&&) = delete;

		zView(std::shared_ptr<zViewSettings> _setting);

		~zView();

		void OnUpdate();
		void OnResize(const zSize2D<>& size);

	private:
		zViewFactory factory;
		std::shared_ptr<zViewSettings> settings;
		std::vector<std::shared_ptr<IGAPI>> gapiList;
		std::shared_ptr<IAppWin> appWin;
		std::shared_ptr<IWinSurface> winSurface;

		void Initialize();
	};

	export zView::zView(std::shared_ptr<zViewSettings> _setting) :
		settings{ _setting }
	{
		ensure(settings, ">>>>> [zView::zView()]. Settings cannot be null.");

		Initialize();
	}

	zView::~zView()
	{
		for (auto& gapi : gapiList)
		{
			if (gapi)
			{
				gapi->~IGAPI();
			}
		}

		gapiList.clear();
	}

	void zView::Initialize()
	{

	}

	void zView::OnUpdate()
	{

	}

	void zView::OnResize(const zSize2D<>& size)
	{
		winSurface->OnResize(size);
	}
}