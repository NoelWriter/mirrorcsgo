#pragma once
#include "..\Utils\DrawManager.h"
#include "..\Utils\GlobalVars.h"
#include "..\Settings.h"

class Aimbot
{
public:
	void Aimbot::DoAimbot();
	C_BaseEntity * GetBestTarget(Vector & outBestPos);
	float getFov(C_BaseCombatWeapon * weapon);
	float getSmooth(C_BaseCombatWeapon * weapon);
	int getHitbox(C_BaseCombatWeapon * weapon);
	void AimAt(CUserCmd * pCmd, C_BaseEntity * pEnt, int hitbox);
	float Get3D_Distance(Vector src, Vector dst);
	void MakeVector(QAngle angle, Vector & vector);
	float get_fov(const QAngle & viewAngles, const QAngle & aimAngles);
	void Aimbot::StartMoveFix();
	void Aimbot::EndMoveFix();
private:
};


class Ragebot
{
public:
	bool Ragebot::Hitscan(C_BaseEntity* pTarget , Vector& hitboxPos);
};
extern Aimbot g_Aimbot;

extern Ragebot g_Ragebot;
