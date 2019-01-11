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
	bool  bBhopEnabled	= false;

	//ESP
	bool  bEspEnable	= false;
	bool  bEspPEnemy	= false;
	bool  bEspPTeam     = false;
    bool  bEspPBoxes    = false;
    bool  bEspPName		= false;
    bool  bEspPWeapon	= false;

	//Aimbot
	bool  bAimbotEnable = false;
	bool  bAimbotTab1	= true;
	bool  bAimbotTab2	= false;
	bool  bAimbotTab3	= false;

	//Rifle
	float bAimbotFovRifle		= 5.f;
	float bAimbotSmoothRifle	= 5.f;
	int   bAimbotHitboxRifle	= 0;

	//Sniper
	float bAimbotFovSniper		= 5.f;
	float bAimbotSmoothSniper	= 5.f;
	int   bAimbotHitboxSniper	= 2;

	//Pistol
	float bAimbotFovPistol		= 5.f;
	float bAimbotSmoothPistol	= 5.f;
	int   bAimbotHitboxPistol	= 0;
};

extern Settings g_Settings;

