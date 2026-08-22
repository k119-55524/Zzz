#pragma once

#include <vector>
#include <string>
#include "core/hardware/CpuInfo.h"
#include "core/hardware/RamInfo.h"
#include "core/hardware/GpuInfo.h"
#include "core/hardware/StorageInfo.h"
#include "core/Serialize/Serializer.h"
#include "core/hardware/MotherboardInfo.h"
#include "core/hardware/MonitorInfo.h"
#include "core/hardware/NetworkAdapterInfo.h"

namespace zzz::core
{
	/**
	 * @brief Состояние оборудования платформы.
	 * Во время работы приложения объект хранит полный актуальный срез текущей системы (CPU, RAM, GPU, Мониторы).
	 * 
	 * @note Смысл пустой строки (id.empty() == true):
	 * Пустая строка может быть ИСКЛЮЧИТЕЛЬНО у m_SelectedGpuId и m_SelectedMonitorId.
	 * Она выставляется в случае первичного запуска приложения или в случае ИЗМЕНЕНИЯ КОНФИГУРАЦИИ ОБОРУДОВАНИЯ (пропажи/замены сохраненного устройства).
	 * Наличие пустой строки указывает подсистемам на необходимость совершить новый выбор устройства:
	 *  - m_SelectedGpuId.empty() == true  -> Подсистема GAPI должна выполнить автовыбор лучшей графической карты.
	 *  - m_SelectedMonitorId.empty() == true -> Оконная система должна привязать окно к Главным монитору (Primary Display).
	 */
	class HardwareState final : public ISerializable
	{
	public:
		HardwareState() = default;

		HardwareState(
			std::vector<CpuInfo> cpus,
			RamInfo ram,
			MotherboardInfo motherboard,
			std::vector<GpuInfo> gpus,
			std::vector<MonitorInfo> monitors,
			std::vector<StorageInfo> storages = {},
			std::vector<NetworkAdapterInfo> networkAdapters = {})
			: m_Cpus(std::move(cpus))
			, m_Ram(std::move(ram))
			, m_Motherboard(std::move(motherboard))
			, m_Gpus(std::move(gpus))
			, m_Monitors(std::move(monitors))
			, m_Storages(std::move(storages))
			, m_NetworkAdapters(std::move(networkAdapters))
		{
			ensure(!m_Cpus.empty(), "HardwareState: список процессоров не может быть пустым.");
			ensure(!m_Gpus.empty(), "HardwareState: список видеокарт не может быть пустым.");
			ensure(!m_Monitors.empty(), "HardwareState: список мониторов не может быть пустым.");
			ensure(m_Ram.GetTotalRamBytes() > 0, "HardwareState: объем ОЗУ должен быть больше 0.");
			ensure(!m_Motherboard.GetVendor().empty() || !m_Motherboard.GetModel().empty() || !m_Motherboard.GetSystemUuid().empty(),
				"HardwareState: данные материнской платы должны быть корректно заданы.");
		}

		// Геттеры данных оборудования
		[[nodiscard]] const std::vector<CpuInfo>& GetCpus() const noexcept { return m_Cpus; }
		[[nodiscard]] const RamInfo& GetRam() const noexcept { return m_Ram; }
		[[nodiscard]] const MotherboardInfo& GetMotherboard() const noexcept { return m_Motherboard; }
		[[nodiscard]] const std::vector<GpuInfo>& GetGpus() const noexcept { return m_Gpus; }
		[[nodiscard]] const std::vector<MonitorInfo>& GetMonitors() const noexcept { return m_Monitors; }
		[[nodiscard]] const std::vector<StorageInfo>& GetStorages() const noexcept { return m_Storages; }
		[[nodiscard]] const std::vector<NetworkAdapterInfo>& GetNetworkAdapters() const noexcept { return m_NetworkAdapters; }
		[[nodiscard]] const std::string& GetSelectedGpuId() const noexcept { return m_SelectedGpuId; }

		void SetSelectedGpuId(std::string gpuId) { m_SelectedGpuId = std::move(gpuId); }

		[[nodiscard]] bool operator==(const HardwareState& other) const noexcept
		{
			return m_Cpus == other.m_Cpus &&
				m_Ram == other.m_Ram &&
				m_Motherboard == other.m_Motherboard &&
				m_Gpus == other.m_Gpus &&
				m_Monitors == other.m_Monitors &&
				m_Storages == other.m_Storages &&
				m_NetworkAdapters == other.m_NetworkAdapters &&
				m_SelectedGpuId == other.m_SelectedGpuId;
		}

		inline void LogFileBlock([[maybe_unused]] std::string_view indentation = {}) const
		{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut("{}[HardwareState]", indentation);

			if (!m_Cpus.empty())
			{
				DOut("{}cpus({})", nestedIndentation, m_Cpus.size());
				for (std::size_t i = 0; i < m_Cpus.size(); ++i)
				{
					DOut("{}cpu #({}/{}):", nestedIndentation + "  ", i + 1, m_Cpus.size());
					m_Cpus[i].LogFileBlock(nestedIndentation + "    ");
				}
				DOut("{}", nestedIndentation);
			}

			DOut("{}ram:", nestedIndentation);
			m_Ram.LogFileBlock(nestedIndentation + "  ");
			DOut("{}", nestedIndentation);

			DOut("{}motherboard:", nestedIndentation);
			m_Motherboard.LogFileBlock(nestedIndentation + "  ");
			DOut("{}", nestedIndentation);

			if (!m_Gpus.empty())
			{
				DOut("{}gpus({})", nestedIndentation, m_Gpus.size());
				for (std::size_t i = 0; i < m_Gpus.size(); ++i)
				{
					DOut("{}gpu #({}/{}):", nestedIndentation + "  ", i + 1, m_Gpus.size());
					m_Gpus[i].LogFileBlock(nestedIndentation + "    ");
				}
				DOut("{}", nestedIndentation);
			}

			if (!m_Monitors.empty())
			{
				DOut("{}monitors({})", nestedIndentation, m_Monitors.size());
				for (std::size_t i = 0; i < m_Monitors.size(); ++i)
				{
					DOut("{}monitor #({}/{}):", nestedIndentation + "  ", i + 1, m_Monitors.size());
					m_Monitors[i].LogFileBlock(nestedIndentation + "    ");
				}
				DOut("{}", nestedIndentation);
			}

			if (!m_Storages.empty())
			{
				DOut("{}storages({})", nestedIndentation, m_Storages.size());
				for (std::size_t i = 0; i < m_Storages.size(); ++i)
				{
					DOut("{}storage #({}/{}):", nestedIndentation + "  ", i + 1, m_Storages.size());
					m_Storages[i].LogFileBlock(nestedIndentation + "    ");
				}
				DOut("{}", nestedIndentation);
			}

			if (!m_NetworkAdapters.empty())
			{
				DOut("{}networkAdapters({})", nestedIndentation, m_NetworkAdapters.size());
				for (std::size_t i = 0; i < m_NetworkAdapters.size(); ++i)
				{
					DOut("{}networkAdapter #({}/{}):", nestedIndentation + "  ", i + 1, m_NetworkAdapters.size());
					m_NetworkAdapters[i].LogFileBlock(nestedIndentation + "    ");
				}
				DOut("{}", nestedIndentation);
			}

			DOut("{}selectedGpuId: {}", nestedIndentation, m_SelectedGpuId);
#endif
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, m_SelectedGpuId);
		}
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, m_SelectedGpuId);
		}

		std::vector<CpuInfo> m_Cpus;
		RamInfo m_Ram;
		MotherboardInfo m_Motherboard;
		std::vector<GpuInfo> m_Gpus;
		std::vector<MonitorInfo> m_Monitors;
		std::vector<StorageInfo> m_Storages;
		std::vector<NetworkAdapterInfo> m_NetworkAdapters;

		std::string m_SelectedGpuId;
	};
}
