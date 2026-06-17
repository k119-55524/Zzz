
#include "WinAndroid.h"
#include "../Platform.h"

using namespace zzz::engine;

WinAndroid::WinAndroid(const Platform& platform, const std::shared_ptr<Input> input, WindowCallbacks callbacks) :
	WindowBase(platform, input, std::move(callbacks))
{
}

WinAndroid::~WinAndroid()
{
}

std::expected<void, std::string> WinAndroid::Initialize(const std::string_view appName)
{
	m_Ctx = { this, m_Input.get() };
	android_app* app = m_Platform->GetNativeData().get();
	if (app)
	{
		app->userData = &m_Ctx;
	}

	return {};
}

void WinAndroid::ProcessAppCmd(int32_t cmd)
{
	switch (cmd)
	{
	case APP_CMD_INIT_WINDOW:
		DOut("APP_CMD_INIT_WINDOW.");
		// [Android] Р’С‹РґРµР»РµРЅР° РіСЂР°С„РёС‡РµСЃРєР°СЏ РїРѕРІРµСЂС…РЅРѕСЃС‚СЊ. РћРєРЅРѕ РіРѕС‚РѕРІРѕ Рє РѕС‚СЂРёСЃРѕРІРєРµ.
		// РџРµСЂРµРґР°РµРј С…СЌРЅРґР» РЅР°С‚РёРІРЅРѕРіРѕ РѕРєРЅР° РґР»СЏ РёРЅРёС†РёР°Р»РёР·Р°С†РёРё Swapchain.
		// TODO: VERIFY_AND_CALL(m_Callbacks.OnSurfaceCreated, app->window);
		break;
	case APP_CMD_TERM_WINDOW:
		DOut("APP_CMD_TERM_WINDOW.");
		// [Android] РћРєРЅРѕ (Рё РµРіРѕ РіСЂР°С„РёС‡РµСЃРєР°СЏ РїРѕРІРµСЂС…РЅРѕСЃС‚СЊ) СЃРєСЂС‹С‚Рѕ РёР»Рё СѓРЅРёС‡С‚РѕР¶Р°РµС‚СЃСЏ СЃРёСЃС‚РµРјРѕР№.
		// Р’С‹Р·С‹РІР°РµРј OnSurfaceDestroyed Р”Рћ СѓРЅРёС‡С‚РѕР¶РµРЅРёСЏ, С‡С‚РѕР±С‹ СѓР±РёС‚СЊ Swapchain.
		VERIFY_AND_CALL(m_Callbacks.OnSurfaceDestroyed);
		
		// Р•СЃР»Рё РїСЂРёР»РѕР¶РµРЅРёРµ РґРµР№СЃС‚РІРёС‚РµР»СЊРЅРѕ Р·Р°РІРµСЂС€Р°РµС‚СЃСЏ, РјРѕР¶РЅРѕ РІС‹Р·РІР°С‚СЊ OnClose.
		// Р’ Android СЌС‚Рѕ С‚Р°РєР¶Рµ РѕР·РЅР°С‡Р°РµС‚, С‡С‚Рѕ Activity Р·Р°РІРµСЂС€Р°РµС‚СЃСЏ.
		VERIFY_AND_CALL(m_Callbacks.OnClose);
		break;
	case APP_CMD_WINDOW_RESIZED:
		DOut("APP_CMD_WINDOW_RESIZED.");
		// [Android] РР·РјРµРЅРёР»СЃСЏ СЂР°Р·РјРµСЂ РѕРєРЅР° (РЅР°РїСЂРёРјРµСЂ, РёР·-Р·Р° СЃРєСЂС‹С‚РёСЏ СЃРёСЃС‚РµРјРЅРѕР№ РїР°РЅРµР»Рё РЅР°РІРёРіР°С†РёРё РёР»Рё РїРѕРІРѕСЂРѕС‚Р°).
		// TODO: РР·РІР»РµС‡СЊ РЅРѕРІС‹Рµ СЂР°Р·РјРµСЂС‹ Рё РїРµСЂРµРґР°С‚СЊ РІ OnResize.
		break;
	}
}

