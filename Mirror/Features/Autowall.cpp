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



void AngleVectors234(const QAngle &angles, Vector *forward)
{
	Assert(s_bMathlibInitialized);
	Assert(forward);

	float	sp, sy, cp, cy;

	sy = sin(DEG2RAD(angles[1]));
	cy = cos(DEG2RAD(angles[1]));

	sp = sin(DEG2RAD(angles[0]));
	cp = cos(DEG2RAD(angles[0]));

	forward->x = cp * cy;
	forward->y = cp * sy;
	forward->z = -sp;
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

autowall_2 new_autowall;

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

void autowall_2::TraceLine(Vector& absStart, Vector& absEnd, unsigned int mask, IClientEntity* ignore, CGameTrace* ptr)
{
	Ray_t ray;
	ray.Init(absStart, absEnd);
	CTraceFilter filter;
	filter.pSkip = ignore;

	g_pEngineTrace->TraceRay(ray, mask, &filter, ptr);
}

void autowall_2::ClipTraceToPlayers(const Vector& absStart, const Vector absEnd, unsigned int mask, ITraceFilter* filter, CGameTrace* tr)
{

	C_BaseEntity *pLocal = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());
	C_BaseCombatWeapon* weapon = (C_BaseCombatWeapon*)pLocal->GetActiveWeapon();

	static DWORD dwAddress = Utils::FindSignature("client_panorama.dll", "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? 8B 43 10");

	if (!dwAddress)
		return;

	if (!weapon->isPistol() && !weapon->isRifle() && !weapon->isSniper())
		return;

	_asm
	{
		MOV		EAX, filter
		LEA		ECX, tr
		PUSH	ECX
		PUSH	EAX
		PUSH	mask
		LEA		EDX, absEnd
		LEA		ECX, absStart
		CALL	dwAddress
		ADD		ESP, 0xC
	}
}

////////////////////////////////////// Legacy Functions //////////////////////////////////////
void autowall_2::GetBulletTypeParameters(float& maxRange, float& maxDistance, char* bulletType, bool sv_penetration_type)
{
	if (sv_penetration_type)
	{
		maxRange = 35.0;
		maxDistance = 3000.0;
	}
	else
	{
		//Play tribune to framerate. Thanks, stringcompare
		//Regardless I doubt anyone will use the old penetration system anyway; so it won't matter much.
		if (!strcmp(bulletType, ("BULLET_PLAYER_338MAG")))
		{
			maxRange = 45.0;
			maxDistance = 8000.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_762MM")))
		{
			maxRange = 39.0;
			maxDistance = 5000.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_556MM")) || !strcmp(bulletType, ("BULLET_PLAYER_556MM_SMALL")) || !strcmp(bulletType, ("BULLET_PLAYER_556MM_BOX")))
		{
			maxRange = 35.0;
			maxDistance = 4000.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_57MM")))
		{
			maxRange = 30.0;
			maxDistance = 2000.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_50AE")))
		{
			maxRange = 30.0;
			maxDistance = 1000.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_357SIG")) || !strcmp(bulletType, ("BULLET_PLAYER_357SIG_SMALL")) || !strcmp(bulletType, ("BULLET_PLAYER_357SIG_P250")) || !strcmp(bulletType, ("BULLET_PLAYER_357SIG_MIN")))
		{
			maxRange = 25.0;
			maxDistance = 800.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_9MM")))
		{
			maxRange = 21.0;
			maxDistance = 800.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_45ACP")))
		{
			maxRange = 15.0;
			maxDistance = 500.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_BUCKSHOT")))
		{
			maxRange = 0.0;
			maxDistance = 0.0;
		}
	}
}

////////////////////////////////////// Misc Functions //////////////////////////////////////
bool autowall_2::BreakableEntity(IClientEntity* entity)
{

	ClientClass* pClass = (ClientClass*)entity->GetClientClass();

	if (!pClass)
		return false;

	if (pClass == nullptr)
		return false;

	return pClass->ClassID == EClassIds::CBreakableProp || pClass->ClassID == EClassIds::CBreakableSurface;

}

void autowall_2::ScaleDamage(CGameTrace &enterTrace, WeaponInfo_t *weaponData, float& currentDamage)
{

	C_BaseEntity *pLocal = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());
	C_BaseCombatWeapon* weapon = (C_BaseCombatWeapon*)pLocal->GetActiveWeapon();
	bool hasHeavyArmor = false;
	int armorValue = ((C_BaseEntity*)enterTrace.hit_entity)->GetArmor();
	int hitGroup = enterTrace.hitgroup;

	if (!pLocal)
		return;

	if (!weapon->isPistol() && !weapon->isRifle() && !weapon->isSniper())
		return;


	auto IsArmored = [&enterTrace]()->bool
	{
		C_BaseEntity* targetEntity = (C_BaseEntity*)enterTrace.hit_entity;
		switch (enterTrace.hitgroup)
		{
		case HITGROUP_HEAD:
			return !!(C_BaseEntity*)targetEntity->hasHelmet();
		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			return true;
		default:
			return false;
		}
	};

	switch (hitGroup)
	{
	case HITGROUP_HEAD:
		currentDamage *= 2.f;
		break;
	case HITGROUP_STOMACH:
		currentDamage *= 1.25f;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		currentDamage *= 0.75f;
		break;
	default:
		break;
	}

	if (armorValue > 0 && IsArmored())
	{
		float bonusValue = 1.f, armorBonusRatio = 0.5f, armorRatio = weaponData->flArmorRatio / 2.f;

		if (hasHeavyArmor)
		{
			armorBonusRatio = 0.33f;
			armorRatio *= 0.5f;
			bonusValue = 0.33f;
		}

		auto NewDamage = currentDamage * armorRatio;

		if (((currentDamage - (currentDamage * armorRatio)) * (bonusValue * armorBonusRatio)) > armorValue)
			NewDamage = currentDamage - (armorValue / armorBonusRatio);

		currentDamage = NewDamage;
	}
}

////////////////////////////////////// Main Autowall Functions //////////////////////////////////////
bool autowall_2::trace_to_exit(CGameTrace& enterTrace, CGameTrace& exitTrace, Vector startPosition, Vector direction)
{
	Vector start, end;
	float maxDistance = 90.f, rayExtension = 4.f, currentDistance = 0;
	int firstContents = 0;

	while (currentDistance <= maxDistance)
	{
		currentDistance += rayExtension;

		start = startPosition + direction * currentDistance;

		if (!firstContents)
			firstContents = g_pEngineTrace->GetPointContents(start, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr);

		int pointContents = g_pEngineTrace->GetPointContents(start, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr);

		if (!(pointContents & MASK_SHOT_HULL) || pointContents & CONTENTS_HITBOX && pointContents != firstContents)
		{
			end = start - (direction * rayExtension);

			TraceLine(start, end, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr, &exitTrace);

			if (exitTrace.startsolid && exitTrace.surface.flags & SURF_HITBOX)
			{
				TraceLine(start, startPosition, MASK_SHOT_HULL, (IClientEntity*)exitTrace.hit_entity, &exitTrace);

				if (exitTrace.DidHit() && !exitTrace.startsolid)
				{
					start = exitTrace.endpos;
					return true;
				}
				continue;
			}

			if (exitTrace.DidHit() && !exitTrace.startsolid)
			{

				if (BreakableEntity((IClientEntity*)enterTrace.hit_entity) && BreakableEntity((IClientEntity*)exitTrace.hit_entity))
					return true;

				if (enterTrace.surface.flags & SURF_NODRAW || !(exitTrace.surface.flags & SURF_NODRAW) && (exitTrace.plane.normal.Dot(direction) <= 1.f))
				{
					float multAmount = exitTrace.fraction * 4.f;
					start -= direction * multAmount;
					return true;
				}

				continue;
			}

			if (!exitTrace.DidHit() || exitTrace.startsolid)
			{
				if (enterTrace.DidHitNonWorldEntity() && BreakableEntity((IClientEntity*)enterTrace.hit_entity))
				{
					exitTrace = enterTrace;
					exitTrace.endpos = start + direction;
					return true;
				}

				continue;
			}
		}
	}
	return false;
}

bool autowall_2::HandleBulletPenetration(WeaponInfo_t* weaponData, CGameTrace& enterTrace, Vector& eyePosition, Vector direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, bool sv_penetration_type, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration)
{
	//Because there's been issues regarding this- putting this here.
	if (&currentDamage == nullptr)
	{
		handle_penetration = false;
		return false;
	}

	C_BaseEntity* local = (C_BaseEntity*)g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());

	auto data = FireBulletData(local->GetEyePosition());
	data.filter = CTraceFilter();
	data.filter.pSkip = local;
	CGameTrace exitTrace;
	C_BaseEntity* pEnemy = (C_BaseEntity*)enterTrace.hit_entity;

	surfacedata_t *enterSurfaceData = g_pPhysSurface->GetSurfaceData(enterTrace.surface.surfaceProps);
	int enterMaterial = enterSurfaceData->game.material;

	float enterSurfPenetrationModifier = enterSurfaceData->game.flPenetrationModifier;
	float enterDamageModifier = enterSurfaceData->game.flDamageModifier;
	float thickness, modifier, lostDamage, finalDamageModifier, combinedPenetrationModifier;
	bool isSolidSurf = ((enterTrace.contents >> 3) & CONTENTS_SOLID);
	bool isLightSurf = ((enterTrace.surface.flags >> 7) & SURF_LIGHT);

	if (possibleHitsRemaining <= 0
		|| (enterTrace.surface.name == (const char*)0x2227c261 && exitTrace.surface.name == (const char*)0x2227c868)
		|| (!possibleHitsRemaining && !isLightSurf && !isSolidSurf && enterMaterial != CHAR_TEX_GRATE && enterMaterial != CHAR_TEX_GLASS)
		|| weaponData->flPenetration <= 0.f
		|| !trace_to_exit(enterTrace, exitTrace, enterTrace.endpos, direction)
		&& !(g_pEngineTrace->GetPointContents(enterTrace.endpos, MASK_SHOT_HULL, nullptr) & MASK_SHOT_HULL))
	{
		handle_penetration = false;
		return false;
	}

	surfacedata_t *exitSurfaceData = g_pPhysSurface->GetSurfaceData(exitTrace.surface.surfaceProps);
	int exitMaterial = exitSurfaceData->game.material;
	float exitSurfPenetrationModifier = exitSurfaceData->game.flPenetrationModifier;
	float exitDamageModifier = exitSurfaceData->game.flDamageModifier;

	if (sv_penetration_type)
	{
		if (enterMaterial == CHAR_TEX_GRATE || enterMaterial == CHAR_TEX_GLASS)
		{
			combinedPenetrationModifier = 3.f;
			finalDamageModifier = 0.05f;
		}
		else if (isSolidSurf || isLightSurf)
		{
			combinedPenetrationModifier = 1.f;
			finalDamageModifier = 0.16f;
		}
		else if (enterMaterial == CHAR_TEX_FLESH && (local->GetTeam() == pEnemy->GetTeam() && ff_damage_reduction_bullets == 0.f))
		{
			if (ff_damage_bullet_penetration == 0.f)
			{
				handle_penetration = false;
				return false;
			}
			combinedPenetrationModifier = ff_damage_bullet_penetration;
			finalDamageModifier = 0.16f;
		}
		else
		{
			combinedPenetrationModifier = (enterSurfPenetrationModifier + exitSurfPenetrationModifier) / 2.f;
			finalDamageModifier = 0.16f;
		}

		if (enterMaterial == exitMaterial)
		{
			if (exitMaterial == CHAR_TEX_CARDBOARD || exitMaterial == CHAR_TEX_WOOD)
				combinedPenetrationModifier = 3.f;
			else if (exitMaterial == CHAR_TEX_PLASTIC)
				combinedPenetrationModifier = 2.f;
		}

		thickness = (exitTrace.endpos - enterTrace.endpos).LengthSqr();
		modifier = fmaxf(1.f / combinedPenetrationModifier, 0.f);

		lostDamage = fmaxf(
			((modifier * thickness) / 24.f)
			+ ((currentDamage * finalDamageModifier)
				+ (fmaxf(3.75f / penetrationPower, 0.f) * 3.f * modifier)), 0.f);

		if (lostDamage > currentDamage)
		{
			handle_penetration = false;
			return false;
		}

		if (lostDamage > 0.f)
			currentDamage -= lostDamage;

		if (currentDamage < 1.f)
		{
			handle_penetration = false;
			return false;
		}

		eyePosition = exitTrace.endpos;
		--possibleHitsRemaining;

		handle_penetration = true;
		return true;
	}
	else
	{
		combinedPenetrationModifier = 1.f;

		if (isSolidSurf || isLightSurf)
			finalDamageModifier = 0.99f;
		else
		{
			finalDamageModifier = fminf(enterDamageModifier, exitDamageModifier);
			combinedPenetrationModifier = fminf(enterSurfPenetrationModifier, exitSurfPenetrationModifier);
		}

		if (enterMaterial == exitMaterial && (exitMaterial == CHAR_TEX_METAL || exitMaterial == CHAR_TEX_WOOD))
			combinedPenetrationModifier += combinedPenetrationModifier;

		thickness = (exitTrace.endpos - enterTrace.endpos).LengthSqr();

		if (sqrt(thickness) <= combinedPenetrationModifier * penetrationPower)
		{
			currentDamage *= finalDamageModifier;
			eyePosition = exitTrace.endpos;
			--possibleHitsRemaining;
			handle_penetration = true;
			return true;
		}
		handle_penetration = false;
		return false;
	}
}

bool autowall_2::FireBullet(C_BaseCombatWeapon* pWeapon, Vector& direction, float& currentDamage)
{
	if (!pWeapon)
		return false;

	C_BaseEntity* local = (C_BaseEntity*)g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());

	auto data = FireBulletData(local->GetEyePosition());

	data.filter = CTraceFilter();
	data.filter.pSkip = local;

	bool sv_penetration_type;

	float currentDistance = 0.f, penetrationPower, penetrationDistance, maxRange, ff_damage_reduction_bullets, ff_damage_bullet_penetration, rayExtension = 40.f;
	Vector eyePosition = local->GetEyePosition();

	static ConVar* penetrationSystem = g_pCVar->FindVar(("sv_penetration_type"));
	static ConVar* damageReductionBullets = g_pCVar->FindVar(("ff_damage_reduction_bullets"));
	static ConVar* damageBulletPenetration = g_pCVar->FindVar(("ff_damage_bullet_penetration"));

	sv_penetration_type = penetrationSystem->GetBool();
	ff_damage_reduction_bullets = damageReductionBullets->GetFloat();
	ff_damage_bullet_penetration = damageBulletPenetration->GetFloat();

	WeaponInfo_t* weaponData = pWeapon->GetCSWpnData();
	CGameTrace enterTrace;
	CTraceFilter filter;

	filter.pSkip = local;

	if (!weaponData)
		return false;

	maxRange = weaponData->flRange;

	GetBulletTypeParameters(penetrationPower, penetrationDistance, weaponData->szBulletType, sv_penetration_type);

	if (sv_penetration_type)
		penetrationPower = weaponData->flPenetration;

	int possibleHitsRemaining = 4;

	currentDamage = weaponData->iDamage;

	while (possibleHitsRemaining > 0 && currentDamage >= 1.f)
	{
		maxRange -= currentDistance;

		Vector end = eyePosition + direction * maxRange;

		TraceLine(eyePosition, end, MASK_SHOT_HULL | CONTENTS_HITBOX, (IClientEntity*)local, &enterTrace);
		ClipTraceToPlayers(eyePosition, end + direction * rayExtension, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &enterTrace); //  | CONTENTS_HITBOX

		surfacedata_t *enterSurfaceData = g_pPhysSurface->GetSurfaceData(enterTrace.surface.surfaceProps);

		float enterSurfPenetrationModifier = enterSurfaceData->game.flPenetrationModifier;

		int enterMaterial = enterSurfaceData->game.material;

		if (enterTrace.fraction == 1.f)
			break;

		currentDistance += enterTrace.fraction * maxRange;

		currentDamage *= pow(weaponData->flRangeModifier, (currentDistance / 500.f));

		if (currentDistance > penetrationDistance && weaponData->flPenetration > 0.f || enterSurfPenetrationModifier < 0.1f)
			break;

		bool canDoDamage = (enterTrace.hitgroup != HITGROUP_GEAR && enterTrace.hitgroup != HITGROUP_GENERIC);
		bool isEnemy = (((C_BaseEntity*)local)->GetTeam() != ((C_BaseEntity*)enterTrace.hit_entity)->GetTeam());

		if ((canDoDamage && isEnemy))
		{
			ScaleDamage(enterTrace, weaponData, currentDamage);
			return true;
		}

		if (!HandleBulletPenetration(weaponData, enterTrace, eyePosition, direction, possibleHitsRemaining, currentDamage, penetrationPower, sv_penetration_type, ff_damage_reduction_bullets, ff_damage_bullet_penetration))
			break;
	}
	return false;
}

////////////////////////////////////// Usage Calls //////////////////////////////////////
float autowall_2::CanHit(Vector &point)
{
	C_BaseEntity* local = (C_BaseEntity*)g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());

	if (!local || !local->IsAlive())
		return -1;

	auto data = FireBulletData(local->GetEyePosition());
	data.filter = CTraceFilter();
	data.filter.pSkip = local;
	QAngle angles;
	Vector direction;
	Vector tmp = point - local->GetEyePosition();
	float currentDamage = 0;

	VectorAngles234(tmp, angles);
	AngleVectors234(angles, &direction);
	direction.NormalizeInPlace();

	if (FireBullet(local->GetActiveWeapon(), direction, currentDamage))
		return currentDamage;
	return -1;
}

bool autowall_2::PenetrateWall(C_BaseEntity* pBaseEntity, Vector& vecPoint)
{
	float min_damage = g_Settings.bRagebotMinDamage;

	if (pBaseEntity->GetHealth() < min_damage)
		min_damage = pBaseEntity->GetHealth();

	if (CanHit(vecPoint) >= min_damage)
		return true;

	return false;
}

bool autowall_2::PenetrateWall(C_BaseEntity* pBaseEntity, Vector& vecPoint , int& iDamage)
{
	float min_damage = g_Settings.bRagebotMinDamage;

	if (pBaseEntity->GetHealth() < min_damage)
		min_damage = pBaseEntity->GetHealth();

	if (CanHit(vecPoint) >= min_damage)
	{
		return true;

	}
	iDamage = CanHit(vecPoint);
	return false;
}
void trace_line(const Vector& vecAbsStart, const Vector& vecAbsEnd, unsigned int mask, const IClientEntity *ignore, int collisionGroup, trace_t *ptr)
{
	typedef int(__fastcall* UTIL_TraceLine_t)(const Vector&, const Vector&, unsigned int, const IClientEntity*, int, trace_t*);
	static UTIL_TraceLine_t TraceLine = (UTIL_TraceLine_t)Utils::FindSignature("client_panorama.dll", "55 8B EC 83 E4 F0 83 EC 7C 56 52");
	TraceLine(vecAbsStart, vecAbsEnd, mask, ignore, collisionGroup, ptr);
}
float autowall_2::trace_awall(float damage)
{
	auto m_local = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());
	if (m_local && m_local->IsAlive()) {
		FireBulletData data = FireBulletData(m_local->GetEyePosition());
		data.filter = CTraceFilter();
		data.filter.pSkip = m_local;
		QAngle eye_angle;  g_pEngine->GetViewAngles(eye_angle);
		Vector dst, forward;
		AngleVectors234(eye_angle, &forward);
		dst = data.src + (forward * 8196.f);
		QAngle angles;
		CalcAngle234(data.src, dst, angles);
		AngleVectors234(angles, &data.direction);
		VectorNormalize(data.direction);
		auto m_weapon = m_local->GetActiveWeapon();
		if (m_weapon) {
			data.penetrate_count = 1;
			data.trace_length = 0.0f;
			WeaponInfo_t *weapon_data = m_weapon->GetCSWpnData();
			if (weapon_data) {
				data.current_damage = weapon_data->iDamage;
				data.trace_length_remaining = weapon_data->flRange - data.trace_length;
				Vector end = data.src + data.direction * data.trace_length_remaining;
				trace_line(data.src, end, MASK_SHOT | CONTENTS_GRATE, (IClientEntity*)m_local, 0, &data.enter_trace);
				if (data.enter_trace.fraction == 1.0f) return false;
				if (handle_penetration) {
					damage = data.current_damage;
					return true;
				}
			}
		}
	}
	return false;
}