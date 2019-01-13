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

// class predefinition
class C_BaseCombatWeapon;


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
    C_BaseCombatWeapon* GetActiveWeapon()
    {
        static int m_hActiveWeapon = g_pNetvars->GetOffset("DT_BaseCombatCharacter", "m_hActiveWeapon");
        const auto weaponData      = GetValue<CBaseHandle>(m_hActiveWeapon);
        return reinterpret_cast<C_BaseCombatWeapon*>(g_pEntityList->GetClientEntityFromHandle(weaponData));
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

	Vector GetBonePos(int i)
	{
		matrix3x4_t boneMatrix[128];
		if (this->SetupBones(boneMatrix, 128, BONE_USED_BY_HITBOX, static_cast<float>(GetTickCount64())))
		{
			return Vector(boneMatrix[i][0][3], boneMatrix[i][1][3], boneMatrix[i][2][3]);
		}
		return Vector(0, 0, 0);
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
		///if (player->IsBehindSmoke(g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer())))
		///	return false;
		CGameTrace tr;
		Ray_t ray;
		CTraceFilter filter;
		filter.pSkip = this;
		auto start = GetEyePosition();
		auto dir = (pos - start).Vector::Normalize();
		ray.Init(start, pos);
		g_pEngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

		return tr.hit_entity == player || tr.fraction > 0.97f;
	}

    Vector GetEyePosition() { return this->GetOrigin() + this->GetViewOffset(); }
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
        static int m_iItemDefinitionIndex = g_pNetvars->GetOffset("DT_BaseAttributableItem", "m_AttributeManager", "m_Item", "m_iItemDefinitionIndex");
        return GetValue<ItemDefinitionIndex>(m_iItemDefinitionIndex);
    }

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
		case ItemDefinitionIndex::WEAPON_M4A1:
		case ItemDefinitionIndex::WEAPON_M4A1_SILENCER:
		case ItemDefinitionIndex::WEAPON_MAC10:
		case ItemDefinitionIndex::WEAPON_MP5SD:
		case ItemDefinitionIndex::WEAPON_MP7:
		case ItemDefinitionIndex::WEAPON_MP9:
		case ItemDefinitionIndex::WEAPON_P90:
		case ItemDefinitionIndex::WEAPON_SG556:
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

    std::string GetName()
    {
        ///TODO: Test if szWeaponName returns proper value for m4a4 / m4a1-s or it doesnt recognize them.
        return std::string(this->GetCSWpnData()->szWeaponName);
    }
};
