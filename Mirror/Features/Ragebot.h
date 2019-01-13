#pragma once
#include "..\Utils\DrawManager.h"
#include "..\Utils\GlobalVars.h"
#include "..\Settings.h"

class RagebotInit
{
public:
	void DoRagebot(CUserCmd * pCmd);
};

class Ragebot
{
public:
	bool AimAt(CUserCmd * pCmd, C_BaseEntity * pEnt, Vector hitboxPos);
	bool AutoShoot(CUserCmd * pCmd);
	float Hitchance(C_BaseCombatWeapon * pWeapon, float hitChance);
	bool Ragebot::GetBestTarget(CUserCmd * pCmd, C_BaseEntity * outBestTarget);
	bool CanHitTarget(C_BaseEntity * pTarget);
	Vector NewExtrapolate(Vector currentPos, Vector Velocity);
	bool TargetMeetsRequirments(C_BaseEntity * target);
	void StartMoveFix();
	void EndMoveFix();
	bool Hitscan(C_BaseEntity * pTarget, Vector & hitboxPos);
};


extern RagebotInit g_Ragebot;
extern Ragebot g_Ragebots;