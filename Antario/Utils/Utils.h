#pragma once
#include <Psapi.h>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include "..\SDK\IVEngineClient.h"

#define INRANGE(x,a,b)   (x >= a && x <= b)
#define GET_BYTE( x )    (GET_BITS(x[0]) << 4 | GET_BITS(x[1]))
#define GET_BITS( x )    (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define PI 3.14159265358979323846f

enum ClassId {
	ClassId_CAI_BaseNPC = 0,
	ClassId_CAK47,
	ClassId_CBaseAnimating,
	ClassId_CBaseAnimatingOverlay,
	ClassId_CBaseAttributableItem,
	ClassId_CBaseButton,
	ClassId_CBaseCombatCharacter,
	ClassId_CBaseCombatWeapon,
	ClassId_CBaseCSGrenade,
	ClassId_CBaseCSGrenadeProjectile,
	ClassId_CBaseDoor,
	ClassId_CBaseEntity,
	ClassId_CBaseFlex,
	ClassId_CBaseGrenade,
	ClassId_CBaseParticleEntity,
	ClassId_CBasePlayer,
	ClassId_CBasePropDoor,
	ClassId_CBaseTeamObjectiveResource,
	ClassId_CBaseTempEntity,
	ClassId_CBaseToggle,
	ClassId_CBaseTrigger,
	ClassId_CBaseViewModel,
	ClassId_CBaseVPhysicsTrigger,
	ClassId_CBaseWeaponWorldModel,
	ClassId_CBeam,
	ClassId_CBeamSpotlight,
	ClassId_CBoneFollower,
	ClassId_CBRC4Target,
	ClassId_CBreachCharge,
	ClassId_CBreachChargeProjectile,
	ClassId_CBreakableProp,
	ClassId_CBreakableSurface,
	ClassId_CC4,
	ClassId_CCascadeLight,
	ClassId_CChicken,
	ClassId_CColorCorrection,
	ClassId_CColorCorrectionVolume,
	ClassId_CCSGameRulesProxy,
	ClassId_CCSPlayer,
	ClassId_CCSPlayerResource,
	ClassId_CCSRagdoll,
	ClassId_CCSTeam,
	ClassId_CDangerZone,
	ClassId_CDangerZoneController,
	ClassId_CDEagle,
	ClassId_CDecoyGrenade,
	ClassId_CDecoyProjectile,
	ClassId_CDrone,
	ClassId_CDronegun,
	ClassId_CDynamicLight,
	ClassId_CDynamicProp,
	ClassId_CEconEntity,
	ClassId_CEconWearable,
	ClassId_CEmbers,
	ClassId_CEntityDissolve,
	ClassId_CEntityFlame,
	ClassId_CEntityFreezing,
	ClassId_CEntityParticleTrail,
	ClassId_CEnvAmbientLight,
	ClassId_CEnvDetailController,
	ClassId_CEnvDOFController,
	ClassId_CEnvGasCanister,
	ClassId_CEnvParticleScript,
	ClassId_CEnvProjectedTexture,
	ClassId_CEnvQuadraticBeam,
	ClassId_CEnvScreenEffect,
	ClassId_CEnvScreenOverlay,
	ClassId_CEnvTonemapController,
	ClassId_CEnvWind,
	ClassId_CFEPlayerDecal,
	ClassId_CFireCrackerBlast,
	ClassId_CFireSmoke,
	ClassId_CFireTrail,
	ClassId_CFish,
	ClassId_CFists,
	ClassId_CFlashbang,
	ClassId_CFogController,
	ClassId_CFootstepControl,
	ClassId_CFunc_Dust,
	ClassId_CFunc_LOD,
	ClassId_CFuncAreaPortalWindow,
	ClassId_CFuncBrush,
	ClassId_CFuncConveyor,
	ClassId_CFuncLadder,
	ClassId_CFuncMonitor,
	ClassId_CFuncMoveLinear,
	ClassId_CFuncOccluder,
	ClassId_CFuncReflectiveGlass,
	ClassId_CFuncRotating,
	ClassId_CFuncSmokeVolume,
	ClassId_CFuncTrackTrain,
	ClassId_CGameRulesProxy,
	ClassId_CGrassBurn,
	ClassId_CHandleTest,
	ClassId_CHEGrenade,
	ClassId_CHostage,
	ClassId_CHostageCarriableProp,
	ClassId_CIncendiaryGrenade,
	ClassId_CInferno,
	ClassId_CInfoLadderDismount,
	ClassId_CInfoMapRegion,
	ClassId_CInfoOverlayAccessor,
	ClassId_CItem_Healthshot,
	ClassId_CItemCash,
	ClassId_CItemDogtags,
	ClassId_CKnife,
	ClassId_CKnifeGG,
	ClassId_CLightGlow,
	ClassId_CMaterialModifyControl,
	ClassId_CMelee,
	ClassId_CMolotovGrenade,
	ClassId_CMolotovProjectile,
	ClassId_CMovieDisplay,
	ClassId_CParadropChopper,
	ClassId_CParticleFire,
	ClassId_CParticlePerformanceMonitor,
	ClassId_CParticleSystem,
	ClassId_CPhysBox,
	ClassId_CPhysBoxMultiplayer,
	ClassId_CPhysicsProp,
	ClassId_CPhysicsPropMultiplayer,
	ClassId_CPhysMagnet,
	ClassId_CPhysPropAmmoBox,
	ClassId_CPhysPropLootCrate,
	ClassId_CPhysPropRadarJammer,
	ClassId_CPhysPropWeaponUpgrade,
	ClassId_CPlantedC4,
	ClassId_CPlasma,
	ClassId_CPlayerResource,
	ClassId_CPointCamera,
	ClassId_CPointCommentaryNode,
	ClassId_CPointWorldText,
	ClassId_CPoseController,
	ClassId_CPostProcessController,
	ClassId_CPrecipitation,
	ClassId_CPrecipitationBlocker,
	ClassId_CPredictedViewModel,
	ClassId_CProp_Hallucination,
	ClassId_CPropCounter,
	ClassId_CPropDoorRotating,
	ClassId_CPropJeep,
	ClassId_CPropVehicleDriveable,
	ClassId_CRagdollManager,
	ClassId_CRagdollProp,
	ClassId_CRagdollPropAttached,
	ClassId_CRopeKeyframe,
	ClassId_CSCAR17,
	ClassId_CSceneEntity,
	ClassId_CSensorGrenade,
	ClassId_CSensorGrenadeProjectile,
	ClassId_CShadowControl,
	ClassId_CSlideshowDisplay,
	ClassId_CSmokeGrenade,
	ClassId_CSmokeGrenadeProjectile,
	ClassId_CSmokeStack,
	ClassId_CSpatialEntity,
	ClassId_CSpotlightEnd,
	ClassId_CSprite,
	ClassId_CSpriteOriented,
	ClassId_CSpriteTrail,
	ClassId_CStatueProp,
	ClassId_CSteamJet,
	ClassId_CSun,
	ClassId_CSunlightShadowControl,
	ClassId_CSurvivalSpawnChopper,
	ClassId_CTablet,
	ClassId_CTeam,
	ClassId_CTeamplayRoundBasedRulesProxy,
	ClassId_CTEArmorRicochet,
	ClassId_CTEBaseBeam,
	ClassId_CTEBeamEntPoint,
	ClassId_CTEBeamEnts,
	ClassId_CTEBeamFollow,
	ClassId_CTEBeamLaser,
	ClassId_CTEBeamPoints,
	ClassId_CTEBeamRing,
	ClassId_CTEBeamRingPoint,
	ClassId_CTEBeamSpline,
	ClassId_CTEBloodSprite,
	ClassId_CTEBloodStream,
	ClassId_CTEBreakModel,
	ClassId_CTEBSPDecal,
	ClassId_CTEBubbles,
	ClassId_CTEBubbleTrail,
	ClassId_CTEClientProjectile,
	ClassId_CTEDecal,
	ClassId_CTEDust,
	ClassId_CTEDynamicLight,
	ClassId_CTEEffectDispatch,
	ClassId_CTEEnergySplash,
	ClassId_CTEExplosion,
	ClassId_CTEFireBullets,
	ClassId_CTEFizz,
	ClassId_CTEFootprintDecal,
	ClassId_CTEFoundryHelpers,
	ClassId_CTEGaussExplosion,
	ClassId_CTEGlowSprite,
	ClassId_CTEImpact,
	ClassId_CTEKillPlayerAttachments,
	ClassId_CTELargeFunnel,
	ClassId_CTEMetalSparks,
	ClassId_CTEMuzzleFlash,
	ClassId_CTEParticleSystem,
	ClassId_CTEPhysicsProp,
	ClassId_CTEPlantBomb,
	ClassId_CTEPlayerAnimEvent,
	ClassId_CTEPlayerDecal,
	ClassId_CTEProjectedDecal,
	ClassId_CTERadioIcon,
	ClassId_CTEShatterSurface,
	ClassId_CTEShowLine,
	ClassId_CTesla,
	ClassId_CTESmoke,
	ClassId_CTESparks,
	ClassId_CTESprite,
	ClassId_CTESpriteSpray,
	ClassId_CTest_ProxyToggle_Networkable,
	ClassId_CTestTraceline,
	ClassId_CTEWorldDecal,
	ClassId_CTriggerPlayerMovement,
	ClassId_CTriggerSoundOperator,
	ClassId_CVGuiScreen,
	ClassId_CVoteController,
	ClassId_CWaterBullet,
	ClassId_CWaterLODControl,
	ClassId_CWeaponAug,
	ClassId_CWeaponAWP,
	ClassId_CWeaponBaseItem,
	ClassId_CWeaponBizon,
	ClassId_CWeaponCSBase,
	ClassId_CWeaponCSBaseGun,
	ClassId_CWeaponCycler,
	ClassId_CWeaponElite,
	ClassId_CWeaponFamas,
	ClassId_CWeaponFiveSeven,
	ClassId_CWeaponG3SG1,
	ClassId_CWeaponGalil,
	ClassId_CWeaponGalilAR,
	ClassId_CWeaponGlock,
	ClassId_CWeaponHKP2000,
	ClassId_CWeaponM249,
	ClassId_CWeaponM3,
	ClassId_CWeaponM4A1,
	ClassId_CWeaponMAC10,
	ClassId_CWeaponMag7,
	ClassId_CWeaponMP5Navy,
	ClassId_CWeaponMP7,
	ClassId_CWeaponMP9,
	ClassId_CWeaponNegev,
	ClassId_CWeaponNOVA,
	ClassId_CWeaponP228,
	ClassId_CWeaponP250,
	ClassId_CWeaponP90,
	ClassId_CWeaponSawedoff,
	ClassId_CWeaponSCAR20,
	ClassId_CWeaponScout,
	ClassId_CWeaponSG550,
	ClassId_CWeaponSG552,
	ClassId_CWeaponSG556,
	ClassId_CWeaponSSG08,
	ClassId_CWeaponTaser,
	ClassId_CWeaponTec9,
	ClassId_CWeaponTMP,
	ClassId_CWeaponUMP45,
	ClassId_CWeaponUSP,
	ClassId_CWeaponXM1014,
	ClassId_CWorld,
	ClassId_CWorldVguiText,
	ClassId_DustTrail,
	ClassId_MovieExplosion,
	ClassId_ParticleSmokeGrenade,
	ClassId_RocketTrail,
	ClassId_SmokeTrail,
	ClassId_SporeExplosion,
	ClassId_SporeTrail,
};

class Utils
{
public:

	

    template<unsigned int IIdx, typename TRet, typename ... TArgs>
    static auto CallVFunc(void* thisptr, TArgs ... argList) -> TRet
    {
        using Fn = TRet(__thiscall*)(void*, decltype(argList)...);
        return (*static_cast<Fn**>(thisptr))[IIdx](thisptr, argList...);
    }

	static const char* GetClientModule()
	{
		static auto h = GetModuleHandleA("client_panorama.dll");
		return (h) ? "client_panorama.dll" : "client.dll";
	}

    static uintptr_t FindSignature(const char* szModule, const char* szSignature)
    {
        const char* pat = szSignature;
        DWORD firstMatch = 0;
        DWORD rangeStart = (DWORD)GetModuleHandleA(szModule);
        MODULEINFO miModInfo;
        GetModuleInformation(GetCurrentProcess(), (HMODULE)rangeStart, &miModInfo, sizeof(MODULEINFO));
        DWORD rangeEnd = rangeStart + miModInfo.SizeOfImage;
        for (DWORD pCur = rangeStart; pCur < rangeEnd; pCur++)
        {
            if (!*pat)
                return firstMatch;

            if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == GET_BYTE(pat))
            {
                if (!firstMatch)
                    firstMatch = pCur;

                if (!pat[2])
                    return firstMatch;

                if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
                    pat += 3;

                else
                    pat += 2;
            }
            else
            {
                pat = szSignature;
                firstMatch = 0;
            }
        }
        return 0u;
    }

    // basefunct
    static std::string SetupStringParams(std::string szBasicString)
    {
        return szBasicString;
    }

    // Replace % with a desired string / value represented after semicolons. Works kinda like printf.
    template <typename T, typename... Targs>
    static std::string SetupStringParams(std::string szBasicString, T arg, Targs&& ...args)
    {
        const auto found = szBasicString.find_first_of('%');
        if (found != std::string::npos)
        {
            std::stringstream tmp;
            tmp << arg;
            szBasicString.replace(found, 1, tmp.str());
            szBasicString = SetupStringParams(szBasicString, std::forward<Targs>(args)...);
        }
        return szBasicString;
    }


    template <typename ... Args>
    static void Log(const std::string& str, Args ...arguments)
    {
        Utils::Log(Utils::SetupStringParams(str, arguments...));
    }


    /**
    *   GetCurrentSystemTime - Gets actual system time
    *   @timeInfo: Reference to your own tm variable, gets modified.
    */
    static void GetCurrentSystemTime(tm& timeInfo)
    {
        const std::chrono::system_clock::time_point systemNow = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(systemNow);
        localtime_s(&timeInfo, &now_c); // using localtime_s as std::localtime is not thread-safe.
    };


    /**
    *   Log - Logs specified message to debug console if in debug mode.
    *   Format: [HH:MM:SS] str
    *   @str: Debug message to be shown.
    */
    static void Log(const std::string& str)
    {
#ifdef _DEBUG
        tm timeInfo { };
        Utils::GetCurrentSystemTime(timeInfo);

        std::stringstream ssTime; // Temp stringstream to keep things clean
        ssTime << "[" << std::put_time(&timeInfo, "%T") << "] " << str << std::endl;

        std::cout << ssTime.str();
#endif // _DEBUG
    };



    /**
    *   Log(HRESULT) - Sets up an error message when you pass HRESULT
    *   Message contains error code only, no specific information
    */
    static void Log(const HRESULT hr)
    {
#ifdef _DEBUG
        std::stringstream strFormatted;
        strFormatted << "Error code: 0x" << std::hex << hr;

        Utils::Log(strFormatted.str());
#endif // _DEBUG
    };

	static inline float NormalizeAngle(float flAng)
	{
		if (!std::isfinite(flAng))
		{
			return 0.0f;
		}

		return std::remainder(flAng, 360.0f);
	}

	static inline void normalize_angles(QAngle& angles) {
		while (angles.y <= -180.f) angles.y += 360.f;
		while (angles.y > 180.f) angles.y -= 360.f;
		while (angles.x <= -180.f) angles.x += 360.f;
		while (angles.x > 180.f) angles.x -= 360.f;


		if (angles.x > 89.f) angles.x = 89.f;
		if (angles.x < -89.f) angles.x = -89.f;
		if (angles.y < -180.f) angles.y = -179.999f;
		if (angles.y > 180.f) angles.y = 179.999f;

		angles.z = 0.f;
	}

	static inline void ClampViewAngles(QAngle& vecAng)
	{
		vecAng.x = max(-89.0f, min(89.0f, NormalizeAngle(vecAng.x)));
		vecAng.y = NormalizeAngle(vecAng.y);
		vecAng.z = 0.0f;
	}

	static void AngleVectors( const Vector &vAngles , Vector& vForward )
	{
		float	sp , sy , cp , cy;

		sy = sin( DEG2RAD( vAngles[1] ) );
		cy = cos( DEG2RAD( vAngles[1] ) );

		sp = sin( DEG2RAD( vAngles[0] ) );
		cp = cos( DEG2RAD( vAngles[0] ) );

		vForward.x = cp*cy;
		vForward.y = cp*sy;
		vForward.z = -sp;
	}

	static float DistanceToRay(const Vector &pos, const Vector &rayStart, const Vector &rayEnd, float *along = nullptr, Vector *pointOnRay = nullptr)
	{
		Vector to = pos - rayStart;
		Vector dir = rayEnd - rayStart;
		float length = dir.NormalizeToFloat();

		float rangeAlong = dir.Dot(to);
		if (along)
			*along = rangeAlong;

		float range;

		if (rangeAlong < 0.0f)
		{
			range = -(pos - rayStart).Length();

			if (pointOnRay)
				*pointOnRay = rayStart;
		}
		else if (rangeAlong > length)
		{
			range = -(pos - rayEnd).Length();

			if (pointOnRay)
				*pointOnRay = rayEnd;
		}
		else
		{
			// TFW it can't find the operator for some reason, hardcoded then :(
			dir = Vector(rangeAlong * dir.x, rangeAlong * dir.y, rangeAlong * dir.z);
			Vector onRay = rayStart + dir;
			range = (pos - onRay).Length();

			if (pointOnRay)
				*pointOnRay = onRay;
		}

		return range;
	}

	static void VectorAngles(const Vector& forward, Vector& up, QAngle& angles)
	{
		Vector left = CrossProduct(up, forward);
		left.NormalizeInPlace();

		float forwardDist = forward.Length2D();

		if (forwardDist > 0.001f)
		{
			angles.x = atan2f(-forward.z, forwardDist) * 180 / PI;
			angles.y = atan2f(forward.y, forward.x) * 180 / PI;

			float upZ = (left.y * forward.x) - (left.x * forward.y);
			angles.z = atan2f(left.z, upZ) * 180 / PI;
		}
		else
		{
			angles.x = atan2f(-forward.z, forwardDist) * 180 / PI;
			angles.y = atan2f(-left.x, left.y) * 180 / PI;
			angles.z= 0;
		}
	}

	static void VectorAngles(const Vector& forward, QAngle &angles)
	{
		float tmp, yaw, pitch;

		if (forward[1] == 0 && forward[0] == 0)
		{
			yaw = 0;
			if (forward[2] > 0)
				pitch = 270;
			else
				pitch = 90;
		}
		else
		{
			yaw = (atan2(forward[1], forward[0]) * 180.f / M_PI);
			if (yaw < 0)
				yaw += 360;

			tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
			pitch = (atan2(-forward[2], tmp) * 180.f / M_PI);
			if (pitch < 0)
				pitch += 360;
		}

		angles[0] = pitch;
		angles[1] = yaw;
		angles[2] = 0;
	}

	static QAngle CalcAngle(Vector src, Vector dst)
	{
		QAngle ret;
		VectorAngles(dst - src, ret);
		ret.Normalize();
		return ret;
	}

	static Vector CalcVecAngle(Vector src, Vector dst) {
		Vector angles;

		Vector delta = src - dst;
		float hyp = delta.Length2D();

		angles.y = std::atanf(delta.y / delta.x) * 57.2957795131f;
		angles.x = std::atanf(-delta.z / hyp) * -57.2957795131f;
		angles.z = 0.0f;

		if (delta.x >= 0.0f)
			angles.y += 180.0f;

		return angles;
	}

    /**
    *   WorldToScreen - Converts game coordinates to screen
    *   @origin: 3D coordinates to be converted
    *   @screen: Viewport coordinates to which we will convert
    */
    static bool WorldToScreen(const Vector &origin, Vector &screen)
    {
        const auto screenTransform = [&origin, &screen]() -> bool
        {
            static std::uintptr_t pViewMatrix;
            if (!pViewMatrix)
            {
                pViewMatrix = static_cast<std::uintptr_t>(Utils::FindSignature("client_panorama.dll", "0F 10 05 ? ? ? ? 8D 85 ? ? ? ? B9"));
                pViewMatrix += 3;
                pViewMatrix = *reinterpret_cast<std::uintptr_t*>(pViewMatrix);
                pViewMatrix += 176;
            }

            const VMatrix& w2sMatrix = *reinterpret_cast<VMatrix*>(pViewMatrix);
            screen.x = w2sMatrix.m[0][0] * origin.x + w2sMatrix.m[0][1] * origin.y + w2sMatrix.m[0][2] * origin.z + w2sMatrix.m[0][3];
            screen.y = w2sMatrix.m[1][0] * origin.x + w2sMatrix.m[1][1] * origin.y + w2sMatrix.m[1][2] * origin.z + w2sMatrix.m[1][3];
            screen.z = 0.0f;

            float w = w2sMatrix.m[3][0] * origin.x + w2sMatrix.m[3][1] * origin.y + w2sMatrix.m[3][2] * origin.z + w2sMatrix.m[3][3];

            if (w < 0.001f)
            {
                screen.x *= 100000;
                screen.y *= 100000;
                return true;
            }

            float invw = 1.f / w;
            screen.x *= invw;
            screen.y *= invw;

            return false;
        };

        if (!screenTransform())
        {
            int iScreenWidth, iScreenHeight;
            g_pEngine->GetScreenSize(iScreenWidth, iScreenHeight);

            screen.x = (iScreenWidth * 0.5f) + (screen.x * iScreenWidth) * 0.5f;
            screen.y = (iScreenHeight * 0.5f) - (screen.y * iScreenHeight) * 0.5f;

            return true;
        }
        return false;
    }
};
