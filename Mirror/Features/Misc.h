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

	void PositionCamera(C_BaseEntity * pPlayer, QAngle angles);

	static void UTIL_TraceHull(Vector & vecAbsStart, Vector & vecAbsEnd, Vector & hullMin, Vector & hullMax, unsigned int mask, ITraceFilter * pFilter, trace_t * ptr);

	static Vector GetDesiredCameraOffset();

	template<class T, class U>
	T clamp(T in, U low, U high);
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
};

extern Misc g_Misc;
