#include "Aimbot.h"
#include <Windows.h>
#include "..\Utils\Utils.h"
#include "..\Utils\GlobalVars.h"
#include "..\Hooks.h"
#include "..\SDK\Studio.hpp"
#include "..\Utils\Interfaces.h"
#include "Backtrack.h"
#include "Ragewall.h"
#include "..\SDK\CGlobalVarsBase.h"

// Declare classes
Aimbot g_Aimbot;

// Movefix variables
float m_oldforward, m_oldsidemove;
QAngle m_oldangle;

// Aimbot variables
C_BaseEntity *target;

void Aimbot::DoAimbot(CUserCmd* pCmd)
{

	if (g_Settings.bAimbotEnable && g_Settings.bRagebotEnable)
		return;

	if (g_Settings.bRagebotEnable)
	{
		DoRageAimbot(pCmd);
		return;
	}

	if (g_Settings.bAimbotEnable)
	{
		DoLegitAimbot(pCmd);
		return;
	}
	
}

void Aimbot::DoRageAimbot(CUserCmd* pCmd) {
	// Check if player is in game
	if (!g::pLocalEntity || !g_pEngine->IsInGame())
		return;

	// Check if player is alive
	if (!g::pLocalEntity
		|| g::pLocalEntity->IsDormant()
		|| !g::pLocalEntity->IsAlive())
		return;

	// Get some other variables
	auto weapon = g::pActiveWeapon;
	if (!weapon)
		return;

	int bestHitbox = 8;

	bool isAttacking = (pCmd->buttons & IN_ATTACK || g_Settings.bRagebotAutoFire);
	if (!isAttacking)
		return;

	// Dont aimbot when we have a knife or grenade in our hands
	if (weapon->isGrenade() || weapon->GetCSWpnData()->weapon_type() == 0)
		return;

	g_RageWall.TargetEntities(pCmd);
}

void Aimbot::DoLegitAimbot(CUserCmd* pCmd) {

	// Check if player is in game
	if (!g::pLocalEntity || !g_pEngine->IsInGame())
		return;

	// Check if player is alive
	if (!g::pLocalEntity
		|| g::pLocalEntity->IsDormant()
		|| !g::pLocalEntity->IsAlive())
		return;

	// Get some other variables
	auto weapon = g::pActiveWeapon;
	if (!weapon)
		return;

	int bestHitbox = 7;

	bool isAttacking = (pCmd->buttons & IN_ATTACK);
	if (!isAttacking)
		return;

	if (weapon->isGrenade() || weapon->GetCSWpnData()->weapon_type() == 0)
		return;

	// Get a new target
	Vector targetHitbox;
	C_BaseEntity *target = GetBestTarget(targetHitbox);

	// Check if target is Null
	if (!target)
		return;

	// Check if target is behind smoke
	if (target->IsBehindSmoke(target))//&& g_Settings.bSmokeCheck)
		return;

	// TODO: Custom hitbox selection
	bestHitbox = getHitbox(weapon);

	// Let's check if wee can see the player, if we can, we aim.
	CGameTrace tr;
	g_RageWall.traceIt(g::pLocalEntity->GetEyePosition(), target->GetBonePos(bestHitbox), MASK_SHOT | CONTENTS_GRATE, g::pLocalEntity, &tr);

	if (tr.fraction > 0.97f && tr.fraction != 1.f)
		AimAt(pCmd, target, bestHitbox);

}


bool isLocked = false;
C_BaseEntity* Aimbot::GetBestTarget(Vector& outBestPos)
{
	// Init some variables
	C_BaseEntity* target = nullptr;
	int curBestTarget;
	float currentFov;
	float weaponFov;
	float bestFov = 360;
	float bestDistance = 8128.f;

	// Variables we need later
	auto weapon = g::pActiveWeapon;
	if (!weapon)
		return nullptr;
	auto localTeam = g::pLocalEntity->GetTeam();

	if (!isLocked && g::bestTarget == 0)
	{
		// Itterate through all the players in the server
		for (int i = 1; i <= g_pEngine->GetMaxClients(); ++i)
		{
			// Get the entity from the player we are currently itterating on
			C_BaseEntity* pPlayerEntity = g_pEntityList->GetClientEntity(i);

			// Various checks
			if (!pPlayerEntity
				|| !pPlayerEntity->IsAlive()
				|| pPlayerEntity->IsDormant()
				|| pPlayerEntity == g::pLocalEntity
				|| pPlayerEntity->GetTeam() == localTeam)
				continue;

			// Get Fov for each weapon
			if (g_Settings.bAimbotEnable)
				weaponFov = getFov(weapon);
			else
				weaponFov = g_Settings.bRagebotFov;

			// Get positions from me and target
			Vector myPos = g::pLocalEntity->GetBonePos(8);
			Vector pEntPos = pPlayerEntity->GetBonePos(8);

			// Calculate angle between me and target
			QAngle aimAngle = Utils::CalcAngle(myPos, pEntPos);

			// Calculate recoil values to compensate aimbot
			currentFov = get_fov(g::pCmd->viewangles + g::pLocalEntity->GetPunchAngles() * 2, aimAngle);

			// Get Distance between me and target
			auto currentDistance = Get3D_Distance(myPos, pEntPos);

			// Check if the target is closer to the crosshair and if the target is in range of FOV
			if (currentFov < weaponFov && (bestFov == NULL || currentFov < bestFov))
			{
				bestFov = currentFov;
				curBestTarget = i;
				target = pPlayerEntity;
			}

			// Check for distance also
			if (currentFov <= weaponFov && currentDistance < bestDistance && (currentFov < bestFov || bestFov == NULL))
			{	
				bestFov = currentFov;
				curBestTarget = i;
				target = pPlayerEntity;
				bestDistance = currentDistance;
			}
		}
		if (g::bestTarget > 0)
		{
			isLocked = true;
			g::bestTarget = curBestTarget;
		}
	}
	if (g::bestTarget > 0)
		return g_pEntityList->GetClientEntity(g::bestTarget);
	else
		return target;
}

bool Aimbot::AutoShoot(CUserCmd* pCmd, C_BaseEntity* BestTarget)
{
	C_BaseCombatWeapon* pWeapon = g::pActiveWeapon;

	if (!pWeapon)
		return false;

	float flServerTime = g::pLocalEntity->GetTickBase() * g_pGlobalVars->intervalPerTick;
	bool canShoot = !(pWeapon->GetNextPrimaryAttack() > flServerTime) && !(pCmd->buttons & IN_RELOAD);

	if (Hitchance(pWeapon, 20) > 20 && !(pCmd->buttons & IN_ATTACK) && canShoot)
		pCmd->buttons |= IN_ATTACK;
	else
		return false;

	return true;
}

float Aimbot::Hitchance(C_BaseCombatWeapon* pWeapon, float hitChance)
{
	float hitchance = 101;
	if (!pWeapon) return 0;
	if (hitChance > 0)
	{
		float inaccuracy = pWeapon->GetInaccuracy();
		if (inaccuracy == 0) inaccuracy = 0.0000001f;
		inaccuracy = 1 / inaccuracy;
		hitchance = inaccuracy;
	}
	return hitchance;
}

/*
bool Aimbot::CanHitTarget(C_BaseEntity* pTarget)
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
		if (g_RageWall.CanHit(pTargetPos))
			return true;
	}
	return false;
}*/

float Aimbot::getFov(C_BaseCombatWeapon* weapon) {
	if (weapon->isSniper())
		return g_Settings.bAimbotFovSniper;
	if (weapon->isRifle())
		return g_Settings.bAimbotFovRifle;
	if (weapon->isPistol())
		return g_Settings.bAimbotFovPistol;
	return g_Settings.bAimbotFovRifle;
}

float Aimbot::getSmooth(C_BaseCombatWeapon* weapon) {
	if (weapon->isSniper())
		return g_Settings.bAimbotSmoothSniper;
	if (weapon->isRifle())
		return g_Settings.bAimbotSmoothRifle;
	if (weapon->isPistol())
		return g_Settings.bAimbotSmoothPistol;
	return g_Settings.bAimbotFovRifle;
}

int Aimbot::getHitbox(C_BaseCombatWeapon* weapon) {
	int curSelected = 3;
	if (weapon->isSniper())
		curSelected = g_Settings.bAimbotHitboxSniper;
	if (weapon->isRifle())
		curSelected = g_Settings.bAimbotHitboxRifle;
	if (weapon->isPistol())
		curSelected = g_Settings.bAimbotHitboxPistol;

	switch (curSelected)
	{
	case 0: return 8;
	case 1: return 4;
	case 2: return 6;
	default: return 8;
	}
}

void Aimbot::AimAtVec(CUserCmd* pCmd, C_BaseEntity* pEnt, Vector hitbox)
{
	if (!g::pLocalEntity || !g_pEngine->IsInGame())
		return;

	auto weapon = g::pActiveWeapon;
	if (!weapon)
		return;

	if (weapon->GetAmmo() == 0 || pCmd->buttons & IN_RELOAD)
		return;

	auto localTeam = g::pLocalEntity->GetTeam();

	if (!pEnt
		|| !pEnt->IsAlive()
		|| pEnt->IsDormant()
		|| pEnt == g::pLocalEntity
		|| pEnt->GetTeam() == localTeam
		|| pEnt->IsImmune())
		return;

	Vector pHitboxServerDistance;
	bool doBacktrack = false;

	QAngle tempAimAngle = Utils::CalcAngle(g::pLocalEntity->GetBonePos(8), pEnt->GetBonePos(8));
	float bestFov = get_fov(pCmd->viewangles, tempAimAngle);
	float curWeaponFov = getFov(g::pActiveWeapon);

	Vector actualHitBox = hitbox;
	Vector myPos = g::pLocalEntity->GetEyePosition();

	// If we have a better tick to aim at, move our target to that tick
	if (doBacktrack)
		actualHitBox -= pHitboxServerDistance;

	Vector pEntPos = actualHitBox;

	// Velocity Compensation
	pEntPos += pEnt->GetVelocity() * g_pGlobalVars->intervalPerTick;

	// Get angle we are supposed to aim at
	QAngle aimAngle = Utils::CalcAngle(myPos, pEntPos);
	if (g::pLocalEntity->GetPunchAngles().Length() > 0)
		aimAngle -= g::pLocalEntity->GetPunchAngles() * 2;

	// Make sure we dont aim out of bounds otherwise we'll get untrusted
	Utils::ClampViewAngles(aimAngle);

	// Get the difference between aim angle and your viewangle
	QAngle deltaAngle = pCmd->viewangles - aimAngle;

	// Make sure we dont aim out of bounds otherwise we'll get untrusted
	Utils::ClampViewAngles(deltaAngle);

	// Get the fov
	auto fov = get_fov(pCmd->viewangles + g::pLocalEntity->GetPunchAngles() * 2, aimAngle);

	QAngle finalAngle;

	finalAngle = pCmd->viewangles - deltaAngle;
	Utils::ClampViewAngles(finalAngle);

	float currentFov;

	currentFov = g_Settings.bRagebotFov;

	if (fov <= currentFov)
	{
		if (!g_Settings.bRagebotSilent)
		{
			g_pEngine->SetViewAngles(finalAngle);
		}
		pCmd->viewangles = finalAngle;
	}
}

void Aimbot::AimAt(CUserCmd* pCmd, C_BaseEntity* pEnt, int hitbox)
{
	if (!g::pLocalEntity || !g_pEngine->IsInGame())
		return;

	auto weapon = g::pActiveWeapon;
	if (!weapon)
		return;

	if (weapon->GetAmmo() == 0 || pCmd->buttons & IN_RELOAD)
		return;

	auto localTeam = g::pLocalEntity->GetTeam();

	if (!pEnt
		|| !pEnt->IsAlive()
		|| pEnt->IsDormant()
		|| pEnt == g::pLocalEntity
		|| pEnt->GetTeam() == localTeam
		|| pEnt->IsImmune())
		return;

	Vector pHitboxServerDistance;
	bool doBacktrack = false;

	QAngle tempAimAngle = Utils::CalcAngle(g::pLocalEntity->GetBonePos(8), pEnt->GetBonePos(8));
	float bestFov = get_fov(pCmd->viewangles, tempAimAngle);
	float curWeaponFov = getFov(g::pActiveWeapon);

	if (g_Settings.bAimbotBacktrack && g_Settings.bAimbotEnable)
	{
		// Loop through backtracking ticks
		for (int i = 0; i < g_Settings.bAimbotBacktrackTicks; i++)
		{
			Vector pHitboxPos = l_SavedTicks[pEnt->EntIndex()][i].hitboxPos;
			QAngle pHitboxAngle = Utils::CalcAngle(g::pLocalEntity->GetBonePos(8), pHitboxPos);

			auto newFov = g_Aimbot.get_fov(pCmd->viewangles, pHitboxAngle);
			if (l_SavedTicks[pEnt->EntIndex()][i].simtime <= g::pLocalEntity->GetSimulationTime() - 1)
				continue;

			// Is our fov closer to the tick we are currently itterating on?
			if (newFov < curWeaponFov && newFov < bestFov)
			{
				bestFov = newFov;
				doBacktrack = true;
				pHitboxServerDistance = Vector(sqrt((pHitboxPos.x - pEnt->GetBonePos(8).x) * (pHitboxPos.x - pEnt->GetBonePos(8).x)), sqrt((pHitboxPos.y - pEnt->GetBonePos(8).y) * (pHitboxPos.y - pEnt->GetBonePos(8).y)), 0);
			}
		}
	}
	  
	Vector actualHitBox = pEnt->GetBonePos(hitbox);
	Vector myPos = g::pLocalEntity->GetEyePosition();

	// If we have a better tick to aim at, move our target to that tick
	if (doBacktrack)
		actualHitBox -= pHitboxServerDistance;

	Vector pEntPos = actualHitBox;

	// Velocity Compensation
	pEntPos += pEnt->GetVelocity() * g_pGlobalVars->intervalPerTick;

	// Get angle we are supposed to aim at
	QAngle aimAngle = Utils::CalcAngle(myPos, pEntPos);
	if (g::pLocalEntity->GetPunchAngles().Length() > 0)
		aimAngle -= g::pLocalEntity->GetPunchAngles() * 2;

	// Make sure we dont aim out of bounds otherwise we'll get untrusted
	Utils::ClampViewAngles(aimAngle);

	// Get the difference between aim angle and your viewangle
	QAngle deltaAngle = pCmd->viewangles - aimAngle;

	// Make sure we dont aim out of bounds otherwise we'll get untrusted
	Utils::ClampViewAngles(deltaAngle);

	// Get the fov
	auto fov = get_fov(pCmd->viewangles + g::pLocalEntity->GetPunchAngles() * 2, aimAngle);

	QAngle finalAngle;

	// TODO: Custom smoothing based on settings / weapon here
	float currentSmooth;
	if (g_Settings.bAimbotEnable)
		currentSmooth = getSmooth(weapon);
	else
		currentSmooth = 1.f;
	

	finalAngle = pCmd->viewangles - deltaAngle / currentSmooth;
	Utils::ClampViewAngles(finalAngle);

	float currentFov;
		
	if (g_Settings.bAimbotEnable)
		currentFov = getFov(weapon);
	else
		currentFov = g_Settings.bRagebotFov;

	if (fov <= currentFov)
	{
		if (!g_Settings.bRagebotSilent)
		{
			g_pEngine->SetViewAngles(finalAngle);
		}
		pCmd->viewangles = finalAngle;
	}
}

float Aimbot::Get3D_Distance(Vector src, Vector dst) {
	return sqrt(powf(src[0] - dst[0], 2.f) + powf(src[1] - dst[1], 2.f) + powf(src[2] - dst[2], 2.f));
}

void Aimbot::MakeVector(QAngle angle, Vector& vector)
{
	float pitch = float(angle[0] * PI / 180);
	float yaw = float(angle[1] * PI / 180);
	float tmp = float(cos(pitch));
	vector[0] = float(-tmp * -cos(yaw));
	vector[1] = float(sin(yaw)*tmp);
	vector[2] = float(-sin(pitch));
}

float Aimbot::get_fov(const QAngle &viewAngles, const QAngle &aimAngles)
{
	Vector ang, aim;
	MakeVector(viewAngles, aim);
	MakeVector(aimAngles, ang);
	return RAD2DEG(acos(aim.Dot(ang) / aim.LengthSqr()));
}
