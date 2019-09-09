#pragma once
#include "..\SDK\CInput.h"
#include "..\SDK\CEntity.h"

namespace g
{
    extern CUserCmd*			pCmd;
    extern C_BaseEntity*		pLocalEntity;
    extern std::uintptr_t		uRandomSeed;
	extern C_BaseCombatWeapon*	pActiveWeapon;
	extern bool					bSendPacket;
	extern QAngle				pVisualAngles;
	extern bool					pThirdperson;

	//Aimbot
	extern int			  bestTarget;
}
