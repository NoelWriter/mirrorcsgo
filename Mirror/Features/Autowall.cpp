#include "autowall.h"
#include "Math.h"
#include "../SDK/IEngineTrace.hpp"
#include "../SDK/IPhysics.h"
#include "../Utils/ICvar.h"
#include "Aimbot.h"
#include <Windows.h>
#include "..\Utils\Utils.h"
#include "..\Utils\GlobalVars.h"
#include "..\Hooks.h"
#include "..\SDK\Studio.hpp"
#include "Backtrack.h"
#include "..\SDK\CGlobalVarsBase.h"
#include "..\SDK\ClientClass.h"

#define    HITGROUP_GENERIC    0
#define    HITGROUP_HEAD        1
#define    HITGROUP_CHEST        2
#define    HITGROUP_STOMACH    3
#define HITGROUP_LEFTARM    4    
#define HITGROUP_RIGHTARM    5
#define HITGROUP_LEFTLEG    6
#define HITGROUP_RIGHTLEG    7
#define HITGROUP_GEAR        10
#define PI 3.14159265358979323846f
#define TIME_TO_TICKS(dt) ((int)( 0.5f + (float)(dt) / g_pGlobalVars->intervalPerTick))

void SinCos2634(float a, float* s, float*c)
{
	*s = sin(a);
	*c = cos(a);
}

void CalcAngle234(Vector src, Vector dst, QAngle &angles)
{
	Vector delta = src - dst;
	double hyp = delta.Length2D();
	angles.y = (atan(delta.y / delta.x) * 57.295779513082f);
	angles.x = (atan(delta.z / hyp) * 57.295779513082f);
	angles[2] = 0.00;

	if (delta.x >= 0.0)
		angles.y += 180.0f;
}

void VectorAngles234(Vector forward, QAngle &angles)
{
	float tmp, yaw, pitch;

	if (forward[2] == 0 && forward[0] == 0)
	{
		yaw = 0;

		if (forward[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180 / PI);

		if (yaw < 0)
			yaw += 360;

		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(-forward[2], tmp) * 180 / PI);

		if (pitch < 0)
			pitch += 360;
	}

	if (pitch > 180)
		pitch -= 360;
	else if (pitch < -180)
		pitch += 360;

	if (yaw > 180)
		yaw -= 360;
	else if (yaw < -180)
		yaw += 360;

	if (pitch > 89)
		pitch = 89;
	else if (pitch < -89)
		pitch = -89;

	if (yaw > 180)
		yaw = 180;
	else if (yaw < -180)
		yaw = -180;

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}

void AngleVectors234(const QAngle &angles, Vector forward)
{
	Assert(s_bMathlibInitialized);
	Assert(forward);

	float	sp, sy, cp, cy;

	sy = sin(DEG2RAD(angles[1]));
	cy = cos(DEG2RAD(angles[1]));

	sp = sin(DEG2RAD(angles[0]));
	cp = cos(DEG2RAD(angles[0]));

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}

void AngleVectors(const QAngle &angles, Vector& forward, Vector& right, Vector& up)
{
	float sr, sp, sy, cr, cp, cy;

	SinCos2634(DEG2RAD(angles[1]), &sy, &cy);
	SinCos2634(DEG2RAD(angles[0]), &sp, &cp);
	SinCos2634(DEG2RAD(angles[2]), &sr, &cr);

	forward.x = (cp * cy);
	forward.y = (cp * sy);
	forward.z = (-sp);
	right.x = (-1 * sr * sp * cy + -1 * cr * -sy);
	right.y = (-1 * sr * sp * sy + -1 * cr *  cy);
	right.z = (-1 * sr * cp);
	up.x = (cr * sp * cy + -sr * -sy);
	up.y = (cr * sp * sy + -sr * cy);
	up.z = (cr * cp);
}

typedef CGameTrace trace_t;

inline bool CGameTrace::DidHitWorld() const
{
	return hit_entity->EntIndex() == 0;
}
inline bool CGameTrace::DidHitNonWorldEntity() const
{
	return hit_entity != NULL && !DidHitWorld();
}

RageWall g_RageWall;

#define HITGROUP_GENERIC    0
#define HITGROUP_HEAD        1
#define HITGROUP_CHEST        2
#define HITGROUP_STOMACH    3
#define HITGROUP_LEFTARM    4    
#define HITGROUP_RIGHTARM    5
#define HITGROUP_LEFTLEG    6
#define HITGROUP_RIGHTLEG    7
#define HITGROUP_GEAR        10
#define DAMAGE_NO		0
#define DAMAGE_EVENTS_ONLY	1	
#define DAMAGE_YES		2
#define DAMAGE_AIM		3
#define CHAR_TEX_ANTLION		'A'
#define CHAR_TEX_BLOODYFLESH	'B'
#define	CHAR_TEX_CONCRETE		'C'
#define CHAR_TEX_DIRT			'D'
#define CHAR_TEX_EGGSHELL		'E' ///< the egg sacs in the tunnels in ep2.
#define CHAR_TEX_FLESH			'F'
#define CHAR_TEX_GRATE			'G'
#define CHAR_TEX_ALIENFLESH		'H'
#define CHAR_TEX_CLIP			'I'
#define CHAR_TEX_PLASTIC		'L'
#define CHAR_TEX_METAL			'M'
#define CHAR_TEX_SAND			'N'
#define CHAR_TEX_FOLIAGE		'O'
#define CHAR_TEX_COMPUTER		'P'
#define CHAR_TEX_SLOSH			'S'
#define CHAR_TEX_TILE			'T'
#define CHAR_TEX_CARDBOARD		'U'
#define CHAR_TEX_VENT			'V'
#define CHAR_TEX_WOOD			'W'
#define CHAR_TEX_GLASS			'Y'
#define CHAR_TEX_WARPSHIELD		'Z' ///< weird-looking jello effect for advisor shield.


// Usefull Math Stuff (It's different then the ones we have in the Utils class)
void SinCos(float a, float* s, float*c)
{
	*s = sin(a);
	*c = cos(a);
}

void AngleVectors(const QAngle &angles, Vector& forward)
{
	float	sp, sy, cp, cy;

	SinCos(DEG2RAD(angles[1]), &sy, &cy);
	SinCos(DEG2RAD(angles[0]), &sp, &cp);

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}

void VectorTransform(const Vector& in1, const matrix3x4_t& in2, Vector& out)
{
	out[0] = in1.Dot(in2[0]) + in2[0][3];
	out[1] = in1.Dot(in2[1]) + in2[1][3];
	out[2] = in1.Dot(in2[2]) + in2[2][3];
}

float RageWall::BestHitPoint(C_BaseEntity *player, int prioritized, float minDmg, mstudiohitboxset_t *hitset, matrix3x4_t matrix[], Vector &vecOut)
{
	mstudiobbox_t *hitbox = hitset->pHitbox(prioritized);
	if (!hitbox)
		return 0.f;

	std::vector<Vector> vecArray;
	float flHigherDamage = 0.f;

	float mod = hitbox->m_flRadius != -1.f ? hitbox->m_flRadius : 0.f;

	Vector max;
	Vector min;

	VectorTransform(hitbox->bbmax + mod, matrix[hitbox->bone], max);
	VectorTransform(hitbox->bbmin - mod, matrix[hitbox->bone], min);

	auto center = (min + max) * 0.5f;

	QAngle curAngles;
	CalcAngle234(center, g::pLocalEntity->GetEyePosition(), curAngles);

	Vector forward;
	AngleVectors234(curAngles, forward);

	Vector right = forward.Cross(Vector(0, 0, 1));
	Vector left = Vector(-right.x, -right.y, right.z);

	Vector top = Vector(0, 0, 1);
	Vector bot = Vector(0, 0, -1);

	const float POINT_SCALE = 0.5f;

	//TODO: Add multipoint feature.
	if (true)
	{
		switch (prioritized)
		{
		case HITBOX_HEAD:
			for (auto i = 0; i < 4; ++i)
			{
				vecArray.emplace_back(center);
			}
			vecArray[1] += top * (hitbox->m_flRadius * POINT_SCALE);
			vecArray[2] += right * (hitbox->m_flRadius * POINT_SCALE);
			vecArray[3] += left * (hitbox->m_flRadius * POINT_SCALE);
			break;

		default:

			for (auto i = 0; i < 2; ++i)
			{
				vecArray.emplace_back(center);
			}
			vecArray[0] += right * (hitbox->m_flRadius * POINT_SCALE);
			vecArray[1] += left * (hitbox->m_flRadius * POINT_SCALE);
			break;
		}
	}
	else
		vecArray.emplace_back(center);

	for (Vector cur : vecArray)
	{
		float flCurDamage = GetDamageVec(cur, player, prioritized);

		if (!flCurDamage)
			continue;

		if ((flCurDamage > flHigherDamage) && (flCurDamage > minDmg))
		{
			flHigherDamage = flCurDamage;
			vecOut = cur;
		}
	}
	return flHigherDamage;
}

float RageWall::GetDamageVec(const Vector &vecPoint, C_BaseEntity *player, int hitbox)
{
	float damage = 0.f;

	Vector rem = vecPoint;

	FireBulletData data;

	data.src = g::pLocalEntity->GetEyePosition();
	data.filter.pSkip = g::pLocalEntity;

	QAngle angle = Utils::CalcAngle(data.src, rem);
	AngleVectors(angle, data.direction);

	data.direction.Normalized();

	auto weap = g::pActiveWeapon;
	if (SimulateFireBullet(weap, data, player, hitbox))
		damage = data.current_damage;

	return damage;
}

void RageWall::traceIt(Vector &vecAbsStart, Vector &vecAbsEnd, unsigned int mask, C_BaseEntity *ign, CGameTrace *tr)
{
	Ray_t ray;

	CTraceFilter filter;
	filter.pSkip = ign;

	ray.Init(vecAbsStart, vecAbsEnd);

	g_pEngineTrace->TraceRay(ray, mask, &filter, tr);
}

bool RageWall::SimulateFireBullet(C_BaseCombatWeapon *weap, FireBulletData &data, C_BaseEntity *player, int hitbox)
{
	if (!weap)
		return false;

	auto GetHitgroup = [](int hitbox) -> int
	{
		switch (hitbox)
		{
		case HITBOX_HEAD:
		case HITBOX_NECK:
			return HITGROUP_HEAD;
		case HITBOX_LOWER_CHEST:
		case HITBOX_CHEST:
		case HITBOX_UPPER_CHEST:
			return HITGROUP_CHEST;
		case HITBOX_STOMACH:
		case HITBOX_PELVIS:
			return HITGROUP_STOMACH;
		case HITBOX_LEFT_HAND:
		case HITBOX_LEFT_UPPER_ARM:
		case HITBOX_LEFT_FOREARM:
			return HITGROUP_LEFTARM;
		case HITBOX_RIGHT_HAND:
		case HITBOX_RIGHT_UPPER_ARM:
		case HITBOX_RIGHT_FOREARM:
			return HITGROUP_RIGHTARM;
		case HITBOX_LEFT_THIGH:
		case HITBOX_LEFT_CALF:
		case HITBOX_LEFT_FOOT:
			return HITGROUP_LEFTLEG;
		case HITBOX_RIGHT_THIGH:
		case HITBOX_RIGHT_CALF:
		case HITBOX_RIGHT_FOOT:
			return HITGROUP_RIGHTLEG;
		default:
			return -1;
		}
	};

	data.penetrate_count = 4;
	data.trace_length = 0.0f;
	WeaponInfo_t *weaponData = g::pActiveWeapon->GetCSWpnData();

	if (weaponData == NULL)
		return false;

	data.current_damage = (float)weaponData->m_iDamage();

	while ((data.penetrate_count > 0) && (data.current_damage >= 1.0f))
	{
		data.trace_length_remaining = weaponData->m_fRange() - data.trace_length;

		Vector end = data.src + data.direction * data.trace_length_remaining;

		traceIt(data.src, end, MASK_SHOT | CONTENTS_GRATE, g::pLocalEntity, &data.enter_trace);
		ClipTraceToPlayers(data.src, end + data.direction * 40.f, MASK_SHOT | CONTENTS_GRATE, &data.filter, &data.enter_trace);

		if (data.enter_trace.fraction == 1.0f)
		{
			if (player && !(player->GetFlags() & FL_ONGROUND))
			{
				data.enter_trace.hitgroup = GetHitgroup(hitbox);
				data.enter_trace.hit_entity = player;
			}
			else
				break;
		}

		surfacedata_t *enter_surface_data = g_pPhysSurface->GetSurfaceData(data.enter_trace.surface.surfaceProps);
		unsigned short enter_material = enter_surface_data->game.material;
		float enter_surf_penetration_mod = enter_surface_data->game.flPenetrationModifier;

		data.trace_length += data.enter_trace.fraction * data.trace_length_remaining;
		data.current_damage *= pow(weaponData->m_fRangeModifier(), data.trace_length * 0.002);

		if (data.trace_length > 3000.f && weaponData->m_fPenetration() > 0.f || enter_surf_penetration_mod < 0.1f)
			break;

		if ((data.enter_trace.hitgroup <= 7) && (data.enter_trace.hitgroup > 0))
		{
			C_BaseEntity *pPlayer = reinterpret_cast<C_BaseEntity*>(data.enter_trace.hit_entity);
			if (pPlayer->IsAlive() && pPlayer->GetTeam() == g::pLocalEntity->GetTeam())
				return false;

			ScaleDamage(data.enter_trace.hitgroup, pPlayer, weaponData->m_fArmorRatio(), data.current_damage);

			return true;
		}

		if (!HandleBulletPenetration(weaponData, data))
			break;
	}

	return false;
}

bool RageWall::HandleBulletPenetration(WeaponInfo_t *wpn_data, FireBulletData &data)
{
	bool bSolidSurf = ((data.enter_trace.contents >> 3) & CONTENTS_SOLID);
	bool bLightSurf = (data.enter_trace.surface.flags >> 7) & SURF_LIGHT;

	surfacedata_t *enter_surface_data = g_pPhysSurface->GetSurfaceData(data.enter_trace.surface.surfaceProps);
	unsigned short enter_material = enter_surface_data->game.material;
	float enter_surf_penetration_mod = enter_surface_data->game.flPenetrationModifier;

	if (!data.penetrate_count && !bLightSurf && !bSolidSurf && enter_material != 89)
	{
		if (enter_material != 71)
			return false;
	}

	if (data.penetrate_count <= 0 || wpn_data->m_fPenetration() <= 0.f)
		return false;

	Vector dummy;
	trace_t trace_exit;

	if (!TraceToExit(dummy, &data.enter_trace, data.enter_trace.endpos, data.direction, &trace_exit))
	{
		if (!(g_pEngineTrace->GetPointContents(dummy, MASK_SHOT_HULL) & MASK_SHOT_HULL))
			return false;
	}

	surfacedata_t *exit_surface_data = g_pPhysSurface->GetSurfaceData(trace_exit.surface.surfaceProps);
	unsigned short exit_material = exit_surface_data->game.material;

	float exit_surf_penetration_mod = exit_surface_data->game.flPenetrationModifier;
	float exit_surf_damage_mod = exit_surface_data->game.flDamageModifier;

	float final_damage_modifier = 0.16f;
	float combined_penetration_modifier = 0.0f;

	if (enter_material == 89 || enter_material == 71)
	{
		combined_penetration_modifier = 3.0f;
		final_damage_modifier = 0.05f;
	}
	else if (bSolidSurf || bLightSurf)
	{
		combined_penetration_modifier = 1.0f;
		final_damage_modifier = 0.16f;
	}
	else
	{
		combined_penetration_modifier = (enter_surf_penetration_mod + exit_surf_penetration_mod) * 0.5f;
	}

	if (enter_material == exit_material)
	{
		if (exit_material == 87 || exit_material == 85)
			combined_penetration_modifier = 3.0f;
		else if (exit_material == 76)
			combined_penetration_modifier = 2.0f;
	}

	float modifier = fmaxf(0.0f, 1.0f / combined_penetration_modifier);
	float thickness = (trace_exit.endpos - data.enter_trace.endpos).LengthSqr();
	float taken_damage = ((modifier * 3.0f) * fmaxf(0.0f, (3.0f / wpn_data->m_fPenetration()) * 1.25f) + (data.current_damage * final_damage_modifier)) + ((thickness * modifier) / 24.0f);

	float lost_damage = fmaxf(0.0f, taken_damage);

	if (lost_damage > data.current_damage)
		return false;

	if (lost_damage > 0.0f)
		data.current_damage -= lost_damage;

	if (data.current_damage < 1.0f)
		return false;

	data.src = trace_exit.endpos;
	data.penetrate_count--;

	return true;
}

bool RageWall::TraceToExit(Vector &end, CGameTrace *enter_trace, Vector start, Vector dir, CGameTrace *exit_trace)
{
	auto distance = 0.0f;
	int first_contents = 0;

	while (distance < 90.0f)
	{
		distance += 4.0f;
		end = start + (dir * distance);

		if (!first_contents)
			first_contents = g_pEngineTrace->GetPointContents(end, MASK_SHOT_HULL | CONTENTS_HITBOX);

		auto point_contents = g_pEngineTrace->GetPointContents(end, MASK_SHOT_HULL | CONTENTS_HITBOX);

		if (point_contents & MASK_SHOT_HULL && (!(point_contents & CONTENTS_HITBOX) || first_contents == point_contents))
			continue;

		auto new_end = end - (dir * 4.0f);

		traceIt(end, new_end, MASK_SHOT | CONTENTS_GRATE, nullptr, exit_trace);

		if (exit_trace->startsolid && (exit_trace->surface.flags & SURF_HITBOX) < 0)
		{
			traceIt(end, start, MASK_SHOT_HULL, reinterpret_cast<C_BaseEntity*>(exit_trace->hit_entity), exit_trace);

			if (exit_trace->DidHit() && !exit_trace->startsolid)
			{
				end = exit_trace->endpos;
				return true;
			}
			continue;
		}

		if (!exit_trace->DidHit() || exit_trace->startsolid)
		{
			if (enter_trace->hit_entity)
			{
				if (enter_trace->DidHitNonWorldEntity() && IsBreakableEntity(reinterpret_cast<C_BaseEntity*>(enter_trace->hit_entity)))
				{
					*exit_trace = *enter_trace;
					exit_trace->endpos = start + dir;
					return true;
				}
			}
			continue;
		}

		if ((exit_trace->surface.flags >> 7) & SURF_LIGHT)
		{
			if (IsBreakableEntity(reinterpret_cast<C_BaseEntity*>(exit_trace->hit_entity)) && IsBreakableEntity(reinterpret_cast<C_BaseEntity*>(enter_trace->hit_entity)))
			{
				end = exit_trace->endpos;
				return true;
			}

			if (!((enter_trace->surface.flags >> 7) & SURF_LIGHT))
				continue;
		}

		if (exit_trace->plane.normal.Dot(dir) <= 1.0f)
		{
			float fraction = exit_trace->fraction * 4.0f;
			end = end - (dir * fraction);

			return true;
		}
	}
	return false;
}

bool RageWall::IsBreakableEntity(C_BaseEntity *ent)
{
	typedef bool(__thiscall *isBreakbaleEntityFn)(C_BaseEntity*);
	static isBreakbaleEntityFn IsBreakableEntityFn = (isBreakbaleEntityFn)Utils::FindSignature("client_panorama.dll", "55 8B EC 51 56 8B F1 85 F6 74 68");

	if (IsBreakableEntityFn)
	{
		// 0x280 = m_takedamage

		auto backupval = *reinterpret_cast<int*>((uint32_t)ent + 0x280);
		auto className = ent->GetClientClass()->pNetworkName;

		if (ent != g_pEntityList->GetClientEntity(0))
		{
			// CFuncBrush:
			// (className[1] != 'F' || className[4] != 'c' || className[5] != 'B' || className[9] != 'h')
			if ((className[1] == 'B' && className[9] == 'e' && className[10] == 'S' && className[16] == 'e') // CBreakableSurface
				|| (className[1] != 'B' || className[5] != 'D')) // CBaseDoor because fuck doors
			{
				*reinterpret_cast<int*>((uint32_t)ent + 0x280) = 2;
			}
		}

		bool retn = IsBreakableEntityFn(ent);

		*reinterpret_cast<int*>((uint32_t)ent + 0x280) = backupval;

		return retn;
	}
	else
		return false;
}

void RageWall::ScaleDamage(int hitgroup, C_BaseEntity *player, float weapon_armor_ratio, float &current_damage)
{
	bool heavArmor = player->hasHeavyArmor();
	int armor = player->GetArmor();

	switch (hitgroup)
	{
	case HITGROUP_HEAD:

		if (heavArmor)
			current_damage *= (current_damage * 4.f) * 0.5f;
		else
			current_damage *= 4.f;

		break;

	case HITGROUP_CHEST:
	case HITGROUP_LEFTARM:
	case HITGROUP_RIGHTARM:

		current_damage *= 1.f;
		break;

	case HITGROUP_STOMACH:

		current_damage *= 1.25f;
		break;

	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:

		current_damage *= 0.75f;
		break;
	}

	if (IsArmored(player, armor, hitgroup))
	{
		float v47 = 1.f, armor_bonus_ratio = 0.5f, armor_ratio = weapon_armor_ratio * 0.5f;

		if (heavArmor)
		{
			armor_bonus_ratio = 0.33f;
			armor_ratio = (weapon_armor_ratio * 0.5f) * 0.5f;
			v47 = 0.33f;
		}

		float new_damage = current_damage * armor_ratio;

		if (heavArmor)
			new_damage *= 0.85f;

		if (((current_damage - (current_damage * armor_ratio)) * (v47 * armor_bonus_ratio)) > armor)
			new_damage = current_damage - (armor / armor_bonus_ratio);

		current_damage = new_damage;
	}
}

bool RageWall::IsArmored(C_BaseEntity *player, int armorVal, int hitgroup)
{
	bool res = false;

	if (armorVal > 0)
	{
		switch (hitgroup)
		{
		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:

			res = true;
			break;

		case HITGROUP_HEAD:

			res = player->hasHelmet();
			break;

		}
	}

	return res;
}

void RageWall::ClipTraceToPlayers(const Vector &vecAbsStart, const Vector &vecAbsEnd, unsigned int mask, ITraceFilter *filter, CGameTrace *tr)
{
	trace_t playerTrace;
	Ray_t ray;
	float smallestFraction = tr->fraction;
	const float maxRange = 60.0f;

	ray.Init(vecAbsStart, vecAbsEnd);

	for (int i = 1; i <= g_pEngine->GetMaxClients(); i++)
	{
		C_BaseEntity *player = g_pEntityList->GetClientEntity(i);

		if (!player || !player->IsAlive() || player->IsDormant())
			continue;

		if (filter && filter->ShouldHitEntity(player, mask) == false)
			continue;

		float range = Utils::DistanceToRay(player->WorldSpaceCenter(), vecAbsStart, vecAbsEnd);
		if (range < 0.0f || range > maxRange)
			continue;

		g_pEngineTrace->ClipRayToEntity(ray, mask | CONTENTS_HITBOX, player, &playerTrace);
		if (playerTrace.fraction < smallestFraction)
		{
			*tr = playerTrace;
			smallestFraction = playerTrace.fraction;
		}
	}
}

void RageWall::TargetEntities(CUserCmd* pCmd)
{
	auto weap = g::pActiveWeapon;
	static C_BaseCombatWeapon *oldWeapon; // what is this for?
	if (weap != oldWeapon)
	{
		oldWeapon = weap;
		pCmd->buttons &= ~IN_ATTACK;
		return;
	}

	if (weap->isPistol() && pCmd->tick_count % 2)
	{
		static int lastshot;
		if (pCmd->buttons & IN_ATTACK)
			lastshot++;

		if (!pCmd->buttons & IN_ATTACK || lastshot > 1)
		{
			pCmd->buttons &= ~IN_ATTACK;
			lastshot = 0;
		}
		return;
	}

	if (prev_aimtarget && CheckTarget(prev_aimtarget))
	{
		if (TargetSpecificEnt(C_BaseEntity::GetEntityByIndex(prev_aimtarget), pCmd))
			return;
	}

	for (int i = 1; i < g_pEngine->GetMaxClients(); i++)
	{

		if (!CheckTarget(i))
			continue;

		C_BaseEntity *player = g_pEntityList->GetClientEntity(i);

		if (TargetSpecificEnt(player, pCmd))
			return;
	}
}

int realHitboxSpot2[] = { 0, 1, 2, 3 };
bool RageWall::TargetSpecificEnt(C_BaseEntity* pEnt, CUserCmd* pCmd)
{
	int i = pEnt->EntIndex();
	//auto firedShots = g::pLocalEntity->f;

	int iHitbox = 0;

	Vector vecTarget;

	C_BaseCombatWeapon* pWeapon = g::pActiveWeapon;

	if (!pWeapon)
		return false;

	float flServerTime = g::pLocalEntity->GetTickBase() * g_pGlobalVars->intervalPerTick;
	bool canShoot = !(pWeapon->GetNextPrimaryAttack() > flServerTime) && !(pCmd->buttons & IN_RELOAD);

	// Disgusting ass codes, can't think of a cleaner way now though. FIX ME.
	bool LagComp_Hitchanced = false;
	if (!g_Settings.bRagebotAutowall)
	{
		vecTarget = pEnt->GetBonePos(8);
	}
	else
	{
		matrix3x4_t matrix[128];
		if (!pEnt->SetupBones2(matrix, 128, 256, pEnt->GetSimulationTime()))
			return false;

		vecTarget = CalculateBestPoint(pEnt, iHitbox, g_Settings.bRagebotMinDamage, false, matrix);
	}

	// Invalid target/no hitable points at all.
	if (!vecTarget.IsValid())
		return false;

	if (vecTarget.x != vecTarget.x)
		vecTarget.x = 0.f;

	if (vecTarget.y != vecTarget.y)
		vecTarget.y = 0.f;
		
	if (vecTarget.z != vecTarget.z)
		vecTarget.z = 0.f;

	if ((vecTarget.x >= -0.0001f && vecTarget.x <= 0.0001f) || (vecTarget.y >= -0.0001f && vecTarget.y <= 0.0001f))
		return false;

	QAngle new_aim_angles = Utils::CalcAngle(g::pLocalEntity->GetEyePosition(), vecTarget) - (g::pLocalEntity->GetPunchAngles() * 2.f);
	Utils::ClampViewAngles(new_aim_angles);

	pCmd->viewangles = new_aim_angles;
	if (!g_Settings.bRagebotSilent)
		g_pEngine->SetViewAngles(new_aim_angles);

	if (canShoot)
	{
		prev_aimtarget = pEnt->EntIndex();

		if (g_Settings.bRagebotAutoFire)
		{
			if (g_Settings.bRagebotHitchance)
			{
				if (HitChance(new_aim_angles, pEnt, g_Settings.bRagebotHitchanceA)) {
					pCmd->buttons |= IN_ATTACK;
					switchTick = 0;
				}
				else
					AutoStop();
			}
			else
			{
				AutoStop();
				pCmd->buttons |= IN_ATTACK;
			}
		}
	}

	pCmd->tick_count = FixTickcount(pEnt);

	return true;
}

int RageWall::FixTickcount(C_BaseEntity * player)
{
	int idx = player->EntIndex();

	auto cl_interp_ratio = g_pCVar->FindVar("cl_interp_ratio");
	auto cl_updaterate = g_pCVar->FindVar("cl_updaterate");
	int lerpTicks = TIME_TO_TICKS(cl_interp_ratio->GetFloat() / cl_updaterate->GetFloat());

	return TIME_TO_TICKS(player->GetSimulationTime()) + lerpTicks;
}

float RandomFloat(float min, float max)
{
	static auto ranFloat = reinterpret_cast<float(*)(float, float)>(GetProcAddress(GetModuleHandleA("vstdlib.dll"), "RandomFloat"));
	if (ranFloat)
		return ranFloat(min, max);
	else
		return 0.f;
}

bool RageWall::HitChance(QAngle angles, C_BaseEntity *ent, float chance)
{
	auto weapon = g::pActiveWeapon;

	if (!weapon)
		return false;

	Vector forward, right, up;
	Vector src = g::pLocalEntity->GetEyePosition();
	AngleVectors(angles, forward, right, up);

	int cHits = 0;
	int cNeededHits = static_cast<int>(150.f * (chance / 100.f));

	weapon->UpdateAccuracyPenalty();
	float weap_spread = weapon->GetSpread();
	float weap_inaccuracy = weapon->GetInaccuracy();

	for (int i = 0; i < 150; i++)
	{
		float a = RandomFloat(0.f, 1.f);
		float b = RandomFloat(0.f, 2.f * PI);
		float c = RandomFloat(0.f, 1.f);
		float d = RandomFloat(0.f, 2.f * PI);

		float inaccuracy = a * weap_inaccuracy;
		float spread = c * weap_spread;

		if (weapon->GetCSWpnData()->weapon_type() == 64)
		{
			a = 1.f - a * a;
			a = 1.f - c * c;
		}

		Vector spreadView((cos(b) * inaccuracy) + (cos(d) * spread), (sin(b) * inaccuracy) + (sin(d) * spread), 0), direction;

		direction.x = forward.x + (spreadView.x * right.x) + (spreadView.y * up.x);
		direction.y = forward.y + (spreadView.x * right.y) + (spreadView.y * up.y);
		direction.z = forward.z + (spreadView.x * right.z) + (spreadView.y * up.z);
		direction.Normalized();

		QAngle viewAnglesSpread;
		Utils::VectorAngles(direction, up, viewAnglesSpread);
		viewAnglesSpread.Normalize();

		Vector viewForward;
		AngleVectors(viewAnglesSpread, viewForward);
		viewForward.NormalizeInPlace();

		viewForward = src + (viewForward * weapon->GetCSWpnData()->m_fRange());

		trace_t tr;
		Ray_t ray;

		ray.Init(src, viewForward);
		g_pEngineTrace->ClipRayToEntity(ray, MASK_SHOT | CONTENTS_GRATE, ent, &tr);

		if (tr.hit_entity == ent)
			++cHits;

		if (static_cast<int>((static_cast<float>(cHits) / 150.f) * 100.f) >= chance)
			return true;

		if ((150 - i + cHits) < cNeededHits)
			return false;
	}
	return false;
}

bool RageWall::CheckTarget(int i)
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

	if (!g::pLocalEntity->CanSeePlayer(player, player->GetBonePos(8)) && !g_Settings.bRagebotAutowall)
		return false;

	return true;
}

Vector RageWall::CalculateBestPoint(C_BaseEntity *player, int prioritized, float minDmg, bool onlyPrioritized, matrix3x4_t matrix[])
{
	studiohdr_t *studioHdr = g_pMdlInfo->GetStudiomodel2(player->GetModel());
	mstudiohitboxset_t *set = studioHdr->GetHitboxSet(0);
	Vector vecOutput = Vector(0,0,0);

	if (BestHitPoint(player, prioritized, minDmg, set, matrix, vecOutput) > minDmg && onlyPrioritized)
	{
		return vecOutput;
	}
	else
	{
		float flHigherDamage = 0.f;

		Vector vecCurVec;

		// why not use all the hitboxes then
		//static Hitboxes hitboxesLoop;
		static int hitboxesLoop[] =
		{
			HITBOX_HEAD,
			HITBOX_PELVIS,
			HITBOX_UPPER_CHEST,
			HITBOX_CHEST,
			HITBOX_NECK,
			HITBOX_LEFT_FOREARM,
			HITBOX_RIGHT_FOREARM,
			HITBOX_RIGHT_HAND,
			HITBOX_LEFT_THIGH,
			HITBOX_RIGHT_THIGH,
			HITBOX_LEFT_CALF,
			HITBOX_RIGHT_CALF,
			HITBOX_LEFT_FOOT,
			HITBOX_RIGHT_FOOT
		};

		int loopSize = ARRAYSIZE(hitboxesLoop);

		for (int i = 0; i < loopSize; ++i)
		{
			float flCurDamage = BestHitPoint(player, hitboxesLoop[i], minDmg, set, matrix, vecCurVec);

			if (!flCurDamage)
				continue;

			if (flCurDamage > flHigherDamage)
			{
				flHigherDamage = flCurDamage;
				vecOutput = vecCurVec;
				if (static_cast<int32_t>(flHigherDamage) >= player->GetHealth())
					break;
			}
		}
		return vecOutput;
	}
}

void RageWall::AutoStop()
{
	if (!g_Settings.bRagebotAutostop)
		return;

	if (g::pLocalEntity->GetVelocity().Length() > (g::pActiveWeapon->GetCSWpnData()->flMaxPlayerSpeed / 3) && switchTick < 5)
	{
		g::pCmd->buttons |= IN_WALK;
		g::pCmd->forwardmove = -g::pCmd->forwardmove;
		g::pCmd->sidemove = -g::pCmd->sidemove;
		g::pCmd->upmove = 0;

		switchTick++;
	}
	else if (switchTick >= 5 && switchTick < 15 || g::pLocalEntity->GetVelocity().Length() < (g::pActiveWeapon->GetCSWpnData()->flMaxPlayerSpeed / 3))
	{
		g::pCmd->forwardmove = 0;
		g::pCmd->sidemove = 0;

		switchTick++;
	}
	else
	{
		switchTick = 0;
	}

}