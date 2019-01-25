#include "Misc.h"
#include "..\SDK\CInput.h"
#include "..\Hooks.h"
#include "..\Utils\ICvar.h"

#define CAM_MIN_DIST		16.0 // Don't let the camera get any closer than ...
#define CAM_MAX_DIST		96.0 // ... or any farther away than ...
#define CAM_SWITCH_DIST		96.0 // the default camera distance when switching 1st to 3rd person
#define CAM_HULL_OFFSET		6.0  // the size of the bounding hull used for collision checking
#define START_TRANS_DIST	40.0 // how close to player when it starts making model translucent
#define TRANS_DELTA	        1.9921875 // Set to 255 / START_TRANS_DIST
#define DIST				150.0

static Vector CAM_HULL_MIN(-CAM_HULL_OFFSET, -CAM_HULL_OFFSET, -CAM_HULL_OFFSET);
static Vector CAM_HULL_MAX(CAM_HULL_OFFSET, CAM_HULL_OFFSET, CAM_HULL_OFFSET);

void Misc::SinCos(float a, float* s, float*c)
{
	*s = sin(a);
	*c = cos(a);
}

void Misc::AngleVectors(const QAngle &angles, Vector& forward, Vector& right, Vector& up)
{
	float sr, sp, sy, cr, cp, cy;

	SinCos(DEG2RAD(angles[1]), &sy, &cy);
	SinCos(DEG2RAD(angles[0]), &sp, &cp);
	SinCos(DEG2RAD(angles[2]), &sr, &cr);

	forward.x = (cp * cy);
	forward.y = (cp * sy);
	forward.z = (-sp);
	right.x = (-1 * sr * sp * cy + -1 * cr * -sy);
	right.y = (-1 * sr * sp * sy + -1 * cr *  cy);
	right.z = (-1 * sr * cp);
	up.x = (cr * sp * cy + -sr * -sy);
	up.y = (cr * sp * sy + -sr * cy);
	up.z = (cr * cp);
}

void Misc::FixMovement(CUserCmd *usercmd, QAngle &wish_angle)
{
	Vector view_fwd, view_right, view_up, cmd_fwd, cmd_right, cmd_up;
	auto viewangles = usercmd->viewangles;
	viewangles.Normalize();

	AngleVectors(wish_angle, view_fwd, view_right, view_up);
	AngleVectors(viewangles, cmd_fwd, cmd_right, cmd_up);

	const float v8 = sqrtf((view_fwd.x * view_fwd.x) + (view_fwd.y * view_fwd.y));
	const float v10 = sqrtf((view_right.x * view_right.x) + (view_right.y * view_right.y));
	const float v12 = sqrtf(view_up.z * view_up.z);

	const Vector norm_view_fwd((1.f / v8) * view_fwd.x, (1.f / v8) * view_fwd.y, 0.f);
	const Vector norm_view_right((1.f / v10) * view_right.x, (1.f / v10) * view_right.y, 0.f);
	const Vector norm_view_up(0.f, 0.f, (1.f / v12) * view_up.z);

	const float v14 = sqrtf((cmd_fwd.x * cmd_fwd.x) + (cmd_fwd.y * cmd_fwd.y));
	const float v16 = sqrtf((cmd_right.x * cmd_right.x) + (cmd_right.y * cmd_right.y));
	const float v18 = sqrtf(cmd_up.z * cmd_up.z);

	const Vector norm_cmd_fwd((1.f / v14) * cmd_fwd.x, (1.f / v14) * cmd_fwd.y, 0.f);
	const Vector norm_cmd_right((1.f / v16) * cmd_right.x, (1.f / v16) * cmd_right.y, 0.f);
	const Vector norm_cmd_up(0.f, 0.f, (1.f / v18) * cmd_up.z);

	const float v22 = norm_view_fwd.x * usercmd->forwardmove;
	const float v26 = norm_view_fwd.y * usercmd->forwardmove;
	const float v28 = norm_view_fwd.z * usercmd->forwardmove;
	const float v24 = norm_view_right.x * usercmd->sidemove;
	const float v23 = norm_view_right.y * usercmd->sidemove;
	const float v25 = norm_view_right.z * usercmd->sidemove;
	const float v30 = norm_view_up.x * usercmd->upmove;
	const float v27 = norm_view_up.z * usercmd->upmove;
	const float v29 = norm_view_up.y * usercmd->upmove;

	usercmd->forwardmove = ((((norm_cmd_fwd.x * v24) + (norm_cmd_fwd.y * v23)) + (norm_cmd_fwd.z * v25))
		+ (((norm_cmd_fwd.x * v22) + (norm_cmd_fwd.y * v26)) + (norm_cmd_fwd.z * v28)))
		+ (((norm_cmd_fwd.y * v30) + (norm_cmd_fwd.x * v29)) + (norm_cmd_fwd.z * v27));
	usercmd->sidemove = ((((norm_cmd_right.x * v24) + (norm_cmd_right.y * v23)) + (norm_cmd_right.z * v25))
		+ (((norm_cmd_right.x * v22) + (norm_cmd_right.y * v26)) + (norm_cmd_right.z * v28)))
		+ (((norm_cmd_right.x * v29) + (norm_cmd_right.y * v30)) + (norm_cmd_right.z * v27));
	usercmd->upmove = ((((norm_cmd_up.x * v23) + (norm_cmd_up.y * v24)) + (norm_cmd_up.z * v25))
		+ (((norm_cmd_up.x * v26) + (norm_cmd_up.y * v22)) + (norm_cmd_up.z * v28)))
		+ (((norm_cmd_up.x * v30) + (norm_cmd_up.y * v29)) + (norm_cmd_up.z * v27));

	usercmd->forwardmove = clamp(usercmd->forwardmove, -450.f, 450.f);
	usercmd->sidemove = clamp(usercmd->sidemove, -450.f, 450.f);
	usercmd->upmove = clamp(usercmd->upmove, -320.f, 320.f);
}

void Misc::doMisc() 
{
	if (g_Settings.bBhopEnabled)
		this->DoBhop();

}

void Misc::DoThirdPerson() {
	if (g_pEngine->IsInGame())
	{
		Utils::Log(g_pInput->m_fCameraInThirdPerson);
		static QAngle vecAngles;
		g_pEngine->GetViewAngles(vecAngles);
		if (g::pLocalEntity->IsAlive() && g_Settings.bMiscThirdPerson)
		{
			if (!g_pInput->m_fCameraInThirdPerson)
			{
				g_pInput->m_fCameraInThirdPerson = true;
				g_pInput->m_vecCameraOffset = Vector(vecAngles.x, vecAngles.y, 150.f);
			}
			PositionCamera(g::pLocalEntity, vecAngles);
			g::pThirdperson = true;
		}
		else
		{
			g_pInput->m_fCameraInThirdPerson = false;
			g_pInput->m_vecCameraOffset = Vector(vecAngles.x, vecAngles.y, 0);
			g::pThirdperson = false;
		}
	}
}


void Misc::PositionCamera(C_BaseEntity* pPlayer, QAngle angles)
{

	if (pPlayer) {
		Vector origin = pPlayer->GetOrigin() + pPlayer->GetViewOffset();

		Vector camForward, camRight, camUp;
		Misc::AngleVectors(angles, camForward, camRight, camUp);
		Vector endPos = origin;
		Vector vecCamOffset = endPos
			+ (camForward * -GetDesiredCameraOffset()[0])
			+ (camRight * GetDesiredCameraOffset()[1])
			+ (camUp * GetDesiredCameraOffset()[2]);

		CTraceFilter traceFilter;
		traceFilter.pSkip = pPlayer;

		trace_t trace;

		UTIL_TraceHull(endPos, vecCamOffset, CAM_HULL_MIN, CAM_HULL_MAX, MASK_SOLID & ~CONTENTS_MONSTER, &traceFilter, &trace);

		if (trace.fraction < 1.0) {
			g_pInput->m_vecCameraOffset[2] *= trace.fraction;
		}
	}
}

inline void Misc::UTIL_TraceHull(Vector& vecAbsStart, Vector& vecAbsEnd, Vector& hullMin,
	Vector& hullMax, unsigned int mask, ITraceFilter* pFilter, trace_t* ptr)
{
	Ray_t ray;
	ray.Init(vecAbsStart, vecAbsEnd, hullMin, hullMax);

	g_pEngineTrace->TraceRay(ray, mask, pFilter, ptr);
}

Vector Misc::GetDesiredCameraOffset()
{
	ConVar* cam_idealdist = g_pCVar->FindVar("cam_idealdist");
	ConVar* cam_idealdistright = g_pCVar->FindVar("cam_idealdistright");
	ConVar* cam_idealdistup = g_pCVar->FindVar("cam_idealdistup");

	return Vector(cam_idealdist->GetFloat(), cam_idealdistright->GetFloat(), cam_idealdistup->GetFloat());
}

template<class T, class U>
T Misc::clamp(T in, U low, U high)
{
	if (in <= low)
		return low;

	if (in >= high)
		return high;

	return in;
}