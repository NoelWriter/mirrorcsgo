#pragma once
#include "..\Utils\DrawManager.h"
#include "..\Utils\GlobalVars.h"
#include "..\Settings.h"


class ESP
{
public:
    void Render();
private:
    void RenderBox(C_BaseEntity* pEnt);
	void Radar(C_BaseEntity * pEnt);
    void RenderName(C_BaseEntity* pEnt, int iterator);
    void RenderWeaponName(C_BaseEntity* pEnt);
	void RenderbtBoneESP(C_BaseEntity * player);
	void DrawBoneESP(C_BaseEntity * pBaseEntity, int it);
	void DrawGrenadeHelper();
	void DrawHealth(C_BaseEntity * pEnt);

    int localTeam{};
    Color teamColor{ 195, 240, 100, 255 };
    Color enemyColor{ 250, 165, 110, 255 };
};
extern ESP g_ESP;