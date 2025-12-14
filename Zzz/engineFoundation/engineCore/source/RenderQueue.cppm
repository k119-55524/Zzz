
export module RenderQueue;

import RenderArea;

export namespace zzz::core
{
	// Класс позволяет накапливать в себе объекты рендринга для последующей их отрисовки
	export class RenderQueue
	{
	public:
		RenderQueue() = default;
		virtual ~RenderQueue() = default;

		// Очищает очередь рендринга перед началом нового кадра
		void ClearQueue(const std::shared_ptr<RenderArea> renderArea);
		[[nodiscard]] const std::shared_ptr<RenderArea> GetRenderArea() const noexcept { return m_RenderArea; };

	protected:
		std::shared_ptr<RenderArea> m_RenderArea;
	};

	void RenderQueue::ClearQueue(const std::shared_ptr<RenderArea> renderArea)
	{
		m_RenderArea = renderArea;
	}
}