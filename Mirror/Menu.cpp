#include "GUI\GUI.h"
#include "Settings.h"
#include "Utils/CGrenadeAPI.h"


void Detach() { g_Settings.bCheatActive = false; };
void updateGrenadeMap() { cGrenade.bUpdateGrenadeInfo(g_pEngine->GetLevelNameShort()); };

void MenuMain::Initialize()
{
    /* Create our main window (Could have multiple if you'd create vec. for it)  */
    auto mainWindow = std::make_shared<Window>("Mirror", SSize(600, 400), g_Fonts.pFontTahoma8, g_Fonts.pFontTahoma8); //Width - Height
    {
		auto tab1 = std::make_shared<Tab>("Aimbot", 1, mainWindow);
		{
			auto sectAimbotMain = tab1->AddSection("Aimbot Settings", 0.5f);
			{
				sectAimbotMain->AddCheckBox("Enable", &g_Settings.bAimbotEnable);
				sectAimbotMain->AddCheckBox("Backtracking", &g_Settings.bAimbotBacktrack);
				sectAimbotMain->AddSlider("Backtrack Ticks", &g_Settings.bAimbotBacktrackTicks, 1, 12);
			}
			auto sectWeapon = tab1->AddSection("Rifle Settings", 0.5f);
			{
				sectWeapon->AddSlider("Rifle Fov", &g_Settings.bAimbotFovRifle, 0.1f, 45);
				sectWeapon->AddSlider("Rifle Smooth", &g_Settings.bAimbotSmoothRifle, 1, 30);
				sectWeapon->AddCombo("Rifle Hitbox", &g_Settings.bAimbotHitboxRifle, std::vector<std::string>{ "Head", "Stomach", "Chest" });
				sectWeapon->AddDummy();
				sectWeapon->AddSlider("Sniper Fov", &g_Settings.bAimbotFovSniper, 0.1f, 45);
				sectWeapon->AddSlider("Sniper smooth", &g_Settings.bAimbotSmoothSniper, 1, 30);
				sectWeapon->AddCombo("Sniper Hitbox", &g_Settings.bAimbotHitboxSniper, std::vector<std::string>{ "Head", "Stomach", "Chest" });
				sectWeapon->AddDummy();
				sectWeapon->AddSlider("Pistol Fov", &g_Settings.bAimbotFovPistol, 0.1f, 45);
				sectWeapon->AddSlider("Pistol Smooth", &g_Settings.bAimbotSmoothPistol, 1, 30);
				sectWeapon->AddCombo("Pistol Hitbox", &g_Settings.bAimbotHitboxPistol, std::vector<std::string>{ "Head", "Stomach", "Chest" });
			}
		}
		mainWindow->AddChild(tab1);
		auto tab2 = std::make_shared<Tab>("Ragebot", 2, mainWindow);
		{
			auto sectRageAimbotMain = tab2->AddSection("Aimbot Settings", 1.f);
			{
				sectRageAimbotMain->AddCheckBox("Enable", &g_Settings.bRagebotEnable);
				sectRageAimbotMain->AddSlider("Fov", &g_Settings.bRagebotFov, 1, 360);
				sectRageAimbotMain->AddCheckBox("Autofire", &g_Settings.bRagebotAutoFire);
				sectRageAimbotMain->AddSlider("Minimum damage", &g_Settings.bRagebotMinDamage, 1, 100);
				sectRageAimbotMain->AddCheckBox("Silent", &g_Settings.bRagebotSilent);
				sectRageAimbotMain->AddCheckBox("Autowall", &g_Settings.bRagebotAutowall);
			}
			auto sectAntiAim = tab2->AddSection("Anti-Aim Settings", 1.f);
			{

			}
		}
		mainWindow->AddChild(tab2);
		auto tab3 = std::make_shared<Tab>("ESP", 2, mainWindow);
		{
			auto sectPlayer = tab3->AddSection("Player", 1.f);
			{
				sectPlayer->AddCheckBox("Enable", &g_Settings.bEspEnable);
				sectPlayer->AddCheckBox("Show Enemy", &g_Settings.bEspPEnemy);
				sectPlayer->AddCheckBox("Show Team", &g_Settings.bEspPTeam);
				sectPlayer->AddCheckBox("Boxes", &g_Settings.bEspPBoxes);
				sectPlayer->AddCheckBox("Weapons", &g_Settings.bEspPWeapon);
				sectPlayer->AddCheckBox("Names", &g_Settings.bEspPName);
				sectPlayer->AddCheckBox("Bones", &g_Settings.bEspPBones);
				sectPlayer->AddCheckBox("Chams", &g_Settings.bEspPChams);
			}
			auto sectWorld = tab3->AddSection("World", 1.f);
			{
				sectWorld->AddCheckBox("Radar", &g_Settings.bEspWRadar);
				sectWorld->AddCheckBox("Grenade Helper", &g_Settings.bEspWGrenade);
				sectWorld->AddButton("Update Map", updateGrenadeMap);
			}
		}
		mainWindow->AddChild(tab3);
		auto tab4 = std::make_shared<Tab>("Other", 1, mainWindow);
		{
			auto sectMisc = tab4->AddSection("", .5f);
			{
				
				sectMisc->AddCheckBox("Bhop", &g_Settings.bBhopEnabled);
				sectMisc->AddDummy();
				sectMisc->AddButton("Detach", Detach);
			}
		}
		mainWindow->AddChild(tab4);
    }
    this->vecChildren.push_back(mainWindow);

    /* Create our mouse cursor (one instance only) */
    mouseCursor = std::make_unique<MouseCursor>();

    /* Do the first init run through all of the objects */
    for (auto& it : vecChildren)
        it->Initialize();
}


//-----------------------------------------------------------------------------
//								  MENU EXAMPLE
//-----------------------------------------------------------------------------

//    ///TODO: window->AddTab()
//    auto tab1 = std::make_shared<Tab>("Main Tab", 2, mainWindow);
//    {
//        /* Create sections for it */
//        auto sectMain = tab1->AddSection("TestSect", 1.f);
//        {
//            /* Add controls within section */
//            sectMain->AddCheckBox("Bunnyhop Enabled", &g_Settings.bBhopEnabled);
//            sectMain->AddCheckBox("Show Player Names", &g_Settings.bShowNames);
//            sectMain->AddButton("Shutdown", Detach);
//            sectMain->AddSlider("TestSlider", &float123, 0, 20);
//            sectMain->AddSlider("intslider", &testint3, 0, 10);
//            sectMain->AddCombo("TestCombo", &testint, std::vector<std::string>{ "Value1", "Value2", "Value3" });
//        }

//        auto sectMain2 = tab1->AddSection("TestSect2", 1.f);
//        {
//            sectMain2->AddCombo("TestCombo2", &testint2, std::vector<std::string>{ "ttest", "ttest2", "ttest3" });
//            sectMain2->AddCheckBox("CheckboxSect2_1", &g_Settings.bShowBoxes);
//            sectMain2->AddCheckBox("Show Player Boxes", &g_Settings.bShowBoxes);
//            sectMain2->AddCheckBox("Show Player Weapons", &g_Settings.bShowWeapons);
//        }
//    } mainWindow->AddChild(tab1);   /* For now */

//    auto tab2 = std::make_shared<Tab>("Test Tab", 1, mainWindow);
//    {
//        auto sectMain = tab2->AddSection("TestSect", .5f);
//        {
//            /* Add controls within section */
//            sectMain->AddCheckBox("CheckboxSect2_1", &g_Settings.bShowBoxes);
//            sectMain->AddCheckBox("Show Player Boxes", &g_Settings.bShowBoxes);
//            sectMain->AddCheckBox("Show Player Weapons", &g_Settings.bShowWeapons);
//            sectMain->AddButton("Shutdown", Detach);
//            sectMain->AddSlider("TestSlider", &float123, 0, 20);
//            sectMain->AddSlider("intslider", &testint3, 0, 10);
			//sectMain->AddCombo("TestCombo", &testint, std::vector<std::string>{ "Value1", "Value2", "Value3" });
//        }

//        auto sectMain2 = tab2->AddSection("TestSect2", .5f);
//        {
			//sectMain2->AddCombo("TestCombo2", &testint2, std::vector<std::string>{ "ttest", "ttest2", "ttest3" });
//            sectMain2->AddCheckBox("Bunnyhop Enabled", &g_Settings.bBhopEnabled);
//            sectMain2->AddCheckBox("Show Player Names", &g_Settings.bShowNames);
//        }
//    } mainWindow->AddChild(tab2);
