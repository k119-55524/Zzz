#pragma once

#include "core/utils/macros/LogMacros.h"

// DOut*-варианты для GAPIDebugLogger::Report (Vulkan/DirectX12/Metal debug-колбэки). Loc = пустой
// source_location - call site тут всегда одна и та же строка колбэка, показывать нечего.
#if Z_ADD_LOGGER || Z_DEBUG_BUILD
#define DOutWarningGAPI(...) Z_LOG_DISPATCH(LogWarning, std::source_location{}, __VA_ARGS__)
#define DOutErrorGAPI(...)   Z_LOG_DISPATCH(LogError,   std::source_location{}, __VA_ARGS__)
#else
#define DOutWarningGAPI(...)
#define DOutErrorGAPI(...)
#endif

// Гейт как у обычных DOut* (без Z_DEBUG_BUILD) - в отличие от Warning/Error, verbose-уровень не обязан
// выживать в чистом Debug без Z_ADD_LOGGER: это опциональный шум, а не риск потерять ошибку валидации.
#if Z_ADD_LOGGER && Z_GAPI_VERBOSE_DEBUG_LAYER
#define DOutGAPI(...) Z_LOG_DISPATCH(LogMessage, std::source_location{}, __VA_ARGS__)
#else
#define DOutGAPI(...)
#endif
