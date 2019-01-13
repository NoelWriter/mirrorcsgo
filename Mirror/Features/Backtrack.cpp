#include "Backtrack.h"
#include <Windows.h>
#include "Aimbot.h"
#include "..\Utils\Utils.h"
#include "..\Utils\GlobalVars.h"
#include "..\Hooks.h"

#define TIME_TO_TICKS( dt )	( ( int )( 0.5f + ( float )( dt ) / g_pGlobalVars->intervalPerTick ) )

BackTrack* backtracking = new BackTrack();
backtrackData headPositions[64][25]; //support for 128tick servers

void BackTrack::Update(int tick_count)
{
	latest_tick = tick_count;
	for (int i = 0; i < 64; i++)
	{
		UpdateRecord(i);
	}
}

bool BackTrack::IsTickValid(int tick)
{
	int delta = latest_tick - tick;
	float deltaTime = delta * g_pGlobalVars->intervalPerTick;
	return (fabs(deltaTime) <= 0.2f);
}

void BackTrack::UpdateRecord(int i)
{
	C_BaseEntity* pEntity = g_pEntityList->GetClientEntity(i);
	if (pEntity && pEntity->IsAlive())
	{
		//float lby = pEntity->GetLowerBodyYaw();
		//if (lby != records[i].lby)
		//{
		//	records[i].tick_count = latest_tick;
		//	records[i].lby = lby;
		//	records[i].headPosition = pEntity->GetBonePos(8);
		//}
	}
	else
	{
		records[i].tick_count = 0;
	}
}

bool BackTrack::RunLBYBackTrack(int i, CUserCmd* cmd, Vector& aimPoint)
{
	if (IsTickValid(records[i].tick_count))
	{
		aimPoint = records[i].headPosition;
		cmd->tick_count = records[i].tick_count;
		return true;
	}
	return false;
}

void BackTrack::legitBackTrack(CUserCmd* cmd)
{
	if (g_Settings.bAimbotBacktrack)
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

				int hitboxSaved = g_Aimbot.getHitbox(g::pLocalEntity->GetActiveWeapon());
				if (!hitboxSaved)
					hitboxSaved = 8;

				float simtime = entity->GetSimulationTime();
				Vector hitboxPos = entity->GetBonePos(hitboxSaved);
				//float lowerBodyYaw = entity->GetLowerBodyYaw();
				//Vector velocity = entity->GetVelocity();

				headPositions[i][cmd->command_number % g_Settings.bAimbotBacktrackTicks] = backtrackData{ 
					simtime, 
					hitboxPos,
					//lowerBodyYaw,
					//velocity
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
				Vector pEntPos = headPositions[bestTargetIndex][t].hitboxPos;
				QAngle aimAngle = Utils::CalcAngle(myPos, pEntPos);

				auto fov = g_Aimbot.get_fov(g::pCmd->viewangles + g::pLocalEntity->GetPunchAngles() * 2, aimAngle);
				if (tempFloat > fov && headPositions[bestTargetIndex][t].simtime > pLocal->GetSimulationTime() - 1)
				{
					tempFloat = fov;
					bestTargetSimTime = headPositions[bestTargetIndex][t].simtime;
				}
			}

			cmd->tick_count = TIME_TO_TICKS(bestTargetSimTime);
		}
	}
}



//#define TIME_TO_TICKS( dt )	( ( int )( 0.5f + ( float )( dt ) / g_pGlobalVars->intervalPerTick ) )
//
//Backtrack g_Backtrack;
//backtrack_data hitbox_positions[64][12];

//void Backtrack::RunBacktrack()
//{
//	if (!g::pLocalEntity)
//		return;
//
//	int bestTarget = -1;
//	float bestFov = 360;
//	float bestDistance = 8164;
//	C_BaseEntity* target = nullptr;
//	auto localTeam = g::pLocalEntity->GetTeam();
//	
//	// Loop through available players connected to the server
//	for (int i = 0; i < g_pEngine->GetMaxClients(); i++)
//	{
//		C_BaseEntity* pPlayerEntity = g_pEntityList->GetClientEntity(i);
//
//		// Various checks
//		if (!pPlayerEntity
//			|| !pPlayerEntity->IsAlive()
//			|| pPlayerEntity->IsDormant()
//			|| pPlayerEntity == g::pLocalEntity
//			|| pPlayerEntity->GetTeam() == localTeam)
//			continue;
//
//		// Get some eye positions
//		Vector myPos = g::pLocalEntity->GetEyePosition();
//		Vector pEntPos = pPlayerEntity->GetEyePosition();
//		QAngle aimAngle = Utils::CalcAngle(myPos, pEntPos);
//
//		// Get the head position
//		Vector head_pos = pPlayerEntity->GetBonePos(8);
//
//		// Get the simulation time
//		float simulation_time = pPlayerEntity->GetSimulationTime();
//
//		// Set up bone matrix
//		bool bIsBoneMatrixSetup = false;
//		matrix3x4_t boneMatrix[128];
//		if (pPlayerEntity->SetupBones(boneMatrix, 128, BONE_USED_BY_HITBOX, static_cast<float>(GetTickCount64())))
//			bIsBoneMatrixSetup = true;
//
//		// Define what data we are going to save
//		hitbox_positions[i][g::pCmd->command_number % 13] = backtrack_data
//		{
//			simulation_time, // When is the tick?
//			head_pos,		 // Where is the head position
//			pPlayerEntity->GetRenderAngles(), // What is the angle of the entity
//			pPlayerEntity->GetOrigin(), // What is the angle of the entity
//			boneMatrix, // The bonematrix from the player so we can calculate other factors as well
//			bIsBoneMatrixSetup, // Did we setup the bonematrix?
//			pPlayerEntity->GetVelocity() // Velocity of the entity
//		};
//
//		// We are going to use the get_fov method from our Aimbot class to get the fov
//		auto fov = g_Aimbot.get_fov(g::pCmd->viewangles + g::pLocalEntity->GetPunchAngles() * 2, aimAngle);
//
//		// Get the closest target to backtrack
//		if (fov < bestFov)
//		{
//			bestFov = fov;
//			bestTarget = i;
//		}
//	}
//
//	if (bestTarget == -1)
//		return;
//
//	float simulation_time = -1;
//	bestFov = 360;
//
//	// TODO: Custom backtracking based on settings.
//	for (int i = 0; i < g_Settings.bAimbotBacktrackTicks; i++)
//	{
//		// Get the head position from the tick we are currently on
//		Vector pEntPos = hitbox_positions[bestTarget][i].head_position;
//
//		// Get the angle from our view to the head of the tick we are currently on
//		QAngle tickAimAngle = Utils::CalcAngle(g::pLocalEntity->GetBonePos(8), pEntPos);
//
//		// The fov including the recoil value;
//		auto tickFov = g_Aimbot.get_fov(g::pCmd->viewangles + g::pLocalEntity->GetPunchAngles() * 2, tickAimAngle);
//
//		// Check if the closes tick isn't currently happening
//		if (hitbox_positions[bestTarget][i].simtime <= g::pLocalEntity->GetSimulationTime() - 1)
//			continue;
//
//		if (tickFov < bestFov)
//		{
//			bestFov = tickFov;
//			simulation_time = hitbox_positions[bestTarget][i].simtime;
//		}
//	}
//
//	if (!simulation_time || simulation_time <= g::pLocalEntity->GetSimulationTime() - 1)
//		return;
//
//	if (g::pCmd->buttons & IN_ATTACK)
//		g::pCmd->tick_count = TIME_TO_TICKS(simulation_time);
//}