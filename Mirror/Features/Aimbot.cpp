#include "Aimbot.h"
#include <Windows.h>
#include "..\Utils\Utils.h"
#include "..\Utils\GlobalVars.h"
#include "..\Hooks.h"
#include "..\SDK\Studio.hpp"
#include "..\Utils\Interfaces.h"
#include "Backtrack.h"

// Define some constant variables
#define PI 3.14159265358979323846264338327f

// Declare classes
Aimbot g_Aimbot;

// Movefix variables
float m_oldforward, m_oldsidemove;
QAngle m_oldangle;

// Aimbot variables
C_BaseEntity *target;


void Aimbot::DoAimbot()
{
	// Is Aimbot on?
	if (!g_Settings.bAimbotEnable || g_Settings.bRagebotEnable)
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
	int bestHitbox = 7;

	bool isAttacking = (g::pCmd->buttons & IN_ATTACK);
	if (!isAttacking)
		return;

	StartMoveFix();

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
	if (g::pLocalEntity->CanSeePlayer(target, target->GetBonePos(bestHitbox)))
		AimAt(g::pCmd, target, bestHitbox);

	EndMoveFix();
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
	auto weapon = g::pLocalEntity->GetActiveWeapon();
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

			// TODO: Add fov based on settings / weapon here
			weaponFov = getFov(weapon);

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
	int curSelected;
	if (weapon->isSniper())
		curSelected = g_Settings.bAimbotHitboxSniper;
	if (weapon->isRifle())
		curSelected = g_Settings.bAimbotHitboxRifle;
	if (weapon->isPistol())
		curSelected = g_Settings.bAimbotHitboxSniper;

	switch (curSelected)
	{
	case 0: return 8;
	case 1: return 4;
	case 2: return 6;
	default: return 8;
	}
}

void Aimbot::AimAt(CUserCmd* pCmd, C_BaseEntity* pEnt, int hitbox)
{
	if (!g::pLocalEntity || !g_pEngine->IsInGame())
		return;

	auto weapon = g::pLocalEntity->GetActiveWeapon();
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
	float curWeaponFov = getFov(g::pLocalEntity->GetActiveWeapon());


	if (g_Settings.bAimbotBacktrack)
	{
		// Loop through backtracking ticks
		for (int i = 0; i < g_Settings.bAimbotBacktrackTicks; i++)
		{
			Vector pHitboxPos = headPositions[pEnt->EntIndex()][i].hitboxPos;
			QAngle pHitboxAngle = Utils::CalcAngle(g::pLocalEntity->GetBonePos(8), pHitboxPos);

			auto newFov = g_Aimbot.get_fov(g::pCmd->viewangles, pHitboxAngle);
			if (headPositions[pEnt->EntIndex()][i].simtime <= g::pLocalEntity->GetSimulationTime() - 1)
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
	auto currentSmooth = getSmooth(weapon);

	finalAngle = pCmd->viewangles - deltaAngle / currentSmooth;
	Utils::ClampViewAngles(finalAngle);
	auto currentFov = getFov(weapon);

	if (fov <= currentFov)
	{
		g_pEngine->SetViewAngles(finalAngle);
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

void Aimbot::StartMoveFix()
{
	m_oldangle = g::pCmd->viewangles;
	m_oldforward = g::pCmd->forwardmove;
	m_oldsidemove = g::pCmd->sidemove;
}

void Aimbot::EndMoveFix()
{
	float yaw_delta = g::pCmd->viewangles.y - m_oldangle.y;
	float f1;
	float f2;

	if (m_oldangle.y < 0.f)
		f1 = 360.0f + m_oldangle.y;
	else
		f1 = m_oldangle.y;

	if (g::pCmd->viewangles.y < 0.0f)
		f2 = 360.0f + g::pCmd->viewangles.y;
	else
		f2 = g::pCmd->viewangles.y;

	if (f2 < f1)
		yaw_delta = abs(f2 - f1);
	else
		yaw_delta = 360.0f - abs(f1 - f2);
	yaw_delta = 360.0f - yaw_delta;
	g::pCmd->forwardmove = cos(DEG2RAD(yaw_delta)) * m_oldforward + cos(DEG2RAD(yaw_delta + 90.f)) * m_oldsidemove;
	g::pCmd->sidemove = sin(DEG2RAD(yaw_delta)) * m_oldforward + sin(DEG2RAD(yaw_delta + 90.f)) * m_oldsidemove;
}
