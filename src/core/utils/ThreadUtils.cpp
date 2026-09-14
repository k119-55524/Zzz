
#include <string>

#include "ThreadUtils.h"
#include "core/CoreIncludes.h"

namespace zzz::core
{
	void SetCurrentThreadName(std::string_view name)
	{
		if (name.empty())
			return;

#if defined(Z_WINDOWS)
		const int wlen = MultiByteToWideChar(CP_UTF8, 0, name.data(), static_cast<int>(name.size()), nullptr, 0);
		if (wlen > 0)
		{
			std::wstring wname(static_cast<size_t>(wlen), L'\0');
			MultiByteToWideChar(CP_UTF8, 0, name.data(), static_cast<int>(name.size()), wname.data(), wlen);
			SetThreadDescription(GetCurrentThread(), wname.c_str());
		}
#elif defined(Z_APPLE)
		pthread_setname_np(std::string(name).c_str());
#elif defined(Z_LINUX) || defined(Z_ANDROID)
		// Ядро Linux жестко ограничивает длину имени потока до 16 байт (TASK_COMM_LEN, включая \0)
		const std::string shortName = (name.size() > 15) ? std::string(name.substr(0, 15)) : std::string(name);
		pthread_setname_np(pthread_self(), shortName.c_str());
#endif
	}

	void SetCurrentThreadPriority(eThreadPriority priority)
	{
#if defined(Z_WINDOWS)
		int winPriority = THREAD_PRIORITY_NORMAL;
		switch (priority)
		{
		case eThreadPriority::Critical:   winPriority = THREAD_PRIORITY_HIGHEST; break;
		case eThreadPriority::High:       winPriority = THREAD_PRIORITY_ABOVE_NORMAL; break;
		case eThreadPriority::Normal:     winPriority = THREAD_PRIORITY_NORMAL; break;
		case eThreadPriority::Background: winPriority = THREAD_PRIORITY_LOWEST; break;
		default:
			THROW_RUNTIME("Неизвестный приоритет потока: {}", static_cast<uint32_t>(priority));
		}
		SetThreadPriority(GetCurrentThread(), winPriority);
#elif defined(Z_APPLE)
		qos_class_t qos = QOS_CLASS_DEFAULT;
		switch (priority)
		{
		case eThreadPriority::Critical:   qos = QOS_CLASS_USER_INTERACTIVE; break;
		case eThreadPriority::High:       qos = QOS_CLASS_USER_INITIATED; break;
		case eThreadPriority::Normal:     qos = QOS_CLASS_DEFAULT; break;
		case eThreadPriority::Background: qos = QOS_CLASS_BACKGROUND; break;
		default:
			THROW_RUNTIME("Неизвестный приоритет потока: {}", static_cast<uint32_t>(priority));
		}
		pthread_set_qos_class_self_np(qos, 0);
#elif defined(Z_LINUX) || defined(Z_ANDROID)
		int niceVal = 0;
		switch (priority)
		{
		case eThreadPriority::Critical:   niceVal = -10; break;
		case eThreadPriority::High:       niceVal = -2; break;
		case eThreadPriority::Normal:     niceVal = 0; break;
		case eThreadPriority::Background: niceVal = 10; break;
		default:
			THROW_RUNTIME("Неизвестный приоритет потока: {}", static_cast<uint32_t>(priority));
		}
		setpriority(PRIO_PROCESS, 0, niceVal);
#endif
	}
}
