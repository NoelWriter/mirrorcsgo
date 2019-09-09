#pragma once
#include "Definitions.h"
#include "IClientUnknown.h"
#include "IClientEntityList.h"
#include "..\Utils\Utils.h"
#include "..\Utils\NetvarManager.h"
#include "Vector.h"
#include "Studio.hpp"
#include "..\SDK\Studio.hpp"
#include "..\SDK\IEngineTrace.hpp"
#include "..\SDK\IVModelInfoClient.hpp"
#include <array>
#include "..\Utils\UtlVector.hpp"

// class predefinition
class C_BaseCombatWeapon;

class AnimationLayer
{
public:
	char  pad_0000[20];
	// These should also be present in the padding, don't see the use for it though
	//float	m_flLayerAnimtime;
	//float	m_flLayerFadeOuttime;
	uint32_t m_nOrder; //0x0014
	uint32_t m_nSequence; //0x0018
	float_t m_flPrevCycle; //0x001C
	float_t m_flWeight; //0x0020
	float_t m_flWeightDeltaRate; //0x0024
	float_t m_flPlaybackRate; //0x0028
	float_t m_flCycle; //0x002C
	void *m_pOwner; //0x0030 // player's thisptr
	char  pad_0038[4]; //0x0034
}; //Size: 0x0038

class VarMapEntry_t
{
public:
	unsigned short type;
	unsigned short m_bNeedsToInterpolate;	// Set to false when this var doesn't
											// need Interpolate() called on it anymore.
	void *data;
	void *watcher;
};

struct VarMapping_t
{
	CUtlVector<VarMapEntry_t> m_Entries;
	int m_nInterpolatedEntries;
	float m_lastInterpolationTime;
};

class C_BaseEntity : public IClientUnknown, public IClientRenderable, public IClientNetworkable
{
private:
    template<class T>
    T GetPointer(const int offset)
    {
        return reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(this) + offset);
    }
    // To get value from the pointer itself
    template<class T>
    T GetValue(const int offset)
    {
        return *reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(this) + offset);
    }

public:
	static __forceinline C_BaseEntity* GetEntityByIndex(int index)
	{
		return static_cast<C_BaseEntity*>(g_pEntityList->GetClientEntity(index));
	}

    C_BaseCombatWeapon* GetActiveWeapon()
    {
        static int m_hActiveWeapon = g_pNetvars->GetOffset("DT_BaseCombatCharacter", "m_hActiveWeapon");
        const auto weaponData      = GetValue<CBaseHandle>(m_hActiveWeapon);
        return reinterpret_cast<C_BaseCombatWeapon*>(g_pEntityList->GetClientEntityFromHandle(weaponData));
    }

    int GetHitboxSet()
    {	
        static int m_nHitboxSet = g_pNetvars->GetOffset("DT_BasePlayer", "m_nHitboxSet");
        return GetValue<int>(m_nHitboxSet);
    }

	void C_BaseEntity::SetAbsOrigin(const Vector &origin)
	{
		using SetAbsOriginFn = void(__thiscall*)(void*, const Vector &origin);
		static SetAbsOriginFn SetAbsOrigin = (SetAbsOriginFn)Utils::FindSignature(("client_panorama.dll"), "55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8");

		SetAbsOrigin(this, origin);
	}

	void C_BaseEntity::SetAbsAngles(const QAngle &angles)
	{
		using SetAbsAnglesFn = void(__thiscall*)(void*, const QAngle &angles);
		static SetAbsAnglesFn SetAbsAngles = (SetAbsAnglesFn)Utils::FindSignature(("client_panorama.dll"), "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1 E8");

		SetAbsAngles(this, angles);
	}

	std::array<float, 24> &C_BaseEntity::m_flPoseParameter()
	{
		static int _m_flPoseParameter = g_pNetvars->GetOffset("DT_BaseAnimating", "m_flPoseParameter");
		return *(std::array<float, 24>*)((uintptr_t)this + _m_flPoseParameter);
	}

	int C_BaseEntity::GetNumAnimOverlays()
	{
		return *(int*)((DWORD)this + 0x298C);
	}

	AnimationLayer *C_BaseEntity::GetAnimOverlays()
	{
		// to find offset: use 9/12/17 dll
		// sig: 55 8B EC 51 53 8B 5D 08 33 C0
		return *(AnimationLayer**)((DWORD)this + 0x2980);
	}

	AnimationLayer *C_BaseEntity::GetAnimOverlay(int i)
	{
		if (i < 15)
			return &GetAnimOverlays()[i];
		return nullptr;
	}

	void C_BaseEntity::InvalidateBoneCache()
	{
		static bool hasSignature = false;
		static unsigned long g_iModelBoneCounter;
		if (!hasSignature)
		{
			DWORD invalidateBoneCache = (DWORD)Utils::FindSignature(("client_panorama.dll"), "80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81");
			g_iModelBoneCounter = **(unsigned long**)(invalidateBoneCache + 10);
			hasSignature = true;
		}
		*(unsigned int*)((DWORD)this + 0x2924) = 0xFF7FFFFF; // m_flLastBoneSetupTime = -FLT_MAX;
		*(unsigned int*)((DWORD)this + 0x2690) = (g_iModelBoneCounter - 1); // m_iMostRecentModelBoneCounter = g_iModelBoneCounter - 1;
	}

	VarMapping_t *C_BaseEntity::VarMapping()
	{
		return reinterpret_cast<VarMapping_t*>((DWORD)this + 0x24);
	}

	QAngle C_BaseEntity::GetVAngles()
	{
		static auto deadflag = g_pNetvars->GetOffset("DT_BasePlayer", "deadflag");
		return *(QAngle*)((uintptr_t)this + deadflag + 0x4);
	}

	void SetVisualAngle(QAngle target)
	{
		static int deadflag = g_pNetvars->GetOffset("DT_BasePlayer", "pl", "deadflag");
		*(QAngle*)((uintptr_t)this + deadflag + 0x4) = target;
	}

	void SetAngle(QAngle target)
	{
		static int m_angEyeAngles = g_pNetvars->GetOffset("DT_CSPlayer", "m_angEyeAngles[0]");
		*(QAngle*)((uintptr_t)this + m_angEyeAngles) = target;
	}

	int GetTeam()
	{
		static int m_iTeamNum = g_pNetvars->GetOffset("DT_BaseEntity", "m_iTeamNum");
		return GetValue<int>(m_iTeamNum);
	}

    EntityFlags GetFlags()
    {
        static int m_fFlags = g_pNetvars->GetOffset("DT_BasePlayer", "m_fFlags");
        return GetValue<EntityFlags>(m_fFlags);
    }

	EntityFlags* GetFlags2()
	{
		static int m_fFlags = g_pNetvars->GetOffset("DT_BasePlayer", "m_fFlags");
		return (EntityFlags*)((uintptr_t)this + m_fFlags);
	}

	const Vector C_BaseEntity::WorldSpaceCenter()
	{
		Vector vecOrigin = GetOrigin();

		Vector min = this->GetCollideable()->OBBMins() + vecOrigin;
		Vector max = this->GetCollideable()->OBBMaxs() + vecOrigin;

		Vector size = max - min;
		size /= 2.f;
		size += min;

		return size;
	}

    MoveType_t GetMoveType()
    {
        static int m_Movetype = g_pNetvars->GetOffset("DT_BaseEntity", "m_nRenderMode") + 1;
        return GetValue<MoveType_t>(m_Movetype);
    }

    bool GetLifeState()
    {
        static int m_lifeState = g_pNetvars->GetOffset("DT_BasePlayer", "m_lifeState");
        return GetValue<bool>(m_lifeState);
    }

    int GetHealth()
    {
        static int m_iHealth = g_pNetvars->GetOffset("DT_BasePlayer", "m_iHealth");
        return GetValue<int>(m_iHealth);
    }

    bool IsAlive() { return this->GetHealth() > 0 && this->GetLifeState() == 0; }

	bool IsImmune()
	{
		static int m_bGunGameImmunity = g_pNetvars->GetOffset("DT_CSPlayer", "m_bGunGameImmunity");
		return GetValue<bool>(m_bGunGameImmunity);
	}

	QAngle GetEyeAngles()
	{
		static int m_angEyeAngles = g_pNetvars->GetOffset("DT_CSPlayer", "m_angEyeAngles[0]");
		return GetValue<QAngle>(m_angEyeAngles);
	}


	float GetLowerbodyYaw()
	{
		static int m_flLowerBodyYawTarget = g_pNetvars->GetOffset("DT_CSPlayer", "m_flLowerBodyYawTarget");
		return GetValue<float>(m_flLowerBodyYawTarget);
	}

    int GetTickBase()
    {
        static int m_nTickBase = g_pNetvars->GetOffset("DT_BasePlayer", "localdata", "m_nTickBase");
        return GetValue<int>(m_nTickBase);
    }

    Vector GetOrigin()
    {
        static int m_vecOrigin = g_pNetvars->GetOffset("DT_BaseEntity", "m_vecOrigin");
        return GetValue<Vector>(m_vecOrigin);
    }

    Vector GetViewOffset()
    {
        static int m_vecViewOffset = g_pNetvars->GetOffset("DT_BasePlayer", "localdata", "m_vecViewOffset[0]");
        return GetValue<Vector>(m_vecViewOffset);
    }

	void SetAngle2(QAngle wantedang)
	{
		typedef void(__thiscall* SetAngleFn)(void*, const QAngle &);
		static SetAngleFn SetAngle2 = reinterpret_cast<SetAngleFn>(Utils::FindSignature("client_panorama.dll", "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1"));
		SetAngle2(this, wantedang);
	}

	Vector GetBonePos(int i)
	{
		matrix3x4_t boneMatrix[128];
		if (this->SetupBones(boneMatrix, 128, BONE_USED_BY_HITBOX, static_cast<float>(GetTickCount64())))
		{
			return Vector(boneMatrix[i][0][3], boneMatrix[i][1][3], boneMatrix[i][2][3]);
		}
		return Vector(0, 0, 0);
	}

	bool C_BaseEntity::SetupBones2(matrix3x4_t *pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime)
	{
		auto backupval = *reinterpret_cast<uint8_t*>((uintptr_t)this + 0x274);

		*reinterpret_cast<uint8_t*>((uintptr_t)this + 0x274) = 0;
		bool setuped_bones = this->SetupBones(pBoneToWorldOut, nMaxBones, boneMask, currentTime);
		*reinterpret_cast<uint8_t*>((uintptr_t)this + 0x274) = backupval;

		return setuped_bones;
	}

	QAngle GetPunchAngles()
	{
		//static int m_viewPunchAngle = g_pNetvars->GetOffset("DT_CSPlayer", "m_viewPunchAngle");
		//return GetValue<QAngle>(m_viewPunchAngle);
		return *(QAngle*)((DWORD)this + 0x302C);
	}

	bool C_BaseEntity::IsBehindSmoke(C_BaseEntity* localPlayer) {
		if (!localPlayer)
			return false;
		static auto LineGoesThroughSmoke
			= reinterpret_cast<bool(__cdecl*) (Vector, Vector)>(
				Utils::FindSignature(Utils::GetClientModule(), "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0")
				);
		Vector vLocalOrigin = localPlayer->GetBonePos(8);
		Vector vTargetOrigin = this->GetBonePos(8);

		return LineGoesThroughSmoke(vLocalOrigin, vTargetOrigin);
	}

	IClientRenderable* GetRenderable()
	{
		return reinterpret_cast<IClientRenderable*>((DWORD)this + 0x4);
	}

	//int draw_model(int flags, uint8_t alpha) {
	//	using fn = int(__thiscall*)(void*, int, uint8_t);
	//	return call_vfunc< fn >(GetRenderable(), 9)(GetRenderable(), flags, alpha);
	//}

	bool hasHelmet()
	{
		static int m_bHasHelmet = g_pNetvars->GetOffset("DT_CSPlayer", "m_bHasHelmet");
		return GetValue<bool>(m_bHasHelmet);
	}

	bool hasHeavyArmor()
	{
		static int m_bHasHeavyArmor = g_pNetvars->GetOffset("DT_CSPlayer", "m_bHasHeavyArmor");
		return GetValue<bool>(m_bHasHeavyArmor);
	}

	int GetArmor()
	{
		static int m_ArmorValue = g_pNetvars->GetOffset("DT_CSPlayer", "m_ArmorValue");
		return GetValue<int>(m_ArmorValue);
	}

	bool hasDefuser()
	{
		static int m_bHasDefuser = g_pNetvars->GetOffset("DT_CSPlayer", "m_bHasDefuser");
		return GetValue<bool>(m_bHasDefuser);
	}

	Vector GetVelocity()
	{
		static int m_vecVelocity = g_pNetvars->GetOffset("DT_BasePlayer", "localdata", "m_vecVelocity[0]");
		return *(Vector*)((uintptr_t)this + m_vecVelocity);
	}

	float GetSimulationTime()
	{
		static int m_flSimulationTime = g_pNetvars->GetOffset("DT_BaseEntity", "m_flSimulationTime");
		return GetValue<float>(m_flSimulationTime);
	}


	Vector GetAimPunchAngle()
	{
		static int m_aimPunchAngle = g_pNetvars->GetOffset("DT_BasePlayer", "m_aimPunchAngle");
		return *(Vector*)(m_aimPunchAngle);
	}

	QAngle& GetAngles()
	{
		return Utils::CallVFunc<11, QAngle&>(this);
	}

	bool C_BaseEntity::CanSeePlayer(C_BaseEntity* player, const Vector& pos)
	{
		if (!player || !player->IsAlive() || player->IsDormant())
			return false;

		CGameTrace tr;
		Ray_t ray;
		CTraceFilter filter;

		filter.pSkip = this;
		auto start = GetEyePosition();

		ray.Init(start, pos);
		g_pEngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

		if (tr.fraction == 1.f)
			return false;

		return tr.hit_entity == player || tr.fraction > 0.97f;
	}

	Vector GetEyePosition() {
		return this->GetOrigin() + this->GetViewOffset();
	}
};


class C_BaseCombatWeapon : public C_BaseEntity
{
private:
    template<class T>
    T GetPointer(const int offset)
    {
        return reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(this) + offset);
    }
    // To get value from the pointer itself
    template<class T>
    T GetValue(const int offset)
    {
        return *reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(this) + offset);
    }

public:
    ItemDefinitionIndex GetItemDefinitionIndex()
    {
		if (!this)
			return ItemDefinitionIndex::WEAPON_XM1014;

        static int m_iItemDefinitionIndex = g_pNetvars->GetOffset("DT_BaseAttributableItem", "m_AttributeManager", "m_Item", "m_iItemDefinitionIndex");
        return GetValue<ItemDefinitionIndex>(m_iItemDefinitionIndex);
    } // Crash 3x

    float GetNextPrimaryAttack()
    {
        static int m_flNextPrimaryAttack = g_pNetvars->GetOffset("DT_BaseCombatWeapon", "LocalActiveWeaponData", "m_flNextPrimaryAttack");
        return GetValue<float>(m_flNextPrimaryAttack);
    }

    int GetAmmo()
    {
        static int m_iClip1 = g_pNetvars->GetOffset("DT_BaseCombatWeapon", "m_iClip1");
        return GetValue<int>(m_iClip1);
    }

    WeaponInfo_t* GetCSWpnData()
    {
        return Utils::CallVFunc<448, WeaponInfo_t*>(this);
    }

	bool isRifle()
	{
		auto itemIndex = this->GetItemDefinitionIndex();

		switch (itemIndex)
		{
		case ItemDefinitionIndex::WEAPON_AK47:
		case ItemDefinitionIndex::WEAPON_AUG:
		case ItemDefinitionIndex::WEAPON_GALILAR:
		case ItemDefinitionIndex::WEAPON_FAMAS:
		case ItemDefinitionIndex::WEAPON_M4A1:
		case ItemDefinitionIndex::WEAPON_M4A1_SILENCER:
		case ItemDefinitionIndex::WEAPON_MAC10:
		case ItemDefinitionIndex::WEAPON_MP5SD:
		case ItemDefinitionIndex::WEAPON_MP7:
		case ItemDefinitionIndex::WEAPON_MP9:
		case ItemDefinitionIndex::WEAPON_P90:
		case ItemDefinitionIndex::WEAPON_SG553:
		case ItemDefinitionIndex::WEAPON_UMP45:
			return true;


		default:
			return false;

		}

	}

	bool isSniper()
	{
		auto itemIndex = this->GetItemDefinitionIndex();
		switch (itemIndex)
		{
		case ItemDefinitionIndex::WEAPON_AWP:
		case ItemDefinitionIndex::WEAPON_SSG08:
		case ItemDefinitionIndex::WEAPON_SCAR20:
		case ItemDefinitionIndex::WEAPON_G3SG1:
			return true;
		default:
			return false;
		}
	}

	bool isGrenade()
	{
		auto itemIndex = this->GetItemDefinitionIndex();
		switch (itemIndex)
		{
		case ItemDefinitionIndex::WEAPON_HEGRENADE:
		case ItemDefinitionIndex::WEAPON_INCGRENADE:
		case ItemDefinitionIndex::WEAPON_MOLOTOV:
		case ItemDefinitionIndex::WEAPON_SMOKEGRENADE:
		case ItemDefinitionIndex::WEAPON_FLASHBANG:
		case ItemDefinitionIndex::WEAPON_DECOY:
			return true;
		default:
			return false;
		}

	}

	bool isPistol()
	{
		auto itemIndex = this->GetItemDefinitionIndex();
		switch (itemIndex)
		{

		case ItemDefinitionIndex::WEAPON_DEAGLE:
		case ItemDefinitionIndex::WEAPON_CZ75A:
		case ItemDefinitionIndex::WEAPON_FIVESEVEN:
		case ItemDefinitionIndex::WEAPON_ELITE:
		case ItemDefinitionIndex::WEAPON_GLOCK:
		case ItemDefinitionIndex::WEAPON_HKP2000:
		case ItemDefinitionIndex::WEAPON_P250:
		case ItemDefinitionIndex::WEAPON_TEC9:
		case ItemDefinitionIndex::WEAPON_REVOLVER:
		case ItemDefinitionIndex::WEAPON_USP_SILENCER:
			return true;
		default:
			return false;
		}
	}

	float GetInaccuracy()
	{
		return Utils::CallVFunc<471, float>(this);
	}

	float GetSpread()
	{
		return Utils::CallVFunc<440, float>(this);
	}

	void UpdateAccuracyPenalty()
	{
		return Utils::CallVFunc<472, void>(this);
	}

    std::string GetName()
    {
        ///TODO: Test if szWeaponName returns proper value for m4a4 / m4a1-s or it doesnt recognize them.
        return std::string(this->GetCSWpnData()->szWeaponName);
    }
};
