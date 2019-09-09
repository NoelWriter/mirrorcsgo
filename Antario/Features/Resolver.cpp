#include "Resolver.h"
#include "..\Hooks.h"
#include "..\SDK\CGlobalVarsBase.h"
#include "..\Utils\Utils.h"
#include "..\SDK\IVEngineClient.h"
#include "..\SDK\PlayerInfo.h"
#include "..\SDK\IVModelInfoClient.hpp"

Resolver g_Resolver;

void Resolver::DoResolver() 
{
	if (!g_Settings.bRagebotResolver)
		return;

	for (int i = 1; i < g_pEngine->GetMaxClients(); i++)
	{

		if (!CheckTarget(i))
			continue;

		C_BaseEntity *pEnt = g_pEntityList->GetClientEntity(i);

		cMode = GetResolverMode(pEnt);

		QAngle cViewAngle = pEnt->GetEyeAngles();
		float cLby = pEnt->GetLowerbodyYaw();

		float finalAngleY;

		switch (cMode)
		{
		case MODE_NONE:
			finalAngleY = pEnt->GetEyeAngles().y;
			break;
		case MODE_LBY:
			finalAngleY = cLby;
			break;
		case MODE_DESYNC:
			break;
		case MODE_BRUTEFORCE:
			break;
		default:
			finalAngleY = pEnt->GetEyeAngles().y;
			break;
		}

		cViewAngle.y = finalAngleY;
		pEnt->SetAngle(cViewAngle);
	}

}

int Resolver::GetResolverMode(C_BaseEntity* pEnt) 
{
	if (IsEntityMoving(pEnt)) 
	{
		return MODE_LBY;
	}

	if (false) 
	{
		return MODE_DESYNC;
	}




	return MODE_NONE;
}

bool Resolver::CheckTarget(int i)
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

	return true;
}

bool Resolver::IsEntityMoving(C_BaseEntity *player)
{
	return (player->GetVelocity().Length2D() > 0.1f && player->GetFlags() & FL_ONGROUND);
}