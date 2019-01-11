#include "GUI\GUI.h"
#include "Settings.h"


void Detach() { g_Settings.bCheatActive = false; }
void showRifles() { g_Settings.bAimbotTab1 = true; g_Settings.bAimbotTab2 = false;}

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
			}
			auto sectWeapon = tab1->AddSection("Rifle Settings", 0.5f);
			{
				sectWeapon->AddSlider("Rifle Fov", &g_Settings.bAimbotFovRifle, 0.1f, 45);
				sectWeapon->AddSlider("Rifle Smooth", &g_Settings.bAimbotSmoothRifle, 1, 30);
				sectWeapon->AddCombo("Hitbox", &g_Settings.bAimbotHitboxRifle, std::vector<std::string>{ "Head", "Stomach", "Chest" });
				sectWeapon->AddSlider("Sniper Fov", &g_Settings.bAimbotFovSniper, 0.1f, 45);
				sectWeapon->AddSlider("Sniper smooth", &g_Settings.bAimbotSmoothSniper, 1, 30);
				sectWeapon->AddCombo("Hitbox", &g_Settings.bAimbotHitboxSniper, std::vector<std::string>{ "Head", "Stomach", "Chest" });
				sectWeapon->AddSlider("Pistol Fov", &g_Settings.bAimbotFovPistol, 0.1f, 45);
				sectWeapon->AddSlider("Pistol Smooth", &g_Settings.bAimbotSmoothPistol, 1, 30);
				sectWeapon->AddCombo("Hitbox", &g_Settings.bAimbotHitboxPistol, std::vector<std::string>{ "Head", "Stomach", "Chest" });
			}
		}
		mainWindow->AddChild(tab1);
		auto tab2 = std::make_shared<Tab>("ESP", 2, mainWindow);
		{
			auto sectPlayer = tab2->AddSection("Player", 1.f);
			{
				sectPlayer->AddCheckBox("Enable", &g_Settings.bEspEnable);
				sectPlayer->AddCheckBox("Show Enemy", &g_Settings.bEspPEnemy);
				sectPlayer->AddCheckBox("Show Team", &g_Settings.bEspPTeam);
				sectPlayer->AddCheckBox("Boxes", &g_Settings.bEspPBoxes);
				sectPlayer->AddCheckBox("Weapons", &g_Settings.bEspPWeapon);
				sectPlayer->AddCheckBox("Names", &g_Settings.bEspPName);
			}
			auto sectWorld = tab2->AddSection("World", 1.f);
			{

			}
		}
		mainWindow->AddChild(tab2);
		auto tab3 = std::make_shared<Tab>("Other", 1, mainWindow);
		{
			auto sectDetach = tab3->AddSection("", .5f);
			{
				sectDetach->AddButton("Detach", Detach);
			}
		}
		mainWindow->AddChild(tab3);
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
