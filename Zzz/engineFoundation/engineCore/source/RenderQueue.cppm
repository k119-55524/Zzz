
#include "pch.h"

export module RenderQueue;

//import Scene;

export namespace zzz::core
{
	// Класс позволяет накапливать в себе объекты рендринга для последующей их отрисовки
	export class RenderQueue
	{
	public:
		RenderQueue() = default;
		virtual ~RenderQueue() = default;

		void ClearQueue(); // Очищает очередь рендринга перед началом нового кадра
		//void AddCameraToQueue(); // Добавляет объект в очередь рендринга

	protected:

	};

	void RenderQueue::ClearQueue()
	{
	}


}