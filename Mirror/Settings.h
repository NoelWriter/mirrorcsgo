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

    bool  bCheatActive			= true;
    bool  bMenuOpened			= false;
	bool  bBhopEnabled			= false;
	bool  bMiscThirdPerson		= false;

	//ESP
	bool  bEspEnable			= false;
	bool  bEspPEnemy			= false;
	bool  bEspPTeam				= false;
    bool  bEspPBoxes			= false;
    bool  bEspPName				= false;
    bool  bEspPWeapon			= false;
	bool  bEspPBones			= false;
	bool  bEspPChams			= false;
	bool  bEspPHealth			= false;

	bool  bEspWRadar			= false;
	bool  bEspWGrenade			= false;

	//Aimbot
	bool  bAimbotEnable			= false;
	bool  bAimbotBacktrack		= false;
	int   bAimbotBacktrackTicks = 1;

	//Rifle
	float bAimbotFovRifle		= 3.f;
	float bAimbotSmoothRifle	= 7.f;
	int   bAimbotHitboxRifle	= 0;

	//Sniper
	float bAimbotFovSniper		= 3.f;
	float bAimbotSmoothSniper	= 3.f;
	int   bAimbotHitboxSniper	= 2;

	//Pistol
	float bAimbotFovPistol		= 2.f;
	float bAimbotSmoothPistol	= 8.f;
	int   bAimbotHitboxPistol	= 0;

	//Ragebot
	bool  bRagebotEnable		= false;
	float bRagebotFov			= 1.f;
	bool  bRagebotAutoFire		= false;
	float bRagebotMinDamage		= 1.f;
	bool  bRagebotHitchance		= false;
	float bRagebotHitchanceA	= 1.f;
	bool  bRagebotSilent		= false;
	bool  bRagebotAutowall      = false;
	bool  bRagebotBacktrack     = false;
	bool  bRagebotAutostop		= false;

	bool  bRagebotAAEnable		= false;
	int   bRagebotAAYawReal		= 0;
};

extern Settings g_Settings;

