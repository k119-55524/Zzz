
module ModuleA;
import ModuleB;

namespace zzz
{
	void ModuleA::Method(ModuleB* b)
	{
		b->SetupB();
	}

	void ModuleA::SetupA()
	{

	}
}