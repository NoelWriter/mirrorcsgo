#pragma once
#include "..\Hooks.h"

enum AA_YAW
{
	AA_YAW_OFF,
	AA_YAW_BACKWARDS,
	AA_YAW_BACKWARDS_CYCLE,
	AA_YAW_FREESTANDING,
};

class AntiAim
{
public:
	void doAntiAim(CUserCmd * pCmd);
	float GetPitch();
	float GetYaw();

	float freestanding();

private:


};

extern AntiAim g_AntiAim;