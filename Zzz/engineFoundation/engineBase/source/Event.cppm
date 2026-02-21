
export module Event;

namespace zzz
{
	template<typename CallbackType>
	class eventBase
	{
	protected:
		std::vector<std::pair<std::shared_ptr<CallbackType>, std::weak_ptr<CallbackType>>> listeners;
		std::mutex listenersMutex;

		void removeInvalid()
		{
			std::lock_guard<std::mutex> lock(listenersMutex);
			listeners.erase(std::remove_if(listeners.begin(), listeners.end(),
				[](const auto& entry) { return entry.second.expired(); }), listeners.end());
		}

	public:
		void clear()
		{
			std::lock_guard<std::mutex> lock(listenersMutex);
			listeners.clear();
		}
	};
}

export namespace zzz::core
{
	template<typename... Args>
	class Event : public eventBase<std::function<void(Args...)>>
	{
	public:
		using CallbackType = std::function<void(Args...)>;

		template<typename F>
		void operator+=(F&& func)
		{
			std::lock_guard<std::mutex> lock(this->listenersMutex);
			auto handle = std::make_shared<CallbackType>(std::forward<F>(func));
			this->listeners.push_back({ handle, handle });
		}

		void operator()(Args... args)
		{
			std::lock_guard<std::mutex> lock(this->listenersMutex);
			auto it = this->listeners.begin();
			while (it != this->listeners.end())
			{
				if (auto cb = it->second.lock())
				{
					(*cb)(args...);
					++it;
				}
				else
				{
					it = this->listeners.erase(it);
				}
			}
		}
	};

	template<>
	class Event<void> : public eventBase<std::function<void()>>
	{
	public:
		using CallbackType = std::function<void()>;

		template<typename F>
		void operator+=(F&& func)
		{
			std::lock_guard<std::mutex> lock(this->listenersMutex);
			auto handle = std::make_shared<CallbackType>(std::forward<F>(func));
			this->listeners.push_back({ handle, handle });
		}

		void operator()()
		{
			std::lock_guard<std::mutex> lock(this->listenersMutex);
			auto it = this->listeners.begin();
			while (it != this->listeners.end())
			{
				if (auto cb = it->second.lock())
				{
					(*cb)();
					++it;
				}
				else
				{
					it = this->listeners.erase(it);
				}
			}
		}
	};
}