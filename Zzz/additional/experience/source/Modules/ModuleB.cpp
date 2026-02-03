
module ModuleB;

import ModuleA;

namespace zzz
{
	void ModuleB::Method(ModuleA* a)
	{
		a->SetupA();
	}

	void ModuleB::SetupB()
	{

	}
}