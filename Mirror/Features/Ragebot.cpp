#include "Ragebot.h"
#include "Aimbot.h"
#include "Autowall.h"
#include <Windows.h>
#include "..\Utils\Utils.h"
#include "..\Utils\GlobalVars.h"
#include "..\Hooks.h"
#include "..\SDK\Studio.hpp"
#include "..\Utils\Interfaces.h"
#include "Backtrack.h"

Ragebot g_Ragebot;

// Movefix variables
float m_oldforward2, m_oldsidemove2;
QAngle m_oldangle2;

void Ragebot::DoRagebot()
{
	// Is Ragebot on?
	if (!g_Settings.bRagebotEnable || g_Settings.bAimbotEnable)
		return;

	// Check if player is in game
	if (!g::pLocalEntity || !g_pEngine->IsInGame())
		return;

	// Check if player is alive
	if (!g::pLocalEntity
		|| g::pLocalEntity->IsDormant()
		|| !g::pLocalEntity->IsAlive())
		return;

	// Get some other variables
	auto weapon = g::pLocalEntity->GetActiveWeapon();

	C_BaseEntity* bestTarget = nullptr;
	Vector hitboxPos;

	GetBestTarget(g::pCmd, bestTarget);

	if (!bestTarget)
		return;

	if (!g_Ragebot.Hitscan(bestTarget, hitboxPos));
	return;

	AimAt(g::pCmd, bestTarget, hitboxPos);
	AutoShoot(g::pCmd);
}

bool Ragebot::AimAt(CUserCmd* pCmd, C_BaseEntity* pEnt, Vector hitboxPos)
{
	if (!TargetMeetsRequirments(pEnt))
		return false;
	Vector localPlayerPosition = g::pLocalEntity->GetEyePosition();
	QAngle aimAngle = Utils::CalcAngle(localPlayerPosition, hitboxPos);

	if (g::pLocalEntity->GetPunchAngles().Length() > 0)
		aimAngle -= g::pLocalEntity->GetPunchAngles() * 2;

	Utils::ClampViewAngles(aimAngle);

	pCmd->viewangles = aimAngle;

	return true;
}

bool Ragebot::AutoShoot(CUserCmd* pCmd)
{
	C_BaseCombatWeapon* pWeapon = g::pLocalEntity->GetActiveWeapon();

	if (!pWeapon)
		return false;

	float flServerTime = g::pLocalEntity->GetTickBase() * g_pGlobalVars->intervalPerTick;
	bool canShoot = !(pWeapon->GetNextPrimaryAttack() > flServerTime) && !(pCmd->buttons & IN_RELOAD);

	C_BaseEntity* outBestTarget = nullptr;
	GetBestTarget(pCmd, outBestTarget);

	if (!outBestTarget)
		return false;

	if (Hitchance(pWeapon, 20) > 20 && !(pCmd->buttons & IN_ATTACK) && canShoot && CanHitTarget(outBestTarget))
		pCmd->buttons |= IN_ATTACK;
	else
		return false;

	return true;

}

float Ragebot::Hitchance(C_BaseCombatWeapon* pWeapon, float hitChance)
{
	float hitchance = 101;
	if (!pWeapon) return 0;
	if (hitChance > 0)
	{
		float inaccuracy = pWeapon->GetInaccuracy();
		if (inaccuracy == 0) inaccuracy = 0.0000001;
		inaccuracy = 1 / inaccuracy;
		hitchance = inaccuracy;
	}
	return hitchance;
}

bool TargetIsLocked = false;
C_BaseEntity* lockedTarget;

bool Ragebot::GetBestTarget(CUserCmd* pCmd, C_BaseEntity* outBestTarget)
{
	float fov;
	float bestFov = FLT_MAX;
	bool findNewTarget = true;
	C_BaseEntity* bestTarget = nullptr;

	// Check if we have a locket target
	if (TargetIsLocked && TargetMeetsRequirments(lockedTarget))
		findNewTarget = false;

	if (findNewTarget)
	{
		for (int it = 1; it <= g_pEngine->GetMaxClients(); ++it)
		{
			C_BaseEntity* pPlayerEntity = g_pEntityList->GetClientEntity(it);
			if (!TargetMeetsRequirments(pPlayerEntity))
				return false;
			Vector myPos = g::pLocalEntity->GetBonePos(8);
			Vector pEntPos = pPlayerEntity->GetBonePos(8);
			QAngle aimAngle = Utils::CalcAngle(myPos, pEntPos);
			fov = g_Aimbot.get_fov(pCmd->viewangles + g::pLocalEntity->GetPunchAngles() * 2, aimAngle);
			Vector targetPos;

			if (!CanHitTarget(pPlayerEntity));
			return false;

			if (fov < bestFov)
				bestTarget = pPlayerEntity;
		}
		if (bestTarget)
		{
			TargetIsLocked = true;
			lockedTarget = bestTarget;
		}
	}

	outBestTarget = lockedTarget;
	if (!lockedTarget)
		return false;

	if (lockedTarget)
		true;
}

bool Ragebot::CanHitTarget(C_BaseEntity* pTarget)
{
	Vector newShittyDistance;
	bool hasPenetratedWall = false;
	Vector bestHitboxPos;
	auto pEnt = pTarget;
	Vector hitboxPos;
	studiohdr_t* pStudioHdr = g_pMdlInfo->GetStudiomodel2(pEnt->GetModel());
	std::vector<int> hitboxes;
	if (!pStudioHdr)
		return false;

	Vector vParent, vChild, sParent, sChild;
	for (int j = 0; j < pStudioHdr->numbones; j++)
	{
		mstudiobone_t* pBone = pStudioHdr->GetBone(j);
		if (pBone && (pBone->flags & BONE_USED_BY_HITBOX) && (pBone->parent != -1))
		{
			hitboxes.push_back(j);
			hitboxes.push_back(pBone->parent);
		}
	}

	for (const int &hitbox : hitboxes)
	{
		Vector pTargetPos = pTarget->GetBonePos(hitbox);
		if (new_autowall.CanHit(NewExtrapolate(pTargetPos, pTarget->GetVelocity())))
			return true;
	}
	return false;
}

Vector Ragebot::NewExtrapolate(Vector currentPos, Vector Velocity)
{
	return currentPos + (Velocity * g_pGlobalVars->intervalPerTick);
}

bool Ragebot::TargetMeetsRequirments(C_BaseEntity* target)
{
	if (target && target->IsAlive() && !target->IsDormant() && target->GetTeam() != g::pLocalEntity->GetTeam())
		return true;
	else
		return false;
}

void Ragebot::StartMoveFix()
{
	m_oldangle2 = g::pCmd->viewangles;
	m_oldforward2 = g::pCmd->forwardmove;
	m_oldsidemove2 = g::pCmd->sidemove;
}

void Ragebot::EndMoveFix()
{
	float yaw_delta = g::pCmd->viewangles.y - m_oldangle2.y;
	float f1;
	float f2;

	if (m_oldangle2.y < 0.f)
		f1 = 360.0f + m_oldangle2.y;
	else
		f1 = m_oldangle2.y;

	if (g::pCmd->viewangles.y < 0.0f)
		f2 = 360.0f + g::pCmd->viewangles.y;
	else
		f2 = g::pCmd->viewangles.y;

	if (f2 < f1)
		yaw_delta = abs(f2 - f1);
	else
		yaw_delta = 360.0f - abs(f1 - f2);
	yaw_delta = 360.0f - yaw_delta;
	g::pCmd->forwardmove = cos(DEG2RAD(yaw_delta)) * m_oldforward2 + cos(DEG2RAD(yaw_delta + 90.f)) * m_oldsidemove2;
	g::pCmd->sidemove = sin(DEG2RAD(yaw_delta)) * m_oldforward2 + sin(DEG2RAD(yaw_delta + 90.f)) * m_oldsidemove2;
}

bool Ragebot::Hitscan(C_BaseEntity* pTarget, Vector& hitboxPos)
{
	Vector newShittyDistance;
	bool hasPenetratedWall = false;
	int bestDamage = 0;
	Vector bestHitboxPos;
	auto pEnt = pTarget;
	studiohdr_t* pStudioHdr = g_pMdlInfo->GetStudiomodel2(pEnt->GetModel());
	std::vector<int> hitboxes;
	float ActualDamage;
	if (!pStudioHdr)
		return false;
	Vector vParent, vChild, sParent, sChild;

	for (int j = 0; j < pStudioHdr->numbones; j++)
	{
		mstudiobone_t* pBone = pStudioHdr->GetBone(j);
		if (pBone && (pBone->flags & BONE_USED_BY_HITBOX) && (pBone->parent != -1))
		{
			hitboxes.push_back(j);
			hitboxes.push_back(pBone->parent);
		}
	}

	for (const int &hitbox : hitboxes)
	{
		Vector pTargetPos = pTarget->GetBonePos(hitbox);
		ActualDamage = new_autowall.CanHit(NewExtrapolate(pTargetPos, pTarget->GetVelocity()));
		if (ActualDamage > pTarget->GetHealth())
		{
			hitboxPos = pTargetPos;
			return true;
		}
		if (ActualDamage > bestDamage)
		{
			hasPenetratedWall = true;
			bestDamage = ActualDamage;
			bestHitboxPos = pTargetPos;
		}
	}

	hitboxPos = bestHitboxPos;
	return hasPenetratedWall;
}