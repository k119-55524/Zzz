#include "pch.h"
export module Material;

import IPSO;

namespace zzz
{
	export class Material
	{
	public:
		Material() = delete;
		Material(const Material&) = delete;
		Material(Material&&) = delete;
		Material& operator=(const Material&) = delete;
		Material& operator=(Material&&) = delete;
		explicit Material(std::shared_ptr<IPSO> _PSO);
		~Material() = default;

	private:
		std::shared_ptr<IPSO> m_PSO;
	};

	Material::Material(std::shared_ptr<IPSO> _PSO) :
		m_PSO{ _PSO }
	{
		ensure(m_PSO != nullptr, ">>>>> [Material::Material( ... )]. PSO pointer cannot be null.");
	}
}