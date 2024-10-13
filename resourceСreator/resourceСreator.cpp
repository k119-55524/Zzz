
#include "Header.h"

#ifdef _SERVICES

zResult SaveScene(const shared_ptr<IIO> platformIO, const zStr& fileName)
{
	Scene sceneSave(platformIO);
	zResult res = sceneSave.Save(fileName);

	if (res.isSuccess())
		wcout << L"Success save scene " << fileName << "." << endl;
	else
	{
		wcout << L"Error save scene " << fileName << ". " << res.GetDescription() << endl;
		return zResult(e_ErrorCode::eFailure);
	}

	return res;
}

zResult LoadScene(const shared_ptr<IIO> platformIO, const zStr& fileName)
{
		Scene sceneLoad(platformIO);
		zResult res = sceneLoad.Load(fileName);

		if (res.isSuccess())
			wcout << L"Success load scene " << fileName << "." << endl;
		else
			wcout << L"Error load scene " << fileName << ". " << res.GetDescription() << endl;

		return res;
}

zResult SaveGameSettings(const shared_ptr<IIO> platformIO)
{
	UserGameSettings gs(platformIO);
	gs.SetDefault();
	zResult res = gs.Save();

	if (res.isSuccess())
		wcout << L"Success save GameSettings." << endl;
	else
	{
		wcout << L"Error save GameSettings. " << res.GetDescription() << endl;
		return zResult(e_ErrorCode::eFailure);
	}

	return res;
}

zResult LoadGameSettings(const shared_ptr<IIO> platformIO)
{
	UserGameSettings gs(platformIO);
	zResult res = gs.Load();

	if (res.isSuccess())
		wcout << L"Success load GameSettings." << endl;
	else
		wcout << L"Error load GameSettings. " << res.GetDescription() << endl;

	return res;
}

#endif // _SERVICES

int main()
{
#ifdef _SERVICES

	zStr fileName = c_DefaultStartSceneName;

	FactoryPlatform factoryPlatform;
	shared_ptr<IIO> platformIO = factoryPlatform.GetPlatformIO();

	wcout << L"+-------------------------------------------+" << endl;

	if (!SaveScene(platformIO, fileName).isSuccess())
		return -1;

	if (!LoadScene(platformIO, fileName).isSuccess())
		return -1;

	wcout << L"+-------------------------------------------+" << endl;

	if (!SaveGameSettings(platformIO).isSuccess())
		return -1;

	if (!LoadGameSettings(platformIO).isSuccess())
		return -1;

	wcout << L"+-------------------------------------------+" << endl;

#endif // _SERVICES

	return 0;
}
