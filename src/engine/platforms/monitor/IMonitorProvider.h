#pragma once

#include <vector>
#include <string>
#include <memory>
#include "core/hardware/MonitorInfo.h"
#include "math/Point2D.h"
#include "math/Rect2D.h"

namespace zzz::engine
{
	using namespace zzz::core;
	using namespace zzz::math;

	/**
	 * @brief Абстрактный сервис подсистемы платформы для работы с мониторами (дисплеями).
	 * Предоставляет сведения о подключенных мониторах, их рабочих областях (WorkArea)
	 * и безопасных координатах без использования нативных платформозависимых типов.
	 */
	class IMonitorProvider
	{
	public:
		virtual ~IMonitorProvider() = default;

		/**
		 * @brief Возвращает список всех доступных в системе мониторов.
		 */
		[[nodiscard]] virtual const std::vector<MonitorInfo>& GetMonitors() const noexcept = 0;

		/**
		 * @brief Возвращает информацию о главном мониторе по умолчанию (индекс #0 / Primary Display).
		 */
		[[nodiscard]] virtual MonitorInfo GetPrimaryMonitor() const = 0;

		/**
		 * @brief Находит монитор, содержащий указанную точку (например, центр окна).
		 * @param point Точка в системных виртуальных координатах.
		 */
		[[nodiscard]] virtual MonitorInfo GetMonitorForPoint(const Point2D<zI32>& point) const = 0;

		/**
		 * @brief Находит монитор, имеющий наибольшее перекрытие с указанным прямоугольником окна.
		 * @param rect Прямоугольник окна в виртуальных координатах.
		 */
		[[nodiscard]] virtual MonitorInfo GetMonitorForRect(const Rect2D<zI32>& rect) const = 0;

		/**
		 * @brief Ищет монитор по его платформенному идентификатору (PlatformMonitorId).
		 * В случае отсутствия или отключения монитора возвращает главный монитор (Primary Display #0).
		 * @param monitorId Уникальный идентификатор сохраненного монитора.
		 */
		[[nodiscard]] virtual MonitorInfo GetMonitorById(const std::string& monitorId) const = 0;

		/**
		 * @brief Корректирует и подгоняет прямоугольник окна под рабочую область (WorkArea) указанного монитора.
		 * Если окно превышает габариты WorkArea, его размеры пропорционально уменьшаются.
		 * @param windowRect Исходный прямоугольник окна.
		 * @param monitor Монитор назначения.
		 * @return Скоректированный безопасный прямоугольник окна.
		 */
		[[nodiscard]] virtual Rect2D<zI32> FitToWorkArea(const Rect2D<zI32>& windowRect, const MonitorInfo& monitor) const = 0;

		/**
		 * @brief Центрирует прямоугольник окна по центру рабочей области (WorkArea) указанного монитора.
		 * @param windowRect Исходный прямоугольник окна (используется размер).
		 * @param monitor Монитор назначения.
		 * @return Прямоугольник окна, отцентрированный по рабочей области.
		 */
		[[nodiscard]] virtual Rect2D<zI32> CenterOnWorkArea(const Rect2D<zI32>& windowRect, const MonitorInfo& monitor) const = 0;

		/**
		 * @brief Обновляет внутренний список мониторов при изменении конфигурации/разрешения в системе.
		 */
		virtual void RefreshMonitors() = 0;
	};
}
