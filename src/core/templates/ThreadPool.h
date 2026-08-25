#pragma once

#include "core/CoreIncludes.h"
#include <queue>
#include <future>
#include <type_traits>
#include <functional>

namespace zzz::templates
{
#if defined(_WIN32)
	inline void SetThreadName(const char* threadName)
	{
		if (!threadName)
			return;

		int wlen = MultiByteToWideChar(CP_UTF8, 0, threadName, -1, nullptr, 0);
		if (wlen > 0)
		{
			std::wstring wname(static_cast<size_t>(wlen), L'\0');
			MultiByteToWideChar(CP_UTF8, 0, threadName, -1, wname.data(), wlen);
			SetThreadDescription(GetCurrentThread(), wname.c_str());
		}
	}
#else
	inline void SetThreadName(const char*) {}
#endif

	/**
	 * @brief Пул работяг-потоков для асинхронного выполнения задач движка.
	 * 
	 * @note ВАЖНО: Запрещено вызывать метод Join() или уничтожать объект ThreadPool 
	 * изнутри задачи, исполняемой в этом же пуле потоков (WorkerThread), 
	 * так как это приведет к взаимной блокировке (Deadlock).
	 */
	class ThreadPool
	{
		Z_NO_COPY_MOVE(ThreadPool);

	public:
		explicit ThreadPool(const std::string& threadName, size_t threadCount)
			: done{ false }
			, activeThreadCount{ 0 }
		{
			if (threadCount == 0)
				THROW_RUNTIME("Parameters cannot be 0.");

			threads.reserve(threadCount);
			try
			{
				for (size_t i = 0; i < threadCount; ++i)
				{
					threads.emplace_back(&ThreadPool::WorkerThread, this, threadName, i);
				}
			}
			catch (...)
			{
				{
					std::lock_guard<std::mutex> lock(cv_mutex);
					done = true;
				}
				cv.notify_all();
				for (auto& thread : threads)
				{
					if (thread.joinable())
						thread.join();
				}
				THROW_RUNTIME("Failed to create threads.");
			}
		}

		~ThreadPool()
		{
			{
				std::lock_guard<std::mutex> lock(cv_mutex);
				done = true;
				std::queue<std::function<void()>> emptyQueue;
				std::swap(workQueue, emptyQueue);
			}
			cv.notify_all();

			for (auto& thread : threads)
			{
				if (thread.joinable())
					thread.join();
			}

			threads.clear();
		}

		/**
		 * @brief Отправляет задачу на выполнение в пул потоков.
		 * Поддерживает любые std::invoke-совместимые callable-объекты, лямбды, 
		 * функции и указатели на методы классов (&Class::Method).
		 * Также корректно поддерживает move-only типы (std::unique_ptr).
		 */
		template<typename FunctionType, typename... Args>
		auto Submit(FunctionType&& f, Args&&... args) -> std::future<std::invoke_result_t<FunctionType, Args...>>
		{
			using ReturnType = std::invoke_result_t<FunctionType, Args...>;

			auto task = std::make_shared<std::packaged_task<ReturnType()>>(
				[f = std::forward<FunctionType>(f), ...args = std::forward<Args>(args)]() mutable {
					return std::invoke(std::move(f), std::move(args)...);
				}
			);

			std::future<ReturnType> result = task->get_future();
			{
				std::lock_guard<std::mutex> lock(cv_mutex);
				if (done)
				{
					THROW_RUNTIME("Cannot submit task to stopped ThreadPool.");
				}

				workQueue.push([task]() {
					(*task)();
				});
			}
			cv.notify_one();
			return result;
		}

		/**
		 * @brief Пакетная отправка множества независимых задач в один вызов.
		 * Позволяет захватить мьютекс всего один раз и разбудить доступные воркер-потоки.
		 * Возвращает std::tuple из std::future для каждой из переданных задач.
		 * 
		 * Пример использования:
		 * auto [f1, f2, f3] = pool.SubmitTasks(
		 *     [this] { Task1(); },
		 *     [this] { Task2(); },
		 *     [this] { Task3(); }
		 * );
		 */
		template<typename... Tasks>
		auto SubmitTasks(Tasks&&... tasks)
		{
			static_assert(sizeof...(Tasks) > 0, "SubmitTasks requires at least one task.");

			auto submitSingle = [this](auto&& taskFunc) {
				using TaskType = std::decay_t<decltype(taskFunc)>;
				using ReturnType = std::invoke_result_t<TaskType>;

				auto packagedTask = std::make_shared<std::packaged_task<ReturnType()>>(
					[f = std::forward<TaskType>(taskFunc)]() mutable {
						return std::invoke(std::move(f));
					}
				);

				std::future<ReturnType> fut = packagedTask->get_future();
				return std::pair{ packagedTask, std::move(fut) };
			};

			auto taskPairs = std::tuple{ submitSingle(std::forward<Tasks>(tasks))... };

			{
				std::lock_guard<std::mutex> lock(cv_mutex);
				if (done)
				{
					THROW_RUNTIME("Cannot submit tasks to stopped ThreadPool.");
				}

				std::apply([this](auto&... pairs) {
					(workQueue.push([pkg = pairs.first]() { (*pkg)(); }), ...);
				}, taskPairs);
			}

			cv.notify_all();

			return std::apply([](auto&... pairs) {
				return std::tuple{ std::move(pairs.second)... };
			}, taskPairs);
		}

		[[nodiscard]] bool IsCompleted() const
		{
			std::lock_guard<std::mutex> lk(cv_mutex);
			return workQueue.empty() && activeThreadCount == 0;
		}

		[[nodiscard]] size_t GetThreadCount() const noexcept
		{
			return threads.size();
		}

		/**
		 * @brief Ожидает завершения всех текущих и стоящих в очереди задач.
		 * @warning Не вызывайте метод Join() из задачи самого ThreadPool во избежание дедлока!
		 */
		void Join()
		{
			std::unique_lock<std::mutex> lk(cv_mutex);

			cv.wait(lk, [this]
				{
					return workQueue.empty() && activeThreadCount == 0;
				});
		}

	private:
		mutable std::mutex cv_mutex;
		std::condition_variable cv;

		bool done;
		size_t activeThreadCount;
		std::queue<std::function<void()>> workQueue;
		std::vector<std::thread> threads;

		void WorkerThread(const std::string& _threadName, size_t id)
		{
#if defined(_WIN32)
			std::string threadName = std::format(">>>>> [zzz::ThreadPool]. Thread-using class: zzz::{}({})", _threadName, id);
			SetThreadName(threadName.c_str());
#endif

			while (true)
			{
				std::function<void()> task;

				{
					std::unique_lock<std::mutex> lk(cv_mutex);

					cv.wait(lk, [this] {
						return !workQueue.empty() || done;
					});

					if (done && workQueue.empty())
						return;

					task = std::move(workQueue.front());
					workQueue.pop();
					activeThreadCount++;
				}

				try
				{
					task();
				}
				catch (...)
				{
					DOutError("ThreadPool task exception caught.");
				}

				{
					std::lock_guard<std::mutex> lk(cv_mutex);
					activeThreadCount--;
					if (workQueue.empty() && activeThreadCount == 0)
					{
						cv.notify_all();
					}
				}
			}
		}
	};
}
