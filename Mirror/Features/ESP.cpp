#include "ESP.h"
#include "..\Utils\Utils.h"
#include "..\SDK\IVEngineClient.h"
#include "..\SDK\PlayerInfo.h"
#include "..\SDK\IVModelInfoClient.hpp"
#include "..\Utils\CGrenadeAPI.h"

ESP g_ESP;

void ESP::RenderBox(C_BaseEntity* pEnt)
{
    Vector vecScreenOrigin, vecOrigin = pEnt->GetRenderOrigin();
    if (!Utils::WorldToScreen(vecOrigin, vecScreenOrigin))
        return;

    Vector vecScreenBottom, vecBottom = vecOrigin;
    vecBottom.z += (pEnt->GetFlags() & FL_DUCKING) ? 54.f : 72.f;
    if (!Utils::WorldToScreen(vecBottom, vecScreenBottom))
        return;

    const auto sx = int(std::roundf(vecScreenOrigin.x)),
               sy = int(std::roundf(vecScreenOrigin.y)),
               h  = int(std::roundf(vecScreenBottom.y - vecScreenOrigin.y)),
               w  = int(std::roundf(h * 0.25f));

	auto boxColor = Color::Black();//g::pLocalEntity->CanSeePlayer(pEnt, pEnt->GetBonePos(8)) ? Color::Black() : Color::Red(); // && !pEnt->IsBehindSmoke(g::pLocalEntity) crashes

	//Bottom
	g_Render.Line(sx - w, sy, sx - w * 0.5, sy, boxColor);
	g_Render.Line(sx + w * 0.5, sy, sx + w, sy, boxColor);
	g_Render.Line(sx - w, sy - 1, sx - w * 0.5, sy - 1, Color::Black());
	g_Render.Line(sx + w * 0.5, sy - 1, sx + w, sy - 1, Color::Black());
	g_Render.Line(sx - w, sy + 1, sx - w * 0.5, sy + 1, Color::Black());
	g_Render.Line(sx + w * 0.5, sy + 1, sx + w, sy + 1, Color::Black());

	//Top
	g_Render.Line(sx - w, sy + h, sx - w * 0.5, sy + h, boxColor);
	g_Render.Line(sx + w * 0.5, sy + h, sx + w, sy + h, boxColor);
	g_Render.Line(sx - w, sy + h + 1, sx - w * 0.5, sy + h + 1, Color::Black());
	g_Render.Line(sx + w * 0.5, sy + h + 1, sx + w, sy + h + 1, Color::Black());
	g_Render.Line(sx - w, sy + h - 1, sx - w * 0.5, sy + h - 1, Color::Black());
	g_Render.Line(sx + w * 0.5, sy + h - 1, sx + w, sy + h - 1, Color::Black());

	//Right part
	g_Render.Line(sx - w, sy, sx - w, sy + h / 4, boxColor);
	g_Render.Line(sx - w, sy + h, sx - w, sy + h * 0.75, boxColor);
	g_Render.Line(sx - w - 1, sy, sx - w - 1, sy + h / 4, Color::Black());
	g_Render.Line(sx - w - 1, sy + h, sx - w - 1, sy + h * 0.75, Color::Black());
	g_Render.Line(sx - w + 1, sy, sx - w + 1, sy + h / 4, Color::Black());
	g_Render.Line(sx - w + 1, sy + h, sx - w + 1, sy + h * 0.75, Color::Black());

	//Left part
	g_Render.Line(sx + w, sy, sx + w, sy + h / 4, boxColor);
	g_Render.Line(sx + w, sy + h, sx + w, sy + h * 0.75, boxColor);
	g_Render.Line(sx + w + 1, sy, sx + w + 1, sy + h / 4, Color::Black());
	g_Render.Line(sx + w + 1, sy + h, sx + w + 1, sy + h * 0.75, Color::Black());
	g_Render.Line(sx + w - 1, sy, sx + w - 1, sy + h / 4, Color::Black());
	g_Render.Line(sx + w - 1, sy + h, sx + w - 1, sy + h * 0.75, Color::Black());
}

void ESP::Radar(C_BaseEntity* pEnt)
{
	if (!pEnt->IsAlive() && pEnt->IsDormant() && !pEnt)
		return;
	auto offset = g_pNetvars->GetOffset("DT_BaseEntity", "m_bSpotted");
	*(char*)((DWORD)(pEnt)+offset) = 1;
}

void ESP::RenderName(C_BaseEntity* pEnt, int iterator)
{
    Vector vecScreenOrigin, vecOrigin = pEnt->GetRenderOrigin();
    if (!Utils::WorldToScreen(vecOrigin, vecScreenOrigin))
        return;

    Vector vecScreenBottom, vecBottom = vecOrigin;
    vecBottom.z += (pEnt->GetFlags() & FL_DUCKING) ? 54.f : 72.f;
    if (!Utils::WorldToScreen(vecBottom, vecScreenBottom))
        return;


    PlayerInfo_t pInfo;
    g_pEngine->GetPlayerInfo(iterator, &pInfo);

    int sx = std::roundf(vecScreenOrigin.x);
    int sy = std::roundf(vecScreenOrigin.y);
    int h  = std::roundf(vecScreenBottom.y - vecScreenOrigin.y);

    g_Render.String(sx, sy + h - 16, CD3DFONT_CENTERED_X | CD3DFONT_DROPSHADOW,
                    (localTeam == pEnt->GetTeam()) ? teamColor : enemyColor,
                    g_Fonts.pFontTahoma10.get(), pInfo.szName);
}

void ESP::RenderWeaponName(C_BaseEntity* pEnt)
{
    Vector vecScreenOrigin, vecOrigin = pEnt->GetRenderOrigin();
    if (!Utils::WorldToScreen(vecOrigin, vecScreenOrigin))
        return;


    auto weapon = pEnt->GetActiveWeapon();
    if (!weapon)
        return;

    auto strWeaponName = weapon->GetName(); // Crash 2

    strWeaponName.erase(0, 7);
    std::transform(strWeaponName.begin(), strWeaponName.end(), strWeaponName.begin(), ::toupper);

    g_Render.String(vecScreenOrigin.x, vecScreenOrigin.y, CD3DFONT_CENTERED_X | CD3DFONT_DROPSHADOW,
                    (localTeam == pEnt->GetTeam()) ? teamColor : enemyColor,
                    g_Fonts.pFontTahoma10.get(), strWeaponName.c_str());
}

void ESP::DrawBoneESP(C_BaseEntity* pBaseEntity, int it)
{
	auto pEnt = pBaseEntity;

	// Get model from target entity
	studiohdr_t* pStudioHdr = g_pMdlInfo->GetStudiomodel2(pEnt->GetModel());

	if (!pStudioHdr)
		return;
	Vector vParent, vChild, sParent, sChild;

	// Itterate through all bones
	for (int j = 0; j < pStudioHdr->numbones; j++)
	{
		mstudiobone_t* pBone = pStudioHdr->GetBone(j);

		// Check if bone is used by a hitbox So we can connect hitboxes
		if (pBone && (pBone->flags & BONE_USED_BY_HITBOX) && (pBone->parent != -1))
		{
			vChild = pEnt->GetBonePos(j);
			vParent = pEnt->GetBonePos(pBone->parent);

			//Check if the connection is present
			if (Utils::WorldToScreen(vParent, sParent) && Utils::WorldToScreen(vChild, sChild))
			{
				// Render a line on each connection
				g_Render.Line(sParent[0], sParent[1], sChild[0], sChild[1], Color(0, 0, 0, 255));
			}
		}
	}

}

void ESP::DrawHealth(C_BaseEntity* pEnt)
{
	int pHealth = pEnt->GetHealth() > 100 ? 100 : pEnt->GetHealth();
	if (!pHealth)
		return;

	Vector vecScreenOrigin, vecOrigin = pEnt->GetRenderOrigin();
	if (!Utils::WorldToScreen(vecOrigin, vecScreenOrigin))
		return;

	Vector vecScreenBottom, vecBottom = vecOrigin;
	vecBottom.z += (pEnt->GetFlags() & FL_DUCKING) ? 54.f : 72.f;
	if (!Utils::WorldToScreen(vecBottom, vecScreenBottom))
		return;



	int sx = std::roundf(vecScreenOrigin.x);
	int sy = std::roundf(vecScreenOrigin.y);
	int h = std::roundf(vecScreenBottom.y - vecScreenOrigin.y);

	g_Render.String(sx + 32, sy + h - 16, CD3DFONT_CENTERED_X | CD3DFONT_DROPSHADOW,
		(localTeam == pEnt->GetTeam()) ? teamColor : enemyColor,
		g_Fonts.pFontTahoma10.get(), std::to_string(pHealth).c_str());
}

int StringToWeapon(std::string weapon) {
	if (!strcmp(weapon.c_str(), "smokegrenade"))
		return 45;
	if (!strcmp(weapon.c_str(), "flashbang"))
		return 43;
	if (!strcmp(weapon.c_str(), "incgrenade"))
		return 46;
}

void ESP::Render()
{

	// Grenade Helper
	if (g_Settings.bEspWGrenade && g_pEngine->IsConnected())
	{
		C_BaseEntity* local = g::pLocalEntity;
		for (int i = 0; i < cGrenade.GrenadeInfo.size(); i++)
		{
			GrenadeInfo_t info;
			if (!cGrenade.GetInfo(i, &info))
				continue;

			int iGrenadeID = StringToWeapon(info.szWeapon);

			auto weapon = local->GetActiveWeapon();
			if (!weapon)
				continue;

			if (!weapon->isGrenade())
				continue;

			Vector vecOnScreenOrigin, vecOnScreenAngles;
			int iCenterY, iCenterX;
			g_pEngine->GetScreenSize(iCenterY, iCenterX);
			iCenterX /= 2;
			iCenterY /= 2;

			float dist = sqrt(pow(local->GetRenderOrigin().x - info.vecOrigin.x, 2) + pow(local->GetRenderOrigin().y - info.vecOrigin.y, 2) + pow(local->GetRenderOrigin().z - info.vecOrigin.z, 2)) * 0.0254f;

			if (dist < 0.5f)
			{
				if (Utils::WorldToScreen(info.vecOrigin, vecOnScreenOrigin))
					g_Render.DrawWave1(info.vecOrigin, 7, Color::Red());

				Vector vecAngles;
				Utils::AngleVectors(info.vecViewangles, vecAngles);
				vecAngles *= 100;

				if (Utils::WorldToScreen((local->GetEyePosition() + vecAngles), vecAngles))
					g_Render.RectFilled(vecAngles.x - 5, vecAngles.y - 5, vecAngles.x + 5, vecAngles.y + 5, Color::Green());

				g_Render.String(iCenterX, iCenterY + 30, CD3DFONT_CENTERED_X | CD3DFONT_DROPSHADOW, Color::White(), g_Fonts.pFontTahoma10.get(), info.szName.c_str());
				g_Render.String(iCenterX, iCenterY, CD3DFONT_CENTERED_X | CD3DFONT_DROPSHADOW, Color::White(), g_Fonts.pFontTahoma10.get(), info.szDescription.c_str());

			}
			else
			{
				if (Utils::WorldToScreen(info.vecOrigin, vecOnScreenOrigin));

				g_Render.DrawWave1(info.vecOrigin, 10, Color::Red());
				g_Render.DrawWave1(info.vecOrigin, 7, Color::Red());
			}
		}
	}
	else
	{
		cGrenade.GrenadeInfo.clear();
	}


    if (!g::pLocalEntity || !g_pEngine->IsInGame())
        return;

	if (!g_Settings.bEspEnable)
		return;

    localTeam = g::pLocalEntity->GetTeam();

    for (int it = 1; it <= g_pEngine->GetMaxClients(); ++it)
    {
        C_BaseEntity* pPlayerEntity = g_pEntityList->GetClientEntity(it);

        if (!pPlayerEntity
            || pPlayerEntity == g::pLocalEntity
            || pPlayerEntity->IsDormant()
            || !pPlayerEntity->IsAlive())
            continue;

		if (!g_Settings.bEspPEnemy && pPlayerEntity->GetTeam() != localTeam)
			continue;

		if (!g_Settings.bEspPTeam && pPlayerEntity->GetTeam() == localTeam)
			continue;


        if (g_Settings.bEspPBoxes)
            this->RenderBox(pPlayerEntity);
		                          
        if (g_Settings.bEspPName)
            this->RenderName(pPlayerEntity, it);

        if (g_Settings.bEspPWeapon)
            this->RenderWeaponName(pPlayerEntity);

		if (g_Settings.bEspWRadar)
			this->Radar(pPlayerEntity);

		if (g_Settings.bEspPBones)
			this->DrawBoneESP(pPlayerEntity, it);

		if (g_Settings.bEspPHealth)
			this->DrawHealth(pPlayerEntity);
    }
}
