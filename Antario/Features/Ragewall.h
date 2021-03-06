#pragma once
#include "..\Utils\DrawManager.h"
#include "..\Utils\GlobalVars.h"
#include "..\Settings.h"


struct FireBulletData
{
	Vector src;
	CGameTrace enter_trace;
	Vector direction;
	CTraceFilter filter;
	float trace_length;
	float trace_length_remaining;
	float current_damage;
	int penetrate_count;
};

class RageWall
{
public:
	float BestHitPoint(C_BaseEntity * player, int prioritized, float minDmg, mstudiohitboxset_t * hitset, matrix3x4_t matrix[], Vector & vecOut, bool fromBacktrack);

	float GetDamageVec(const Vector & vecPoint, C_BaseEntity * player, int hitbox);

	void traceIt(Vector & vecAbsStart, Vector & vecAbsEnd, unsigned int mask, C_BaseEntity * ign, CGameTrace * tr);

	bool SimulateFireBullet(C_BaseCombatWeapon * weap, FireBulletData & data, C_BaseEntity * player, int hitbox);

	bool HandleBulletPenetration(WeaponInfo_t * wpn_data, FireBulletData & data);

	bool TraceToExit(Vector & end, CGameTrace * enter_trace, Vector start, Vector dir, CGameTrace * exit_trace);

	bool IsBreakableEntity(C_BaseEntity * ent);

	void ScaleDamage(int hitgroup, C_BaseEntity * player, float weapon_armor_ratio, float & current_damage);

	bool IsArmored(C_BaseEntity * player, int armorVal, int hitgroup);

	void ClipTraceToPlayers(const Vector & vecAbsStart, const Vector & vecAbsEnd, unsigned int mask, ITraceFilter * filter, CGameTrace * tr);

	void TargetEntities(CUserCmd * pCmd);

	bool TargetSpecificEnt(C_BaseEntity * pEnt, CUserCmd * pCmd);

	bool HitChance(QAngle angles, C_BaseEntity * ent, float chance);

	bool CheckTarget(int i);

	Vector CalculateBestPoint(C_BaseEntity * player, int prioritized, float minDmg, bool onlyPrioritized, matrix3x4_t matrix[], bool fromBacktrack, float & returnDamage);

	bool IsEntityMoving(C_BaseEntity * player);

	void AutoStop();

	int prev_aimtarget = NULL;
	int switchTick = 0;
	bool inStop = false;
};
extern RageWall g_RageWall;