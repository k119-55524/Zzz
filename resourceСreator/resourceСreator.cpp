
#include "Header.h"

int main()
{
#ifdef _SERVICES

	zResult res;
	zStr fileName = L"Data\\TestScene.zsc";

	{
		Scene sceneSave;
		zResult res = sceneSave.Save(fileName);

		if (res.isSuccess())
			wcout << L"Success save " << fileName << "." << endl;
		else
			wcout << "Error save" << fileName << ". " << res.GetDescription() << endl;
	}

	{
		Scene sceneLoad;
		res = sceneLoad.Load(fileName);

		if (res.isSuccess())
			wcout << L"Success load " << fileName << "." << endl;
		else
			wcout << "Error load" << fileName << ". " << res.GetDescription() << endl;
	}

#endif

	return 0;
}
