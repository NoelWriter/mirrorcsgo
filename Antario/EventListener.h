#pragma once
#include <vector>
#include "SDK\IGameEvent.h"
#include "Features/DamageIndicator.h"
#include "SDK/CGlobalVarsBase.h"
#include "Settings.h"

class EventListener : public IGameEventListener2
{
public:
    EventListener(std::vector<const char*> events)
    {
        for (auto& it : events)
            g_pEventManager->AddListener(this, it, false);
    }

    ~EventListener()
    {
        g_pEventManager->RemoveListener(this);
    }

    void FireGameEvent(IGameEvent* event) override
    {
		if (!strcmp(event->GetName(), "player_hurt"))
		{
			int iAttacker = g_pEngine->GetPlayerForUserID(event->GetInt("attacker"));
			int iVictim = g_pEngine->GetPlayerForUserID(event->GetInt("userid"));
			int dmg = event->GetInt("dmg_health");

			C_BaseEntity* Attacker = g_pEntityList->GetClientEntity(iAttacker);
			C_BaseEntity* Victim = g_pEntityList->GetClientEntity(iVictim);

			if (Attacker && Victim && g::pLocalEntity)
			{
				if (Attacker == g::pLocalEntity && Victim != g::pLocalEntity)
				{
					if (g_Settings.bEspPDamageIndicator) {
						DamageIndicator_t DmgIndicator;
						DmgIndicator.iDamage = dmg;
						DmgIndicator.Player = Victim;
						DmgIndicator.flEraseTime = g::pLocalEntity->GetTickBase() * g_pGlobalVars->intervalPerTick + 2.f;
						DmgIndicator.bInitialized = false;
						g_pDamageIndicator.data.push_back(DmgIndicator);
					}

					if (g_Settings.bEspPHitsound)
						g_pEngine->ExecuteClientCmd("play physics\\metal\\bullet_metal_solid_06.wav");
				}
			}
		}

		// call functions here
    }

    int GetEventDebugID() override
    {
        return EVENT_DEBUG_ID_INIT;
    }
};