#pragma once
#include "..\Utils\DrawManager.h"
#include "..\Utils\GlobalVars.h"
#include "..\Settings.h"

struct DamageIndicator_t {
	int iDamage;
	bool bInitialized;
	float flEraseTime;
	float flLastUpdate;
	C_BaseEntity * Player;
	Vector Position;
};

class DamageIndicators {
public:
	std::vector<DamageIndicator_t> data;
	void paint();
};
extern DamageIndicators g_pDamageIndicator;
