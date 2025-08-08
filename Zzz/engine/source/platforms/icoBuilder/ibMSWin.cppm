#include "pch.h"
export module ibMSWin;

import result;
import IIcoBuilder;

using namespace zzz::result;

#if defined(_WIN64)
export namespace zzz::icoBuilder
{
#pragma comment(lib, "windowscodecs.lib")

	class ibMSWin : public IIcoBuilder<HICON>
	{
	public:
		ibMSWin();
		~ibMSWin();

		zResult<HICON> LoadIco(const std::wstring& filePath, int size) override;

	private:
		void CleanupResources();

		IWICImagingFactory* factory;
		IWICBitmapDecoder* decoder;
		IWICBitmapFrameDecode* frame;
		IWICBitmapScaler* scaler;
		IWICFormatConverter* converter;
		HBITMAP colorBitmap;
		HBITMAP maskBitmap;
		HDC hdc;
	};

	ibMSWin::ibMSWin() :
		factory(nullptr),
		decoder(nullptr),
		frame(nullptr),
		scaler(nullptr),
		converter(nullptr),
		colorBitmap(nullptr),
		maskBitmap(nullptr),
		hdc(nullptr)
	{
	}

	ibMSWin::~ibMSWin()
	{
		CleanupResources();
	}

	void ibMSWin::CleanupResources()
	{
		if (maskBitmap)
		{
			DeleteObject(maskBitmap);
			maskBitmap = nullptr;
		}

		if (colorBitmap)
		{
			DeleteObject(colorBitmap);
			colorBitmap = nullptr;
		}

		SafeRelease(converter);
		SafeRelease(scaler);
		SafeRelease(frame);
		SafeRelease(decoder);
		SafeRelease(factory);

		if (hdc)
		{
			ReleaseDC(nullptr, hdc);
			hdc = nullptr;
		}
	}

	zResult<HICON> ibMSWin::LoadIco(const std::wstring& filePath, int iconSize)
	{
		HICON hIcon = nullptr;
		bool comInitialized = false;

		HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
		if (SUCCEEDED(hr))
		{
			comInitialized = true;
		}
		else if (hr != RPC_E_CHANGED_MODE)
		{
			return Unexpected(eResult::failure, L">>>>> [ibMSWin::LoadIco]. Ошибка инициализации COM.");
		}

		if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr,
			CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (void**)&factory)))
		{
			if (comInitialized) CoUninitialize();
			return Unexpected(eResult::failure, L">>>>> [ibMSWin::LoadIco]. Не удалось создать экземпляр WIC Imaging Factory.");
		}

		if (FAILED(factory->CreateDecoderFromFilename(filePath.c_str(), nullptr,
			GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder)))
		{
			CleanupResources();
			if (comInitialized) CoUninitialize();
			return Unexpected(eResult::failure, L">>>>> [ibMSWin::LoadIco]. Не удалось создать WIC Bitmap Decoder.");
		}

		if (FAILED(decoder->GetFrame(0, &frame)))
		{
			CleanupResources();
			if (comInitialized) CoUninitialize();
			return Unexpected(eResult::failure, L">>>>> [ibMSWin::LoadIco]. Не удалось получить кадр из WIC Bitmap Decoder.");
		}

		UINT srcWidth = 0, srcHeight = 0;
		if (FAILED(frame->GetSize(&srcWidth, &srcHeight)))
		{
			CleanupResources();
			if (comInitialized) CoUninitialize();
			return Unexpected(eResult::failure, L">>>>> [ibMSWin::LoadIco]. Не удалось получить размеры изображения.");
		}

		if (FAILED(factory->CreateBitmapScaler(&scaler)))
		{
			CleanupResources();
			if (comInitialized) CoUninitialize();
			return Unexpected(eResult::failure, L">>>>> [ibMSWin::LoadIco]. Не удалось создать WIC Bitmap Scaler.");
		}

		if (FAILED(scaler->Initialize(frame, iconSize, iconSize, WICBitmapInterpolationModeFant)))
		{
			CleanupResources();
			if (comInitialized) CoUninitialize();
			return Unexpected(eResult::failure, L">>>>> [ibMSWin::LoadIco]. Не удалось инициализировать Bitmap Scaler.");
		}

		if (FAILED(factory->CreateFormatConverter(&converter)))
		{
			CleanupResources();
			if (comInitialized) CoUninitialize();
			return Unexpected(eResult::failure, L">>>>> [ibMSWin::LoadIco]. Не удалось создать WIC Format Converter.");
		}

		if (FAILED(converter->Initialize( scaler, GUID_WICPixelFormat32bppBGRA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom)))
		{
			CleanupResources();
			if (comInitialized) CoUninitialize();
			return Unexpected(eResult::failure, L">>>>> [ibMSWin::LoadIco]. Не удалось инициализировать WIC Format Converter.");
		}

		hdc = GetDC(nullptr);
		if (!hdc)
		{
			CleanupResources();
			if (comInitialized) CoUninitialize();
			return Unexpected(eResult::failure, L">>>>> [ibMSWin::LoadIco]. Не удалось получить контекст устройства.");
		}

		BITMAPV5HEADER bi = {};
		bi.bV5Size = sizeof(bi);
		bi.bV5Width = iconSize;
		bi.bV5Height = -iconSize;
		bi.bV5Planes = 1;
		bi.bV5BitCount = 32;
		bi.bV5Compression = BI_BITFIELDS;
		bi.bV5RedMask = 0x00FF0000;
		bi.bV5GreenMask = 0x0000FF00;
		bi.bV5BlueMask = 0x000000FF;
		bi.bV5AlphaMask = 0xFF000000;

		void* bits = nullptr;
		colorBitmap = CreateDIBSection(hdc, reinterpret_cast<BITMAPINFO*>(&bi),
			DIB_RGB_COLORS, &bits, nullptr, 0);
		ReleaseDC(nullptr, hdc);
		hdc = nullptr;

		if (!colorBitmap)
		{
			CleanupResources();
			if (comInitialized) CoUninitialize();
			return Unexpected(eResult::failure, L">>>>> [ibMSWin::LoadIco]. Не удалось создать DIB-секцию для цветного битмапа.");
		}

		if (FAILED(converter->CopyPixels(nullptr, iconSize * 4, iconSize * iconSize * 4, (BYTE*)bits)))
		{
			CleanupResources();
			if (comInitialized) CoUninitialize();
			return Unexpected(eResult::failure, L">>>>> [ibMSWin::LoadIco]. Не удалось скопировать пиксели в цветной битмап.");
		}

		// Создание маски битмапа с использованием альфа-канала
		std::vector<BYTE> maskData(iconSize * iconSize / 8, 0); // 1 бит на пиксель
		if (FAILED(converter->CopyPixels(nullptr, iconSize * 4, iconSize * iconSize * 4, (BYTE*)bits)))
		{
			CleanupResources();
			if (comInitialized) CoUninitialize();
			return Unexpected(eResult::failure, L">>>>> [ibMSWin::LoadIco]. Не удалось скопировать пиксели для обработки маски.");
		}

		// Обработка альфа-канала для создания монохромной маски
		for (int i = 0; i < iconSize * iconSize; ++i)
		{
			BYTE alpha = ((BYTE*)bits)[i * 4 + 3]; // Альфа-канал
			if (alpha < 128) // Пиксели с низкой альфой считаются прозрачными
			{
				int byteIndex = i / 8;
				int bitIndex = 7 - (i % 8);
				maskData[byteIndex] |= (1 << bitIndex);
			}
		}

		maskBitmap = CreateBitmap(iconSize, iconSize, 1, 1, maskData.data());
		if (!maskBitmap)
		{
			CleanupResources();
			if (comInitialized) CoUninitialize();
			return Unexpected(eResult::failure, L">>>>> [ibMSWin::LoadIco]. Не удалось создать маску битмапа.");
		}

		ICONINFO iconInfo = {};
		iconInfo.fIcon = TRUE;
		iconInfo.hbmColor = colorBitmap;
		iconInfo.hbmMask = maskBitmap;

		hIcon = CreateIconIndirect(&iconInfo);
		if (!hIcon)
		{
			CleanupResources();
			if (comInitialized) CoUninitialize();
			return Unexpected(eResult::failure, L">>>>> [ibMSWin::LoadIco]. Не удалось создать иконку.");
		}

		CleanupResources();
		if (comInitialized) CoUninitialize();

		return hIcon;
	}
}
#endif // _WIN64