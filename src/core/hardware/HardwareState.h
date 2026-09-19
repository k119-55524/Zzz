#pragma once
#include <span>

#include "core/CoreIncludes.h"
#include "core/hardware/CpuInfo.h"
#include "core/hardware/CpuTopology.h"
#include "core/hardware/RamInfo.h"
#include "core/hardware/GpuInfo.h"
#include "core/hardware/StorageInfo.h"
#include "core/hardware/MonitorInfo.h"
#include "core/hardware/MotherboardInfo.h"
#include "core/hardware/NetworkAdapterInfo.h"

namespace zzz::core
{
	/**
	 * @brief Состояние оборудования платформы.
	 * Во время работы приложения объект хранит полный актуальный срез текущей системы (CPU, RAM, GPU, Мониторы).
	 */
	class HardwareState final
	{
	public:
		HardwareState() = delete;
		HardwareState(
			std::vector<CpuInfo> cpus,
			CpuTopology cpuTopology,
			RamInfo ram,
			MotherboardInfo motherboard,
			std::vector<GpuInfo> gpus,
			std::vector<MonitorInfo> monitors,
			std::vector<StorageInfo> storages = {},
			std::vector<NetworkAdapterInfo> networkAdapters = {})
			: m_Cpus(std::move(cpus))
			, m_CpuTopology(std::move(cpuTopology))
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
		[[nodiscard]] std::span<const CpuInfo> GetCpus() const noexcept { return m_Cpus; }
		[[nodiscard]] const CpuTopology& GetCpuTopology() const noexcept { return m_CpuTopology; }
		[[nodiscard]] const RamInfo& GetRam() const noexcept { return m_Ram; }
		[[nodiscard]] const MotherboardInfo& GetMotherboard() const noexcept { return m_Motherboard; }
		[[nodiscard]] std::span<const GpuInfo> GetGpus() const noexcept { return m_Gpus; }
		[[nodiscard]] std::span<const MonitorInfo> GetMonitors() const noexcept { return m_Monitors; }
		[[nodiscard]] std::span<const StorageInfo> GetStorages() const noexcept { return m_Storages; }
		[[nodiscard]] std::span<const NetworkAdapterInfo> GetNetworkAdapters() const noexcept { return m_NetworkAdapters; }

		[[nodiscard]] bool operator==(const HardwareState& other) const noexcept
		{
			return m_Cpus == other.m_Cpus &&
				m_CpuTopology == other.m_CpuTopology &&
				m_Ram == other.m_Ram &&
				m_Motherboard == other.m_Motherboard &&
				m_Gpus == other.m_Gpus &&
				m_Monitors == other.m_Monitors &&
				m_Storages == other.m_Storages &&
				m_NetworkAdapters == other.m_NetworkAdapters;
		}

		inline void LogFileBlock([[maybe_unused]] std::string_view indentation = {}) const
		{
#if Z_ADD_LOGGER
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut(::zzz::core::Hardware, "========== [HardwareState] Platform Hardware State ==========");
			DOut(::zzz::core::Hardware, "{}[HardwareState]", indentation);

			if (!m_Cpus.empty())
			{
				DOut(::zzz::core::Hardware, "{}cpus({})", nestedIndentation, m_Cpus.size());
				for (std::size_t i = 0; i < m_Cpus.size(); ++i)
				{
					DOut(::zzz::core::Hardware, "{}cpu #({}/{}):", nestedIndentation + "  ", i + 1, m_Cpus.size());
					m_Cpus[i].LogFileBlock(nestedIndentation + "    ");
				}
				DOut(::zzz::core::Hardware, "{}", nestedIndentation);
			}

			m_CpuTopology.LogFileBlock(nestedIndentation);
			DOut(::zzz::core::Hardware, "{}", nestedIndentation);

			DOut(::zzz::core::Hardware, "{}ram:", nestedIndentation);
			m_Ram.LogFileBlock(nestedIndentation + "  ");
			DOut(::zzz::core::Hardware, "{}", nestedIndentation);

			DOut(::zzz::core::Hardware, "{}motherboard:", nestedIndentation);
			m_Motherboard.LogFileBlock(nestedIndentation + "  ");
			DOut(::zzz::core::Hardware, "{}", nestedIndentation);

			if (!m_Gpus.empty())
			{
				DOut(::zzz::core::Hardware, "{}gpus({})", nestedIndentation, m_Gpus.size());
				for (std::size_t i = 0; i < m_Gpus.size(); ++i)
				{
					DOut(::zzz::core::Hardware, "{}gpu #({}/{}):", nestedIndentation + "  ", i + 1, m_Gpus.size());
					m_Gpus[i].LogFileBlock(nestedIndentation + "    ");
				}
				DOut(::zzz::core::Hardware, "{}", nestedIndentation);
			}

			if (!m_Monitors.empty())
			{
				DOut(::zzz::core::Hardware, "{}monitors({})", nestedIndentation, m_Monitors.size());
				for (std::size_t i = 0; i < m_Monitors.size(); ++i)
				{
					DOut(::zzz::core::Hardware, "{}monitor #({}/{}):", nestedIndentation + "  ", i + 1, m_Monitors.size());
					m_Monitors[i].LogFileBlock(nestedIndentation + "    ");
				}
				DOut(::zzz::core::Hardware, "{}", nestedIndentation);
			}

			if (!m_Storages.empty())
			{
				DOut(::zzz::core::Hardware, "{}storages({})", nestedIndentation, m_Storages.size());
				for (std::size_t i = 0; i < m_Storages.size(); ++i)
				{
					DOut(::zzz::core::Hardware, "{}storage #({}/{}):", nestedIndentation + "  ", i + 1, m_Storages.size());
					m_Storages[i].LogFileBlock(nestedIndentation + "    ");
				}
				DOut(::zzz::core::Hardware, "{}", nestedIndentation);
			}

			if (!m_NetworkAdapters.empty())
			{
				DOut(::zzz::core::Hardware, "{}networkAdapters({})", nestedIndentation, m_NetworkAdapters.size());
				for (std::size_t i = 0; i < m_NetworkAdapters.size(); ++i)
				{
					DOut(::zzz::core::Hardware, "{}networkAdapter #({}/{}):", nestedIndentation + "  ", i + 1, m_NetworkAdapters.size());
					m_NetworkAdapters[i].LogFileBlock(nestedIndentation + "    ");
				}
				DOut(::zzz::core::Hardware, "{}", nestedIndentation);
			}
#endif
		}

	private:
		std::vector<CpuInfo> m_Cpus;
		CpuTopology m_CpuTopology;
		RamInfo m_Ram;
		MotherboardInfo m_Motherboard;
		std::vector<GpuInfo> m_Gpus;
		std::vector<MonitorInfo> m_Monitors;
		std::vector<StorageInfo> m_Storages;
		std::vector<NetworkAdapterInfo> m_NetworkAdapters;
	};
}
