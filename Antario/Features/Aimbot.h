#pragma once
#include "..\Utils\DrawManager.h"
#include "..\Utils\GlobalVars.h"
#include "..\Settings.h"

class Aimbot
{
public:
	void DoAimbot(CUserCmd * pCmd);
	void DoRageAimbot(CUserCmd * pCmd);
	void DoLegitAimbot(CUserCmd * pCmd);
	bool CanHitTarget(C_BaseEntity * pTarget);
	C_BaseEntity * GetBestTarget(Vector & outBestPos);
	bool AutoShoot(CUserCmd * pCmd, C_BaseEntity * BestTarget);
	float Hitchance(C_BaseCombatWeapon * pWeapon, float hitChance);
	float getFov(C_BaseCombatWeapon * weapon);
	float getSmooth(C_BaseCombatWeapon * weapon);
	int getHitbox(C_BaseCombatWeapon * weapon);
	void AimAtVec(CUserCmd * pCmd, C_BaseEntity * pEnt, Vector hitbox);
	void AimAt(CUserCmd * pCmd, C_BaseEntity * pEnt, int hitbox);
	float Get3D_Distance(Vector src, Vector dst);
	void MakeVector(QAngle angle, Vector & vector);
	float get_fov(const QAngle & viewAngles, const QAngle & aimAngles);
	void Aimbot::StartMoveFix();
	void Aimbot::EndMoveFix();
private:
};

extern Aimbot g_Aimbot;

