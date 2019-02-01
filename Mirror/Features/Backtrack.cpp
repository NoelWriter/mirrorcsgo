#include "Backtrack.h"
#include <Windows.h>
#include "Aimbot.h"
#include "..\Utils\Utils.h"
#include "..\Utils\GlobalVars.h"
#include "..\Hooks.h"
#include "..\SDK\CGlobalVarsBase.h"
#include "..\SDK\IClientEntity.h"
#include "Ragewall.h"

#define TIME_TO_TICKS( dt )	( ( int )( 0.5f + ( float )( dt ) / g_pGlobalVars->intervalPerTick ) )

BackTrack* backtracking = new BackTrack();
backtrackData l_SavedTicks[64][25]; //support for 128tick servers
rageBacktrackData r_SavedTicks[64][25]; //support for 128tick servers

bool BackTrack::IsTickValid(int tick)
{
	int delta = latest_tick - tick;
	float deltaTime = delta * g_pGlobalVars->intervalPerTick;
	return (fabs(deltaTime) <= 0.2f);
}

void BackTrack::legitBackTrack(CUserCmd* cmd)
{
	if (g_Settings.bAimbotBacktrack && !g_Settings.bRagebotBacktrack)
	{
		int bestTargetIndex = -1;
		float bestFov = FLT_MAX;
		C_BaseEntity* pLocal = g::pLocalEntity;
		if (!pLocal->IsAlive())
			return;

		for (int i = 0; i < g_pEngine->GetMaxClients(); i++)
		{
			auto entity = g_pEntityList->GetClientEntity(i);

			if (!entity || !pLocal)
				continue;

			if (entity == pLocal)
				continue;

			if (entity->GetTeam() == pLocal->GetTeam())
				continue;

			if (entity->IsAlive())
			{

				int hitboxSaved = g_Aimbot.getHitbox(g::pActiveWeapon);
				if (!hitboxSaved)
					hitboxSaved = 8;

				float simtime = entity->GetSimulationTime();
				Vector hitboxPos = entity->GetBonePos(hitboxSaved);

				l_SavedTicks[i][cmd->command_number % g_Settings.bAimbotBacktrackTicks] = backtrackData{ 
					simtime, 
					hitboxPos,
				};

				Vector myPos = g::pLocalEntity->GetEyePosition();
				Vector pEntPos = entity->GetEyePosition();
				QAngle aimAngle = Utils::CalcAngle(myPos, pEntPos);

				auto fov = g_Aimbot.get_fov(g::pCmd->viewangles + g::pLocalEntity->GetPunchAngles() * 2, aimAngle);

				if (fov < bestFov)
				{
					bestFov = fov;
					bestTargetIndex = i;
				}
			}
		}

		float bestTargetSimTime;
		if (bestTargetIndex != -1)
		{
			float tempFloat = FLT_MAX;

			for (int t = 0; t < g_Settings.bAimbotBacktrackTicks; ++t)
			{
				Vector myPos = g::pLocalEntity->GetEyePosition();
				Vector pEntPos = l_SavedTicks[bestTargetIndex][t].hitboxPos;
				QAngle aimAngle = Utils::CalcAngle(myPos, pEntPos);

				auto fov = g_Aimbot.get_fov(g::pCmd->viewangles + g::pLocalEntity->GetPunchAngles() * 2, aimAngle);
				if (tempFloat > fov && l_SavedTicks[bestTargetIndex][t].simtime > pLocal->GetSimulationTime() - 1)
				{
					tempFloat = fov;
					bestTargetSimTime = l_SavedTicks[bestTargetIndex][t].simtime;
				}
			}

			cmd->tick_count = TIME_TO_TICKS(bestTargetSimTime);
		}
	}
}

void rageBacktrackData::SaveRecord(C_BaseEntity* pEnt) {

}

void BackTrack::RageBackTrack(CUserCmd* cmd, C_BaseEntity* pEnt)
{
	if (!g_Settings.bRagebotBacktrack)
		return;

	C_BaseEntity* pLocal = g::pLocalEntity;
	if (!pLocal->IsAlive())
		return;

	for (int i = 0; i < g_pEngine->GetMaxClients(); i++)
	{
		auto entity = g_pEntityList->GetClientEntity(i);

		if (!entity || !pLocal)
			continue;

		if (entity == pLocal)
			continue;

		if (entity->GetTeam() == pLocal->GetTeam())
			continue;

		if (!entity->IsAlive())
			continue;

		

	}

}

int realHitboxSpot[] = { 0, 1, 2, 3 };