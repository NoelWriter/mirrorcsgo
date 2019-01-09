#pragma once
#include "GUI\GUI.h"
#include <filesystem>

using namespace ui;
namespace fs = std::experimental::filesystem;

class Settings
{
public:
    void Initialize(MenuMain* pMenuObj);

    void SaveSettings(const std::string& strFileName, MenuMain* pMenuObj);
    void LoadSettings(const std::string& strFileName, MenuMain* pMenuObj);

private:
    void UpdateDirectoryContent(const fs::path& fsPath);
    inline fs::path GetFolderPath();

    fs::path                 fsPath{};
    std::vector<std::string> vecFileNames{};

public:
    /* All our settings variables will be here  *
    * The best would be if they'd get          *
    * initialized in the class itself.         */

    bool  bCheatActive  = true;
    bool  bMenuOpened   = false;
    bool  bBhopEnabled  = false;
    bool  bShowBoxes    = false;
    bool  bShowNames    = false;
    bool  bShowWeapons  = false;

	//Aimbot
	bool  bEnableAimbot = false;
	float bAimbotFov = 5.f;
	float bAimbotSmooth = 5.f;
	int   bAimbotHitbox = 1;
};

extern Settings g_Settings;

