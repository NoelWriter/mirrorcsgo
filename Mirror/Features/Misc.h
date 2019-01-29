#pragma once
#include "..\Utils\GlobalVars.h"
#include "..\Settings.h"

class Misc
{
public:
	void SinCos(float a, float * s, float * c);
	void AngleVectors(const QAngle & angles, Vector & forward, Vector & right, Vector & up);
	void Misc::FixMovement(CUserCmd *usercmd, QAngle &wish_angle);

	void doMisc();

	void RankRevealAll();

	void DoThirdPerson();

	void CrouchExploit();

	void HandleColors();

	Color GetColor(int colorSelected);

	void PositionCamera(C_BaseEntity * pPlayer, QAngle angles);

	static void UTIL_TraceHull(Vector & vecAbsStart, Vector & vecAbsEnd, Vector & hullMin, Vector & hullMax, unsigned int mask, ITraceFilter * pFilter, trace_t * ptr);

	static Vector GetDesiredCameraOffset();

	template<class T, class U>
	T clamp(T in, U low, U high);

	Color cMenu = { 250, 45, 110, 255 };
	Color cBox = { 0, 0, 0, 255 };
	Color cBone = { 255, 255, 255, 255 };
	Color cDIndicator = { 250, 165, 110, 255 };
	Color cChams = { 250, 45, 110, 255 };
	Color cChamsXQZ = { 0, 0, 0, 255 };
	Color cEnemy = { 250, 165, 110, 255 };
	Color cTeam = { 195, 240, 100, 255 };

private:

    void DoBhop() const
    {
		if (!g::pLocalEntity->IsAlive())
			return;

        if (g::pLocalEntity->GetMoveType() == MoveType_t::MOVETYPE_LADDER)
            return;

        static bool bLastJumped = false;
        static bool bShouldFake = false;

        if (!bLastJumped && bShouldFake)
        {
            bShouldFake = false;
			g::pCmd->buttons |= IN_JUMP;
        }
        else if (g::pCmd->buttons & IN_JUMP)
        {
            if (g::pLocalEntity->GetFlags() & FL_ONGROUND)
                bShouldFake = bLastJumped = true;
            else 
            {
				g::pCmd->buttons &= ~IN_JUMP;
                bLastJumped = false;
            }
        }
        else
            bShouldFake = bLastJumped = false;
    }

	enum colorSelected
	{
		COLORS_MENU,
		COLORS_BOX,
		COLORS_BONES,
		COLORS_DINDICATOR,
		COLORS_CHAMS,
		COLORS_CHAMS_XQZ,
		COLORS_ENEMY,
		COLORS_TEAM
	};

	int selectedPrev;
	int selectedRedPrev;
	int selectedGreenPrev;
	int selectedBluePrev;

	
};

extern Misc g_Misc;
