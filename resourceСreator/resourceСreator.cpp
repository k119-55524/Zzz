
#include "Header.h"

int main()
{
#ifdef _EDITOR
	Scene scene;

	zColor clearColor;
	clearColor.Default();
	scene.SetClearColor(clearColor);

	scene.Save(L"Data/TestScene.zsc");
#endif

	return 0;
}
