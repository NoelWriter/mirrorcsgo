#include "Backtrack.h"
#include <Windows.h>
#include "Aimbot.h"
#include "..\Utils\Utils.h"
#include "..\Utils\GlobalVars.h"
#include "..\Hooks.h"
#include "..\SDK\CGlobalVarsBase.h"
#include "..\SDK\IClientEntity.h"
#include "Ragewall.h"
#include "..\Utils\ICvar.h"

#define TIME_TO_TICKS( dt )	( ( int )( 0.5f + ( float )( dt ) / g_pGlobalVars->intervalPerTick ) )
#define TICKS_TO_TIME(t) (g_pGlobalVars->intervalPerTick * (t) )

BackTrack g_backtrack;
backtrackData l_SavedTicks[64][25]; //support for 128tick servers
rageBacktrackData r_SavedTicks[64][25]; //support for 128tick servers


bool BackTrack::IsTickValid(int tick)
{
	INetChannelInfo *nci = g_pEngine->GetNetChannelInfo();

	static auto sv_maxunlag = g_pCVar->FindVar("sv_maxunlag");

	if (!nci || !sv_maxunlag)
		return false;

	float correct = clamp(nci->GetLatency(FLOW_OUTGOING) + GetLerpTime(), 0.f, sv_maxunlag->GetFloat());

	float deltaTime = correct - (g_pGlobalVars->curtime - TICKS_TO_TIME(tick));

	return fabsf(deltaTime) < 0.2f;
}

float BackTrack::GetLerpTime()
{
	int ud_rate = g_pCVar->FindVar("cl_updaterate")->GetInt();
	ConVar *min_ud_rate = g_pCVar->FindVar("sv_minupdaterate");
	ConVar *max_ud_rate = g_pCVar->FindVar("sv_maxupdaterate");

	if (min_ud_rate && max_ud_rate)
		ud_rate = max_ud_rate->GetInt();

	float ratio = g_pCVar->FindVar("cl_interp_ratio")->GetFloat();

	if (ratio == 0)
		ratio = 1.0f;

	float lerp = g_pCVar->FindVar("cl_interp")->GetFloat();
	ConVar *c_min_ratio = g_pCVar->FindVar("sv_client_min_interp_ratio");
	ConVar *c_max_ratio = g_pCVar->FindVar("sv_client_max_interp_ratio");

	if (c_min_ratio && c_max_ratio && c_min_ratio->GetFloat() != 1)
		ratio = clamp(ratio, c_min_ratio->GetFloat(), c_max_ratio->GetFloat());

	return max(lerp, (ratio / ud_rate));
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

void LagRecord::SaveRecord(C_BaseEntity *player)
{
	m_vecOrigin = player->GetOrigin();
	//m_vecAbsOrigin = player->SetAbsOrigin();
	m_angAngles = player->GetEyeAngles();
	m_flSimulationTime = player->GetSimulationTime();
	m_vecMins = player->GetCollideable()->OBBMins();
	m_vecMax = player->GetCollideable()->OBBMaxs();
	m_nFlags = player->GetFlags();
	m_vecVelocity = player->GetVelocity();

	int layerCount = player->GetNumAnimOverlays();
	for (int i = 0; i < layerCount; i++)
	{
		AnimationLayer *currentLayer = player->GetAnimOverlay(i);
		m_LayerRecords[i].m_nOrder = currentLayer->m_nOrder;
		m_LayerRecords[i].m_nSequence = currentLayer->m_nSequence;
		m_LayerRecords[i].m_flWeight = currentLayer->m_flWeight;
		m_LayerRecords[i].m_flCycle = currentLayer->m_flCycle;
	}
	m_arrflPoseParameters = player->m_flPoseParameter();

	m_iTickCount = g_pGlobalVars->tickcount;
	m_vecHeadSpot = player->GetBonePos(8);
}

void BackTrack::RageBackTrack()
{
	if (!g_Settings.bRagebotBacktrack)
		return;

	C_BaseEntity* pLocal = g::pLocalEntity;
	if (!pLocal->IsAlive())
		return;

	if (g_pGlobalVars->maxClients <= 1)
	{
		ClearHistory();
		return;
	}

	for (int i = 0; i < g_pEngine->GetMaxClients(); i++)
	{
		// Get player
		C_BaseEntity *player = g_pEntityList->GetClientEntity(i);

		// Get the lag records for this player
		auto &lag_records = this->m_LagRecord[i];

		// Check if player is available to backtrack
		if (!CheckTarget(i))
		{
			if (lag_records.size() > 0)
				lag_records.clear();

			continue;
		}
		
		int32_t ent_index = player->EntIndex();
		float_t sim_time = player->GetSimulationTime();

		LagRecord cur_lagrecord;

		RemoveBadRecords(ent_index, lag_records);

		if (lag_records.size() > 0)
		{
			auto &tail = lag_records.back();

			if (tail.m_flSimulationTime == sim_time)
				continue;
		}

		cur_lagrecord.SaveRecord(player); // first let's create the record

		if (!lag_records.empty()) // apply specific stuff that is needed
		{
			auto &temp_lagrecord = lag_records.back();
			int32_t priority_level = GetPriorityLevel(player, &temp_lagrecord);

			cur_lagrecord.m_iPriority = priority_level;
			cur_lagrecord.m_flPrevLowerBodyYaw = temp_lagrecord.m_flPrevLowerBodyYaw;
			cur_lagrecord.m_arrflPrevPoseParameters = temp_lagrecord.m_arrflPrevPoseParameters;

			if (priority_level == 3)
				cur_lagrecord.m_angAngles.y = temp_lagrecord.m_angAngles.y;
		}

		lag_records.emplace_back(cur_lagrecord);
	}
}

bool BackTrack::RunTicks(C_BaseEntity* target, CUserCmd* usercmd, Vector &aim_point, bool &hitchanced)
{
	if (StartLagCompensation(target))
	{
		LagRecord cur_record;
		auto& m_LagRecords = this->m_LagRecord[target->EntIndex()];
		while (FindViableRecord(target, &cur_record))
		{
			auto iter = std::find(m_LagRecords.begin(), m_LagRecords.end(), cur_record);
			if (iter == m_LagRecords.end())
				continue;

			if (iter->m_bNoGoodSpots)
			{
				// Already awalled from same spot, don't try again like a dumbass.
				if (iter->m_vecLocalAimspot == g::pLocalEntity->GetEyePosition())
					continue;
				else
					iter->m_bNoGoodSpots = false;
			}

			if (!iter->m_bMatrixBuilt)
			{
				if (!target->SetupBones2(iter->matrix, 128, 256, iter->m_flSimulationTime))
					continue;

				iter->m_bMatrixBuilt = true;
			}

			float btReturnDamage = 0;
			aim_point = g_RageWall.CalculateBestPoint(target, 0, g_Settings.bRagebotMinDamage, false, iter->matrix, true, btReturnDamage);

			matrix3x4_t matrix[128];
			if (!target->SetupBones2(matrix, 128, 256, target->GetSimulationTime()))
				return false;

			float returnDamage = 0;
			g_RageWall.CalculateBestPoint(target, 0, g_Settings.bRagebotMinDamage, false, matrix, false, returnDamage);

			if (!aim_point.IsValid())
			{
				FinishLagCompensation(target);
				iter->m_bNoGoodSpots = true;
				iter->m_vecLocalAimspot = g::pLocalEntity->GetEyePosition();
				continue;
			}

			if (returnDamage >= btReturnDamage)
				continue;

			QAngle aimAngle = Utils::CalcAngle(g::pLocalEntity->GetEyePosition(), aim_point) - g::pLocalEntity->GetPunchAngles() * 2.f;

			hitchanced = g_RageWall.HitChance(aimAngle, target, g_Settings.bRagebotHitchanceA);

			this->current_record[target->EntIndex()] = *iter;
			break;
		}
		FinishLagCompensation(target);
		ProcessCMD(target->EntIndex(), usercmd);
		return true;
	}
	else
	{
		return false;
	}
}

int BackTrack::GetPriorityLevel(C_BaseEntity *player, LagRecord* lag_record)
{
	int priority = 0;

	if (lag_record->m_flPrevLowerBodyYaw != player->GetLowerbodyYaw())
	{
		lag_record->m_angAngles.y = player->GetLowerbodyYaw();
		priority = 3;
	}

	if (player->GetVelocity().Length2D() > 20.f)
		priority = 1;

	lag_record->m_flPrevLowerBodyYaw = player->GetLowerbodyYaw();
	lag_record->m_arrflPrevPoseParameters = player->m_flPoseParameter();

	return priority;
}

void BackTrack::ProcessCMD(int iTargetIdx, CUserCmd *usercmd)
{
	LagRecord recentLR = m_RestoreLagRecord[iTargetIdx].first;
	if (!IsTickValid(TIME_TO_TICKS(recentLR.m_flSimulationTime)))
		usercmd->tick_count = TIME_TO_TICKS(g_pEntityList->GetClientEntity(iTargetIdx)->GetSimulationTime() + GetLerpTime());
	else
		usercmd->tick_count = TIME_TO_TICKS(recentLR.m_flSimulationTime + GetLerpTime());
}

void BackTrack::RemoveBadRecords(int Idx, std::deque<LagRecord>& records)
{
	auto& m_LagRecords = records; // Should use rbegin but can't erase
	for (auto lag_record = m_LagRecords.begin(); lag_record != m_LagRecords.end(); lag_record++)
	{
		if (!IsTickValid(TIME_TO_TICKS(lag_record->m_flSimulationTime)))
		{
			m_LagRecords.erase(lag_record);
			if (!m_LagRecords.empty())
				lag_record = m_LagRecords.begin();
			else break;
		}
	}
}

bool BackTrack::StartLagCompensation(C_BaseEntity *player)
{
	backtrack_records.clear();
	auto& m_LagRecords = this->m_LagRecord[player->EntIndex()];
	m_RestoreLagRecord[player->EntIndex()].second.SaveRecord(player);

	LagRecord newest_record = LagRecord();
	for (auto it : m_LagRecords)
	{
		if (it.m_flSimulationTime > newest_record.m_flSimulationTime)
			newest_record = it;
		if (it.m_iPriority >= 1 /*&& !(it.m_nFlags & FL_ONGROUND) && it.m_vecVelocity.Length2D() > 150*/)
			backtrack_records.emplace_back(it);
	}
	backtrack_records.emplace_back(newest_record);

	std::sort(backtrack_records.begin(), backtrack_records.end(), [](LagRecord const &a, LagRecord const &b) { return a.m_iPriority > b.m_iPriority; });
	return backtrack_records.size() > 0;
}

void BackTrack::FinishLagCompensation(C_BaseEntity *player)
{
	int idx = player->EntIndex();

	player->InvalidateBoneCache();

	player->GetCollideable()->OBBMins() = m_RestoreLagRecord[idx].second.m_vecMins;
	player->GetCollideable()->OBBMaxs() = m_RestoreLagRecord[idx].second.m_vecMax;

	player->SetAbsAngles(QAngle(0, m_RestoreLagRecord[idx].second.m_angAngles.y, 0));
	player->SetAbsOrigin(m_RestoreLagRecord[idx].second.m_vecOrigin);

	//player->GetFlags2() = m_RestoreLagRecord[idx].second.m_nFlags;

	int layerCount = player->GetNumAnimOverlays();
	for (int i = 0; i < layerCount; ++i)
	{
		AnimationLayer *currentLayer = player->GetAnimOverlay(i);
		currentLayer->m_nOrder = m_RestoreLagRecord[idx].second.m_LayerRecords[i].m_nOrder;
		currentLayer->m_nSequence = m_RestoreLagRecord[idx].second.m_LayerRecords[i].m_nSequence;
		currentLayer->m_flWeight = m_RestoreLagRecord[idx].second.m_LayerRecords[i].m_flWeight;
		currentLayer->m_flCycle = m_RestoreLagRecord[idx].second.m_LayerRecords[i].m_flCycle;
	}

	player->m_flPoseParameter() = m_RestoreLagRecord[idx].second.m_arrflPoseParameters;
}

bool BackTrack::FindViableRecord(C_BaseEntity *player, LagRecord* record)
{
	auto &m_LagRecords = this->m_LagRecord[player->EntIndex()];

	// Ran out of records to check. Go back.
	if (backtrack_records.empty())
	{
		return false;
	}

	LagRecord
		recentLR = *backtrack_records.begin(),
		prevLR;

	// Should still use m_LagRecords because we're checking for LC break.
	auto iter = std::find(m_LagRecords.begin(), m_LagRecords.end(), recentLR);
	auto idx = std::distance(m_LagRecords.begin(), iter);
	if (0 != idx) prevLR = *std::prev(iter);

	// Saving first record for processcmd.
	m_RestoreLagRecord[player->EntIndex()].first = recentLR;

	if (!IsTickValid(TIME_TO_TICKS(recentLR.m_flSimulationTime)))
	{
		backtrack_records.pop_front();
		return backtrack_records.size() > 0; // RET_NO_RECORDS true false
	}

	// Remove a record...
	backtrack_records.pop_front();

	if ((0 != idx) && (recentLR.m_vecOrigin - prevLR.m_vecOrigin).LengthSqr() > 4096.f)
	{
		// Bandage fix so we "restore" to the lagfixed player.
		m_RestoreLagRecord[player->EntIndex()].second.SaveRecord(player);
		*record = m_RestoreLagRecord[player->EntIndex()].second;

		// Clear so we don't try to bt shit we can't
		backtrack_records.clear();

		return false; // Return true so we still try to aimbot.
	}
	else
	{
		player->InvalidateBoneCache();

		player->GetCollideable()->OBBMins() = recentLR.m_vecMins;
		player->GetCollideable()->OBBMaxs() = recentLR.m_vecMax;

		player->SetAbsAngles(QAngle(0, recentLR.m_angAngles.y, 0));
		player->SetAbsOrigin(recentLR.m_vecOrigin);

		//player->m_fFlags() = recentLR.m_nFlags;

		int layerCount = player->GetNumAnimOverlays();
		for (int i = 0; i < layerCount; ++i)
		{
			AnimationLayer *currentLayer = player->GetAnimOverlay(i);
			currentLayer->m_nOrder = recentLR.m_LayerRecords[i].m_nOrder;
			currentLayer->m_nSequence = recentLR.m_LayerRecords[i].m_nSequence;
			currentLayer->m_flWeight = recentLR.m_LayerRecords[i].m_flWeight;
			currentLayer->m_flCycle = recentLR.m_LayerRecords[i].m_flCycle;
		}

		player->m_flPoseParameter() = recentLR.m_arrflPoseParameters;

		*record = recentLR;
		return true;
	}
}

bool BackTrack::CheckTarget(int i)
{
	C_BaseEntity *player = g_pEntityList->GetClientEntity(i);

	if (!player || player == nullptr)
		return false;

	if (player == g::pLocalEntity)
		return false;

	if (player->GetTeam() == g::pLocalEntity->GetTeam())
		return false;

	if (player->IsDormant())
		return false;

	if (player->IsImmune())
		return false;

	if (!player->IsAlive())
		return false;

	return true;
}

template<class T, class U>
T BackTrack::clamp(T in, U low, U high)
{
	if (in <= low)
		return low;

	if (in >= high)
		return high;

	return in;
}

int realHitboxSpot[] = { 0, 1, 2, 3 };