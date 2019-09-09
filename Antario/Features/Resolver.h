#pragma once
#include "Resolver.h"
#include "..\Hooks.h"
#include "..\SDK\CGlobalVarsBase.h"
#include "..\Utils\Utils.h"
#include "..\SDK\IVEngineClient.h"
#include "..\SDK\PlayerInfo.h"
#include "..\SDK\IVModelInfoClient.hpp"

class Resolver
{
public:

	void DoResolver();

	int GetResolverMode(C_BaseEntity * pEnt);

	bool CheckTarget(int i);

	bool IsEntityMoving(C_BaseEntity * player);

private:
	enum ResolverMode
	{
		MODE_NONE,
		MODE_LBY,
		MODE_DESYNC,
		MODE_BRUTEFORCE,
	};

	int cMode;
};

extern Resolver g_Resolver;