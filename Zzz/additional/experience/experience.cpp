
import ModuleA;
import ModuleB;

using namespace zzz;

int main()
{
	ModuleA a;
	ModuleB b;

	a.Method(&b);
	b.Method(&a);

	return 0;
}
