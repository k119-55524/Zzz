#pragma once

#include "core/enums/eGAPIType.h"
#include "core/enums/eLogMessageType.h"

namespace zzz::engine
{
	using namespace zzz::core;

	// Единая точка форматирования и маршрутизации debug-сообщений слоя GAPI (Vulkan validation layer /
	// DirectX12 Info Queue / Metal command buffer error) в логгер движка.
	//
	// Report переиспользует "родной" для движка eLogMessageType вместо отдельного eGAPIDebugSeverity - тип
	// сообщения (Warning/Error/...) и категория (LogGAPI - гарантированные сообщения; LogGAPIVerbose -
	// фильтруемый рантаймом информационный/verbose шум) решают те же задачи, что раньше решали
	// eGAPIDebugSeverity/eGAPIDebugCategory + список c_GAPIDebugReportFlags, но уже общей для всего движка
	// системой категорий (см. core/utils/LogCategory.h): Warning/Error/Exception/Critical/Fatal гарантированно
	// доходят до IDE (см. LogMacros.h, ApplyFilter=false), а Message (Info/Verbose) фильтруется рантаймом через
	// категорию LogGAPIVerbose (Logger::IsCategoryEnabled) - решать "репортить ли" отдельным списком флагов
	// (как раньше ShouldReport/c_GAPIDebugReportFlags) больше не нужно.
	//
	// Каждый бэкенд сам мапит свой нативный severity в eLogMessageType и разбирает специфичный для своего API
	// формат сообщения (например построчная фильтрация у Vulkan) ДО вызова Report - сюда приходит уже готовая
	// для печати строка.
	//
	// Z_GAPI_VERBOSE_DEBUG_LAYER остаётся отдельной, более ранней оптимизацией на стороне самого нативного
	// слоя (см. VulkanAPI::EnableDebugMessenger/DirectX12API) - она решает, включать ли verbose-режим самого
	// GAPI-sлоя вообще (и тем самым избегает лишних вызовов Report), а категория LogGAPIVerbose - независимый
	// от неё рантайм-фильтр уже принятых сообщений на стороне движка.
	//
	// Гейт как у обычных DOut* (Z_ADD_LOGGER, без отдельного Z_DEBUG_BUILD) - осознанное решение: раз
	// DOutError/DOutWarning для обычных логов уже целиком зависят от Z_ADD_LOGGER, отдельный гейт для класса,
	// который лишь маршрутизирует сообщения в те же DOut*, ничего не даёт.
	class GAPIDebugLogger
	{
	public:
#if Z_ADD_LOGGER
		static void Report(eGAPIType backend, eLogMessageType severity, std::string_view message);
#else
		static void Report(eGAPIType, eLogMessageType, std::string_view) {}
#endif
	};
}
