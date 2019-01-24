#include "AntiAim.h"
#include "autowall.h"
#include "Math.h"
#include "../SDK/IEngineTrace.hpp"
#include "../SDK/IPhysics.h"
#include "../Utils/ICvar.h"
#include "Aimbot.h"
#include <Windows.h>
#include "..\Utils\Utils.h"
#include "..\Utils\GlobalVars.h"
#include "..\Hooks.h"
#include "..\SDK\Studio.hpp"
#include "Backtrack.h"
#include "..\SDK\CGlobalVarsBase.h"
#include "..\SDK\ClientClass.h"

AntiAim g_AntiAim;


void AntiAim::doAntiAim(CUserCmd *pCmd)
{
	if (!g_Settings.bRagebotAAEnable || (!g_Settings.bRagebotAAYawFake && !g_Settings.bRagebotAAYawReal))
		return;

	if (pCmd->buttons & IN_USE)
		return;

	auto weapon = g::pActiveWeapon;

	if (!weapon)
		return;

	if (weapon->GetItemDefinitionIndex() == WEAPON_REVOLVER)
	{
		if (pCmd->buttons & IN_ATTACK2)
			return;

		//if (weapon->CanFirePostPone() && (usercmd->buttons & IN_ATTACK))
		//	return;
	}
	else if (weapon->isGrenade())
			return;
	else
	{
		if (weapon->GetCSWpnData()->weapon_type() == 0 && ((pCmd->buttons & IN_ATTACK) || (pCmd->buttons & IN_ATTACK2)))
			return;
		else if ((pCmd->buttons & IN_ATTACK) && (weapon->GetItemDefinitionIndex() != WEAPON_C4))
			return;
	}

	if (g::pLocalEntity->GetMoveType() == MoveType_t::MOVETYPE_NOCLIP || g::pLocalEntity->GetMoveType() == MoveType_t::MOVETYPE_LADDER)
		return;

	PVOID pebp;
	__asm mov pebp, ebp;
	bool* pbSendPacket = (bool*)(*(DWORD*)pebp - 0x1C);
	bool& bSendPacket = *pbSendPacket;
	g::bSendPacket = bSendPacket;

	pCmd->viewangles.x = GetPitch();
	pCmd->viewangles.y = GetYaw();

	if (pCmd->command_number % 3) pCmd->viewangles.y += 50;
}

float AntiAim::GetPitch()
{
	return 88.99f;
}

float AntiAim::GetYaw()
{
	static bool left = false;
	static bool right = false;
	static bool backwards = true;

	static bool flip = false;
	flip = !flip;

	float_t pos = g::pCmd->viewangles.y;

	QAngle qPos;
	g_pEngine->GetViewAngles(qPos);
	float vPos = qPos.y;

	switch (g_Settings.bRagebotAAYawReal)
	{
	case AA_YAW_BACKWARDS:

		return vPos + 180.0f;
		break;

	case AA_YAW_BACKWARDS_CYCLE:
		
		float fValue;
		if (g::pCmd->tick_count > 40)
		{
			int tCycle = g::pCmd->tick_count % 40;
			int fCycle = tCycle - (tCycle / 2);
			fValue = 180.0f + fCycle;
		}
		else
		{
			fValue = 180.f;
		}
		
		return vPos + fValue;
		break;

	case AA_YAW_FAKE_EVADE:

		return vPos + 180.0f;
		break;
	}


	return vPos;
}