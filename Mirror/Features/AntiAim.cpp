#include "AntiAim.h"
#include "Ragewall.h"
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

	if (!g_Settings.bRagebotAAEnable || !g_Settings.bRagebotAAYawReal)
		return;

	if (pCmd->buttons & IN_USE)
		return;

	auto weapon = g::pActiveWeapon;

	if (!weapon)
		return;

	if (weapon->GetItemDefinitionIndex() == WEAPON_KNIFE)
		return;

	if (g::pLocalEntity->IsImmune())
		return;

	if (weapon->GetItemDefinitionIndex() == WEAPON_REVOLVER)
	{
		if (pCmd->buttons & IN_ATTACK2)
			return;

		//if (weapon->CanFirePostPone() && (usercmd->buttons & IN_ATTACK))
		//	return;
	}
	else if (weapon->isGrenade())
	{
		return;
	}
	else
	{
		if (weapon->GetCSWpnData()->weapon_type() == 0 && ((pCmd->buttons & IN_ATTACK) || (pCmd->buttons & IN_ATTACK2)))
			return;
		else if ((pCmd->buttons & IN_ATTACK) && (weapon->GetItemDefinitionIndex() != WEAPON_C4))
			return;
	}

	if (g::pLocalEntity->GetMoveType() == MoveType_t::MOVETYPE_NOCLIP || g::pLocalEntity->GetMoveType() == MoveType_t::MOVETYPE_LADDER)
		return;

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

	case AA_YAW_FREESTANDING:

		return freestanding();
		break;
	}


	return vPos;
}

float AntiAim::freestanding() {
	enum {
		back,
		right,
		left
	};

	QAngle vecAngles;
	g_pEngine->GetViewAngles(vecAngles);

	C_BaseEntity* pTarget = nullptr;

	float best_fov = 360.f;
	std::vector< C_BaseEntity * > enemies;
	static constexpr int damage_tolerance = 30;

	for (int i = 1; i <= g_pEngine->GetMaxClients(); ++i)
	{
		C_BaseEntity *player = g_pEntityList->GetClientEntity(i);

		if (!player || player == nullptr)
			continue;

		if (player == g::pLocalEntity)
			continue;

		if (player->GetTeam() == g::pLocalEntity->GetTeam())
			continue;

		if (player->IsDormant())
			continue;

		if (player->IsImmune())
			continue;

		if (!player->IsAlive())
			continue;

		float fov = g_Aimbot.get_fov(vecAngles, Utils::CalcAngle(g::pLocalEntity->GetEyePosition(), player->GetOrigin()));

		if (fov < best_fov)
		{
			best_fov = fov;
			pTarget = player;
		}

		enemies.push_back(player);
	}

	if (!pTarget)
		return vecAngles.y + 180.f;

	auto calculate_damage = [&](Vector point) -> int {
		int damage = 0;
		for (auto& enemy : enemies) {
			damage += g_RageWall.GetDamageVec(point, enemy, 1);
		}
		return damage;
	};

	auto rotate_and_extend = [](Vector position, float yaw, float distance) -> Vector {
		Vector direction;
		Utils::AngleVectors(Vector(0, yaw, 0), direction);

		return position + (direction * distance);
	};

	Vector
		head_position = g::pLocalEntity->GetEyePosition(),
		at_target = Utils::CalcVecAngle(g::pLocalEntity->GetOrigin(), pTarget->GetOrigin());

	float angles[3] = {
		at_target.y + 180.f,
		at_target.y - 70.f,
		at_target.y + 70.f
	};

	Vector head_positions[3] = {
		rotate_and_extend(head_position, at_target.y + 180.f, 17.f),
		rotate_and_extend(head_position, at_target.y - 70.f, 17.f),
		rotate_and_extend(head_position, at_target.y + 70.f, 17.f)
	};

	int damages[3] = {
		calculate_damage(head_positions[back]),
		calculate_damage(head_positions[right]),
		calculate_damage(head_positions[left])
	};

	if (damages[right] > damage_tolerance && damages[left] > damage_tolerance)
		return angles[back];

	if (at_target.x > 15.f)
		return angles[back];

	if (damages[right] == damages[left]) {
		auto trace_to_end = [](Vector start, Vector end) -> Vector {
			trace_t trace;
			CTraceFilterWorldOnly filter;
			Ray_t ray;

			ray.Init(start, end);
			g_pEngineTrace->TraceRay(ray, MASK_ALL, &filter, &trace);

			return trace.endpos;
		};

		Vector
			trace_right_endpos = trace_to_end(head_position, head_positions[right]),
			trace_left_endpos = trace_to_end(head_position, head_positions[left]);

		Ray_t ray;
		trace_t trace;
		CTraceFilterWorldOnly filter;

		ray.Init(trace_right_endpos, pTarget->GetEyePosition());
		g_pEngineTrace->TraceRay(ray, MASK_ALL, &filter, &trace);
		float distance_1 = (trace.startpos - trace.endpos).Length();

		ray.Init(trace_left_endpos, pTarget->GetEyePosition());
		g_pEngineTrace->TraceRay(ray, MASK_ALL, &filter, &trace);
		float distance_2 = (trace.startpos - trace.endpos).Length();

		if (fabs(distance_1 - distance_2) > 15.f)
			return (distance_1 < distance_2) ? angles[right] : angles[left];
		else
			return angles[back];
	}
	else
		return (damages[right] < damages[left]) ? angles[right] : angles[left];
}