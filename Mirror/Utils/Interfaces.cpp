#include "Interfaces.h"
#include "Utils.h"
#include "ICvar.h"

#include "..\SDK\IClientMode.h"
#include "..\SDK\IBaseClientDll.h"
#include "..\SDK\IClientEntityList.h"
#include "..\SDK\IVEngineClient.h"
#include "..\SDK\CPrediction.h"
#include "..\SDK\IGameEvent.h"
#include "..\SDK\ISurface.h"
#include "..\SDK\IVModelInfoClient.hpp"
#include "..\SDK\IVModelRender.hpp"
#include "..\SDK\IPhysics.h"
#include "..\SDK\IEngineTrace.hpp"

// Initializing global interfaces

IBaseClientDLL*     g_pClientDll    = nullptr;
IClientMode*        g_pClientMode   = nullptr;
IClientEntityList*  g_pEntityList   = nullptr;
IVEngineClient*     g_pEngine       = nullptr;
CPrediction*        g_pPrediction   = nullptr;
IGameMovement*      g_pMovement     = nullptr;
IMoveHelper*        g_pMoveHelper   = nullptr;
CGlobalVarsBase*    g_pGlobalVars   = nullptr;
IGameEventManager2* g_pEventManager = nullptr;
ISurface*           g_pSurface      = nullptr;
IEngineTrace*		g_pEngineTrace	= nullptr;
IVModelInfoClient*  g_pMdlInfo		= nullptr;
CVRenderView*		g_RenderView	= nullptr;
CMaterialSystem*    g_MaterialSystem= nullptr;
IVModelRender*      g_pMdlRender    = nullptr;
IPhysicsSurfaceProps* g_pPhysSurface= nullptr;
ICVar*				  g_pCVar		= nullptr;

namespace interfaces
{
    template<typename T>
    T* CaptureInterface(const char* szModuleName, const char* szInterfaceVersion)
    {
        HMODULE moduleHandle = GetModuleHandleA(szModuleName);
        if (moduleHandle)   /* In case of not finding module handle, throw an error. */
        {
            CreateInterfaceFn pfnFactory = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(moduleHandle, "CreateInterface"));
            return reinterpret_cast<T*>(pfnFactory(szInterfaceVersion, nullptr));
        }
        Utils::Log("Error getting interface %", szInterfaceVersion);
        return nullptr;
    }


    void Init()
    {
        g_pClientDll    = CaptureInterface<IBaseClientDLL>("client_panorama.dll", "VClient018");					// Get IBaseClientDLL
        g_pClientMode   = **reinterpret_cast<IClientMode***>    ((*reinterpret_cast<uintptr_t**>(g_pClientDll))[10] + 0x5u);  // Get IClientMode
        g_pGlobalVars   = **reinterpret_cast<CGlobalVarsBase***>((*reinterpret_cast<uintptr_t**>(g_pClientDll))[0]  + 0x1Bu); // Get CGlobalVarsBase
        g_pEntityList   = CaptureInterface<IClientEntityList>("client_panorama.dll", "VClientEntityList003");    // Get IClientEntityList
        g_pEngine       = CaptureInterface<IVEngineClient>("engine.dll", "VEngineClient014");						// Get IVEngineClient
        g_pPrediction   = CaptureInterface<CPrediction>("client_panorama.dll", "VClientPrediction001");          // Get CPrediction
        g_pMovement     = CaptureInterface<IGameMovement>("client_panorama.dll", "GameMovement001");             // Get IGameMovement
        g_pMoveHelper   = **reinterpret_cast<IMoveHelper***>((Utils::FindSignature("client_panorama.dll", "8B 0D ? ? ? ? 8B 46 08 68") + 0x2));  // Get IMoveHelper
        g_pEventManager = CaptureInterface<IGameEventManager2>("engine.dll", "GAMEEVENTSMANAGER002");				// Get IGameEventManager2
        g_pSurface      = CaptureInterface<ISurface>("vguimatsurface.dll", "VGUI_Surface031");						// Get ISurface
		g_pEngineTrace  = CaptureInterface<IEngineTrace>("engine.dll", "EngineTraceClient004"); // Get EngineTrace
		g_pMdlInfo		= CaptureInterface<IVModelInfoClient>("engine.dll", "VModelInfoClient004"); // GetModelInfo
		g_RenderView	= CaptureInterface<CVRenderView>("engine.dll", "VEngineRenderView014");
		g_MaterialSystem= CaptureInterface<CMaterialSystem>("materialsystem.dll", "VMaterialSystem080");
		g_pMdlRender	= CaptureInterface<IVModelRender>("engine.dll", "VEngineModel016");
		g_pPhysSurface	= CaptureInterface<IPhysicsSurfaceProps>("vphysics.dll", "VPhysicsSurfaceProps001");
		g_pCVar			= CaptureInterface<ICVar>("vstdlib.dll", "VEngineCvar007");
    }
}
