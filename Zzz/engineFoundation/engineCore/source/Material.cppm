
export module Material;

import IPSO;

namespace zzz
{
	export class Material
	{
		Z_NO_COPY_MOVE(Material);

	public:
		explicit Material(std::shared_ptr<IPSO> _PSO);
		~Material() = default;

		[[nodiscard]] inline std::shared_ptr<IPSO> GetPSO() const noexcept { return m_PSO; }

	private:
		std::shared_ptr<IPSO> m_PSO;
	};

	Material::Material(std::shared_ptr<IPSO> _PSO) :
		m_PSO{ _PSO }
	{
		ensure(m_PSO != nullptr, ">>>>> [Material::Material( ... )]. PSO pointer cannot be null.");
	}
}