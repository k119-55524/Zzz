#pragma once

#include "Helpers/zVersion.h"

namespace Zzz
{
	//------------------------------------------
	// ����� ��������� ��� ���� ����� ��������
	//------------------------------------------

	const zU64 c_MinimumWindowsWidth = 100;
	const zU64 c_MinimumWindowsHeight = 100;
	const zU64 c_MaximumWindowsWidth = 3840;	// Ultra HD 4K
	const zU64 c_MaximumWindowsHeight = 2160;	// Ultra HD 4K

	const zStr c_AppResourcesFolder(L"data");

	const zStr c_DefaultStartSceneName = L"DefaultScene.zsc";
	const zStr c_HeapSceneFile(L"hzsfn");	// ������ ������� � ����� �����
	const zVersion c_VerScenesSerialize(0, 0, 1, 1);

	const zStr c_GameSettingsFileName(L"zugs.zgs");
	const zStr c_HeapGameSettingsFile(L"hzugs");	// ������ ������� � ����� ���������������� ��������
	const zVersion c_VerGameSettingsSerialize(0, 0, 1, 1);

	const zStr c_MSWindowsClassName = L"zGameWinClass";	// ��� ������ ���� ��� ����������� ��� � Microsoft Windows
	const zU64 c_MSIcoID = 133;	// TODO. ���� ���. �������� ��� ������ �����������.
	const zStr c_CaptionWindows = L"zGame";
}