#include "GlobalVars.h"

namespace g
{
    CUserCmd*			pCmd			= nullptr;
    C_BaseEntity*		pLocalEntity	= nullptr;
    std::uintptr_t		uRandomSeed		= NULL;
	C_BaseCombatWeapon*	pActiveWeapon	= nullptr;
	bool				bSendPacket		= nullptr;
	QAngle				pVisualAngles	= QAngle(0,0,0);
	bool				pThirdperson	= nullptr;

	//Aimbot
	int			   bestTarget = 0;
}
