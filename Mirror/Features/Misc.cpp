#include "Misc.h"
#include "..\SDK\CInput.h"
#include "..\Hooks.h"
#include "..\Utils\ICvar.h"
#include "..\Utils\Color.h"

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

	if (g_Settings.bMiscCrouchExploit)
		this->CrouchExploit();

	if (g_Settings.bMiscFakelag)
		this->Fakelag();
}

void Misc::DoThirdPerson() {
	if (g_pEngine->IsInGame())
	{
		QAngle viewangle;
		g_pEngine->GetViewAngles(viewangle);
		if (g::pLocalEntity->IsAlive() && g_Settings.bMiscThirdPerson)
		{
			if (!g::pThirdperson)
			{
				g_pInput->m_fCameraInThirdPerson = true;
				g_pInput->m_vecCameraOffset = Vector(viewangle.x, viewangle.y, 150.f);
				g::pThirdperson = true;
			}
			PositionCamera(g::pLocalEntity, viewangle);
		}
		else
		{
			g_pInput->m_fCameraInThirdPerson = false;
			g_pInput->m_vecCameraOffset = Vector(viewangle.x, viewangle.y, 0.f);
			g::pThirdperson = false;
		}
	}
}

bool switchPacket = false;
void Misc::CrouchExploit() {

	g::pCmd->buttons |= IN_BULLRUSH;
	/*PVOID pebp;
	__asm mov pebp, ebp;
	bool* pbSendPacket = (bool*)(*(DWORD*)pebp - 0x1C);
	bool& bSendPacket = *pbSendPacket;

	

	bool do_once = false, _do;

	if (g::pCmd->buttons & IN_DUCK)
	{
		switchPacket != switchPacket;
		if (!switchPacket) {
			bSendPacket = true;
		}
		else {
			bSendPacket = false;
			g::pCmd->buttons &= ~IN_DUCK;
		}
	}*/
}

void Misc::Fakelag()
{
	if (!g_Settings.bMiscFakelag)
		return;

	int choke = std::min<int>(false ? static_cast<int>(std::ceilf(64 / (g::pLocalEntity->GetVelocity().Length2D() * g_pGlobalVars->intervalPerTick))) : g_Settings.bMiscFakelagAmount, 14);

	if (g::pCmd->buttons & IN_ATTACK)
		return;
	if (g::pLocalEntity->GetVelocity().Length2D() < 3.0f)
		return;

	if (false && choke > 13)
		return;

	g::bSendPacket = (choked > choke);

	if (g::bSendPacket)
		choked = 0;
	else
		choked++;
}


void Misc::HandleColors() {

	// If we change selected items
	if (selectedPrev != g_Settings.bMiscColorSelected)
	{
		switch (g_Settings.bMiscColorSelected)
		{
		case COLORS_MENU:
			selectedRedPrev = cMenu.red;
			selectedGreenPrev = cMenu.green;
			selectedBluePrev = cMenu.blue;
			g_Settings.bMiscRed = cMenu.red;
			g_Settings.bMiscGreen = cMenu.green;
			g_Settings.bMiscBlue = cMenu.blue;
			break;
		case COLORS_BOX:
			selectedRedPrev = cBox.red;
			selectedGreenPrev = cBox.green;
			selectedBluePrev = cBox.blue;
			g_Settings.bMiscRed = cBox.red;
			g_Settings.bMiscGreen = cBox.green;
			g_Settings.bMiscBlue = cBox.blue;
			break;
		case COLORS_BONES:
			selectedRedPrev = cBone.red;
			selectedGreenPrev = cBone.green;
			selectedBluePrev = cBone.blue;
			g_Settings.bMiscRed = cBone.red;
			g_Settings.bMiscGreen = cBone.green;
			g_Settings.bMiscBlue = cBone.blue;
			break;
		case COLORS_DINDICATOR:
			selectedRedPrev = cDIndicator.red;
			selectedGreenPrev = cDIndicator.green;
			selectedBluePrev = cDIndicator.blue;
			g_Settings.bMiscRed = cDIndicator.red;
			g_Settings.bMiscGreen = cDIndicator.green;
			g_Settings.bMiscBlue = cDIndicator.blue;
			break;
		case COLORS_CHAMS:
			selectedRedPrev = cChams.red;
			selectedGreenPrev = cChams.green;
			selectedBluePrev = cChams.blue;
			g_Settings.bMiscRed = cChams.red;
			g_Settings.bMiscGreen = cChams.green;
			g_Settings.bMiscBlue = cChams.blue;
			break;
		case COLORS_CHAMS_XQZ:
			selectedRedPrev = cChamsXQZ.red;
			selectedGreenPrev = cChamsXQZ.green;
			selectedBluePrev = cChamsXQZ.blue;
			g_Settings.bMiscRed = cChamsXQZ.red;
			g_Settings.bMiscGreen = cChamsXQZ.green;
			g_Settings.bMiscBlue = cChamsXQZ.blue;
			break;
		case COLORS_ENEMY:
			selectedRedPrev = cEnemy.red;
			selectedGreenPrev = cEnemy.green;
			selectedBluePrev = cEnemy.blue;
			g_Settings.bMiscRed = cEnemy.red;
			g_Settings.bMiscGreen = cEnemy.green;
			g_Settings.bMiscBlue = cEnemy.blue;
			break;
		case COLORS_TEAM:
			selectedRedPrev = cTeam.red;
			selectedGreenPrev = cTeam.green;
			selectedBluePrev = cTeam.blue;
			g_Settings.bMiscRed = cTeam.red;
			g_Settings.bMiscGreen = cTeam.green;
			g_Settings.bMiscBlue = cTeam.blue;
			break;
		}
		selectedPrev = g_Settings.bMiscColorSelected;
	}

	// If we didn't change any color settings we should not reassign colors
	if ((selectedRedPrev == g_Settings.bMiscRed) && (selectedGreenPrev == g_Settings.bMiscGreen) && (selectedBluePrev == g_Settings.bMiscBlue))
		return;
	
	switch (g_Settings.bMiscColorSelected)
	{
	case COLORS_MENU:
		cMenu.red = g_Settings.bMiscRed;
		cMenu.green = g_Settings.bMiscGreen;
		cMenu.blue = g_Settings.bMiscBlue;
		break;
	case COLORS_BOX:
		cBox.red = g_Settings.bMiscRed;
		cBox.green = g_Settings.bMiscGreen;
		cBox.blue = g_Settings.bMiscBlue;
		break;
	case COLORS_BONES:
		cBone.red = g_Settings.bMiscRed;
		cBone.green = g_Settings.bMiscGreen;
		cBone.blue = g_Settings.bMiscBlue;
		break;
	case COLORS_DINDICATOR:
		cDIndicator.red = g_Settings.bMiscRed;
		cDIndicator.green = g_Settings.bMiscGreen;
		cDIndicator.blue = g_Settings.bMiscBlue;
		break;
	case COLORS_CHAMS:
		cChams.red = g_Settings.bMiscRed;
		cChams.green = g_Settings.bMiscGreen;
		cChams.blue = g_Settings.bMiscBlue;
		break;
	case COLORS_CHAMS_XQZ:
		cChamsXQZ.red = g_Settings.bMiscRed;
		cChamsXQZ.green = g_Settings.bMiscGreen;
		cChamsXQZ.blue = g_Settings.bMiscBlue;
		break;
	case COLORS_ENEMY:
		cEnemy.red = g_Settings.bMiscRed;
		cEnemy.green = g_Settings.bMiscGreen;
		cEnemy.blue = g_Settings.bMiscBlue;
		break;
	case COLORS_TEAM:
		cTeam.red = g_Settings.bMiscRed;
		cTeam.green = g_Settings.bMiscGreen;
		cTeam.blue = g_Settings.bMiscBlue;
		break;
	}

	selectedRedPrev = g_Settings.bMiscRed;
	selectedGreenPrev = g_Settings.bMiscGreen;
	selectedBluePrev = g_Settings.bMiscBlue;
}

//Color Misc::GetColor(int colorSelected)
//{
//	switch (colorSelected)
//	{
//	case COLORS_MENU:
//		return;
//	case COLORS_BOX:
//		break;
//	case COLORS_BONES:
//		break;
//	case COLORS_DINDICATOR:
//		break;
//	}
//}


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