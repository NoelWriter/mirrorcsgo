#include <thread>
#include <intrin.h>
#include "Hooks.h"
#include "Utils\Utils.h"
#include "Features\Features.h"
#include "SDK\IVModelRender.hpp"
#include "Utils/ICvar.h"

Misc     g_Misc;
Hooks    g_Hooks;
Settings g_Settings;


void Hooks::Init()
{
    // Get window handle
    while (!(g_Hooks.hCSGOWindow = FindWindowA("Valve001", nullptr)))
    {
        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(50ms);
    }

    interfaces::Init();                         // Get interfaces
    g_pNetvars = std::make_unique<NetvarTree>();// Get netvars after getting interfaces as we use them

    Utils::Log("Hooking in progress...");

    // D3D Device pointer
    const uintptr_t d3dDevice = **reinterpret_cast<uintptr_t**>(Utils::FindSignature("shaderapidx9.dll", "A1 ? ? ? ? 50 8B 08 FF 51 0C") + 1);

	// Hook WNDProc to capture mouse / keyboard input
    if (g_Hooks.hCSGOWindow)        
        g_Hooks.pOriginalWNDProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(g_Hooks.hCSGOWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Hooks::WndProc)));

    // VMTHooks
    g_Hooks.pD3DDevice9Hook			= std::make_unique<VMTHook>(reinterpret_cast<void*>(d3dDevice));
	g_Hooks.pClientHook				= std::make_unique<VMTHook>(g_pClientDll);
    g_Hooks.pClientModeHook			= std::make_unique<VMTHook>(g_pClientMode);
    g_Hooks.pSurfaceHook			= std::make_unique<VMTHook>(g_pSurface);
	g_Hooks.pRenderViewHook			= std::make_unique<VMTHook>(g_RenderView);
	g_Hooks.pModelRenderHook		= std::make_unique<VMTHook>(g_pMdlRender);
	g_Hooks.pConvarHook				= std::make_unique<VMTHook>(g_pCVar->FindVar("sv_cheats"));

    // Hook the table functions
    g_Hooks.pD3DDevice9Hook			->Hook(vtable_indexes::reset,				Hooks::Reset);
    g_Hooks.pD3DDevice9Hook			->Hook(vtable_indexes::present,				Hooks::Present);
    g_Hooks.pClientModeHook			->Hook(vtable_indexes::createMove,			Hooks::CreateMove);
    g_Hooks.pSurfaceHook			->Hook(vtable_indexes::lockCursor,			Hooks::LockCursor);
	g_Hooks.pRenderViewHook			->Hook(vtable_indexes::sceneend,			Hooks::SceneEnd);
	g_Hooks.pModelRenderHook		->Hook(vtable_indexes::drawmodelexecute,	Hooks::DrawModelExecute);
	g_Hooks.pClientHook				->Hook(vtable_indexes::framestagenotify,	Hooks::FrameStageNotify);
	//g_Hooks.pClientModeHook			->Hook(vtable_indexes::overrideView,		Hooks::OverrideView); 
	g_Hooks.pConvarHook				->Hook(vtable_indexes::getboolsvcheats,		Hooks::GetBool_SVCheats_h);

    const std::vector<const char*> vecEventNames = { "player_hurt" };
    g_Hooks.pEventListener = std::make_unique<EventListener>(vecEventNames);

    Utils::Log("Hooking completed!");
}


void Hooks::Restore()
{
	Utils::Log("Unhooking in progress...");
    {   // Unhook every function we hooked and restore original one
        g_Hooks.pD3DDevice9Hook->Unhook(vtable_indexes::reset);
        g_Hooks.pD3DDevice9Hook->Unhook(vtable_indexes::present);
        g_Hooks.pClientModeHook->Unhook(vtable_indexes::createMove);
        g_Hooks.pSurfaceHook->Unhook(vtable_indexes::lockCursor);
		g_Hooks.pRenderViewHook->Unhook(vtable_indexes::sceneend);
		g_Hooks.pModelRenderHook->Unhook(vtable_indexes::drawmodelexecute);
		g_Hooks.pClientHook->Unhook(vtable_indexes::framestagenotify);
		//g_Hooks.pClientModeHook->Unhook(vtable_indexes::overrideView);
		g_Hooks.pConvarHook->Unhook(vtable_indexes::getboolsvcheats);

        SetWindowLongPtr(g_Hooks.hCSGOWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_Hooks.pOriginalWNDProc));

        g_pNetvars.reset();   /* Need to reset by-hand, global pointer so doesnt go out-of-scope */
    }
    Utils::Log("Unhooking succeded!");

    // Destroy fonts and all textures we created
    g_Render.InvalidateDeviceObjects();
    g_Fonts.DeleteDeviceObjects();
}



bool __fastcall Hooks::CreateMove(IClientMode* thisptr, void* edx, float sample_frametime, CUserCmd* pCmd)
{
    // Call original createmove before we start screwing with it
    static auto oCreateMove = g_Hooks.pClientModeHook->GetOriginal<CreateMove_t>(vtable_indexes::createMove);
    oCreateMove(thisptr, edx, sample_frametime, pCmd);

	// Check if we can use commands
    if (!pCmd || !pCmd->command_number)
        return oCreateMove;

    // Get globals
    g::pCmd         = pCmd;
    g::pLocalEntity = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());
	g::pActiveWeapon = g::pLocalEntity->GetActiveWeapon();
	g::bSendPacket	= true;
	g::pVisualAngles = QAngle(0, 0, 0);
	g::pThirdperson = false;

	// Check globals
	if (!g::pLocalEntity)
		return oCreateMove;
	if (!g::pActiveWeapon)
		return oCreateMove;

	// Frame Pointer for bSendPacket
	uintptr_t *framePtr;
	__asm mov framePtr, ebp;

	// MovementFix 
	QAngle wish_angle = pCmd->viewangles;

	// Misc Functions
	g_Misc.DoThirdPerson();
	g_Misc.doMisc();

    engine_prediction::RunEnginePred();
	{
		// Running features in engine prediction
		g_Aimbot.DoAimbot(pCmd);
		backtracking->legitBackTrack(pCmd);
		g_AntiAim.doAntiAim(pCmd);
		g_Misc.FixMovement(pCmd, wish_angle);
	}
    engine_prediction::EndEnginePred();

	// Choke Packets
	*(bool*)(*framePtr - 0x1C) = g::bSendPacket;

	// Visual Angles for when we are in thirdperson
	g::pVisualAngles = pCmd->viewangles;

	// Clamp movement speed
	pCmd->forwardmove = g_Misc.clamp(pCmd->forwardmove, -450.f, 450.f);
	pCmd->sidemove = g_Misc.clamp(pCmd->sidemove, -450.f, 450.f);
	pCmd->upmove = g_Misc.clamp(pCmd->upmove, -320.f, 320.f);

	// Clamp viewangles.
	pCmd->viewangles.Normalize();
	Utils::normalize_angles(pCmd->viewangles);

    return false;
}

void __stdcall Hooks::FrameStageNotify(ClientFrameStage_t Stage)
{
	static auto ofunc = g_Hooks.pClientHook->GetOriginal<FrameStageNotify_t>(vtable_indexes::framestagenotify);

	if (!g::pLocalEntity)
	{
		ofunc(Stage);
		return;
	}

	if (Stage == ClientFrameStage_t::FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{
		if (g_pEngine->IsConnected())
			g_Resolver.DoResolver();
	}

	if (Stage == ClientFrameStage_t::FRAME_RENDER_START)
	{
		if (g::pLocalEntity->IsAlive() && g_pInput->m_fCameraInThirdPerson)
			g::pLocalEntity->SetVisualAngle(QAngle(g::pVisualAngles.x, g::pVisualAngles.y, 0));
	}

	if (Stage == ClientFrameStage_t::FRAME_NET_UPDATE_END)
	{
		backtracking->RageBackTrack();
	}

	ofunc(Stage);
}

void __fastcall Hooks::LockCursor(ISurface* thisptr, void* edx)
{
    static auto oLockCursor = g_Hooks.pSurfaceHook->GetOriginal<LockCursor_t>(vtable_indexes::lockCursor);

    if (!g_Settings.bMenuOpened)
        return oLockCursor(thisptr, edx);

    g_pSurface->UnlockCursor();
}


HRESULT __stdcall Hooks::Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    static auto oReset = g_Hooks.pD3DDevice9Hook->GetOriginal<Reset_t>(vtable_indexes::reset);

    if (g_Hooks.bInitializedDrawManager)
    {
        Utils::Log("Reseting draw manager.");
        g_Render.InvalidateDeviceObjects();
        HRESULT hr = oReset(pDevice, pPresentationParameters);
        g_Render.RestoreDeviceObjects(pDevice);
        Utils::Log("DrawManager reset succeded.");
        return hr;
    }

    return oReset(pDevice, pPresentationParameters);
}


HRESULT __stdcall Hooks::Present(IDirect3DDevice9* pDevice, const RECT* pSourceRect, const RECT* pDestRect, 
                                 HWND hDestWindowOverride,  const RGNDATA* pDirtyRegion)
{
    IDirect3DStateBlock9* stateBlock     = nullptr;
    IDirect3DVertexDeclaration9* vertDec = nullptr;

    pDevice->GetVertexDeclaration(&vertDec);
    pDevice->CreateStateBlock(D3DSBT_PIXELSTATE, &stateBlock);

    [pDevice]()
    {
        if (!g_Hooks.bInitializedDrawManager)
        {
            Utils::Log("Initializing Draw manager");
            g_Render.InitDeviceObjects(pDevice);
            g_Hooks.nMenu.Initialize();
            g_Hooks.bInitializedDrawManager = true;
            Utils::Log("Draw manager initialized");
        }
        else
        {
            g_Render.SetupRenderStates(); // Sets up proper render states for our state block

            static std::string szWatermark = "Mirror";
            g_Render.String(8, 8, CD3DFONT_DROPSHADOW, Color(250, 250, 250, 180), g_Fonts.pFontTahoma8.get(), szWatermark.c_str());


			// Put your draw calls here
            g_ESP.Render();

			if (g_Settings.bEspPDamageIndicator)
				g_pDamageIndicator.paint();

            if (g_Settings.bMenuOpened)
            {
				g_Misc.HandleColors();
                g_Hooks.nMenu.Render();             // Render our menu
                g_Hooks.nMenu.mouseCursor->Render();// Render mouse cursor in the end so its not overlapped
            }

        }
    }();

    stateBlock->Apply();
    stateBlock->Release();
    pDevice   ->SetVertexDeclaration(vertDec);

    static auto oPresent = g_Hooks.pD3DDevice9Hook->GetOriginal<Present_t>(17);
    return oPresent(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

void __fastcall Hooks::SceneEnd(void *pEcx, void *pEdx)
{
	static auto oSceneEnd = g_Hooks.pRenderViewHook->GetOriginal<SceneEnd_t>(vtable_indexes::sceneend);

	if (!g::pLocalEntity || !g_pEngine->IsConnected() || !g_pEngine->IsInGame())
		return;

	Vector oldOrigin;
	QAngle oldAngs;
	static IMaterial* norm = CreateMaterial(false, true, false);
	static IMaterial* flatnorm = CreateMaterial(false, false, false);
	static IMaterial* wirenorm = CreateMaterial(false, true, true);
	static IMaterial* znorm = CreateMaterial(true, true, false);
	static IMaterial* zflatnorm = CreateMaterial(true, false, false);
	static IMaterial* zwirenorm = CreateMaterial(true, true, true);

	float colorHidden[3] = { g_Misc.cChamsXQZ.red / 255.f, g_Misc.cChamsXQZ.green / 255.f, g_Misc.cChamsXQZ.blue / 255.f };
	float colorVisible[3] = { g_Misc.cChams.red / 255.f, g_Misc.cChams.green / 255.f, g_Misc.cChams.blue / 255.f };

	if (g_Settings.bEspPChams && g::pLocalEntity && g_pEngine->IsConnected() && g_pEngine->IsInGame())
	{
		auto localTeam = g::pLocalEntity->GetTeam();

		for (int it = 1; it <= g_pEngine->GetMaxClients(); ++it)
		{
			C_BaseEntity* pEntity = g_pEntityList->GetClientEntity(it);
			if (!pEntity
				|| pEntity->IsDormant()
				|| pEntity == g::pLocalEntity
				|| !pEntity->IsAlive())
				continue;

			if (!norm || !flatnorm || !wirenorm || !znorm || !zflatnorm || !zwirenorm)
				return;

			if (!pEntity || !g::pLocalEntity)
				return;

			if (pEntity->IsAlive() && pEntity->GetHealth() > 0 && pEntity->GetTeam() != g::pLocalEntity->GetTeam())
			{
				if (g_Settings.bEspPChamsInvisible)
				{
					g_RenderView->SetColorModulation(colorHidden);
					g_pMdlRender->ForcedMaterialOverride(zflatnorm);
					pEntity->DrawModel(0x01, 255);
				}
				g_RenderView->SetColorModulation(colorVisible);
				g_pMdlRender->ForcedMaterialOverride(flatnorm);
				pEntity->DrawModel(0x01, 255);

				g_pMdlRender->ForcedMaterialOverride(nullptr);
			}
			g_pMdlRender->ForcedMaterialOverride(nullptr);
		}

		return oSceneEnd(pEcx);
	}
	return oSceneEnd(pEcx);
}

void __stdcall Hooks::OverrideView(CViewSetup* pSetup)
{
	static auto oOverrideView = g_Hooks.pRenderViewHook->GetOriginal<OverrideView_t>(vtable_indexes::overrideView);

	if (g_pEngine->IsInGame() && g_pEngine->IsConnected())
	{
		if (g::pLocalEntity)
		{
			QAngle viewangle;
			g_pEngine->GetViewAngles(viewangle);
			g_pInput->m_fCameraInThirdPerson = true;
			g_pInput->m_vecCameraOffset = Vector(viewangle.x, viewangle.y, 150.f);
			//g_Misc.DoThirdPerson();
		}
	}

	return oOverrideView(pSetup);
}

void __stdcall Hooks::DrawModelExecute(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
{
	static auto oDME = g_Hooks.pModelRenderHook->GetOriginal<DrawModelExecute_t>(vtable_indexes::drawmodelexecute);

	if (!g::pLocalEntity || !g_pEngine->IsInGame() || !g_pEngine->IsConnected())
		oDME(g_pMdlRender, ctx, state, pInfo, pCustomBoneToWorld);

	oDME(g_pMdlRender, ctx, state, pInfo, pCustomBoneToWorld);
}

bool __fastcall Hooks::GetBool_SVCheats_h(PVOID pConVar, int edx)
{
	static auto oGetBool = g_Hooks.pConvarHook->GetOriginal<GetBool_t>(vtable_indexes::getboolsvcheats);

	static DWORD CAM_THINK = (DWORD)Utils::FindSignature(("client_panorama.dll"), "85 C0 75 30 38 86");
	if (!pConVar)
		return false;

	if (g_Settings.bMiscThirdPerson)
	{
		if ((DWORD)_ReturnAddress() == CAM_THINK)
			return true;
	}

	return oGetBool(pConVar);
}

LRESULT Hooks::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // for now as a lambda, to be transfered somewhere
    // Thanks uc/WasserEsser for pointing out my mistake!
    // Working when you HOLD th button, not when you press it.
    const auto getButtonHeld = [uMsg, wParam](bool& bButton, int vKey)
    {
		if (wParam != vKey) return;

        if (uMsg == WM_KEYDOWN)
            bButton = true;
        else if (uMsg == WM_KEYUP)
            bButton = false;
    };

	const auto getButtonToggle = [uMsg, wParam](bool& bButton, int vKey)
	{
		if (wParam != vKey) return;

		if (uMsg == WM_KEYUP)
			bButton = !bButton;
	};

	getButtonToggle(g_Settings.bMenuOpened, VK_INSERT);

    if (g_Hooks.bInitializedDrawManager)
    {
        // our wndproc capture fn
        if (g_Settings.bMenuOpened)
        {
            g_Hooks.nMenu.MsgProc(uMsg, wParam, lParam);
            return true;
        }
    }

    // Call original wndproc to make game use input again
    return CallWindowProcA(g_Hooks.pOriginalWNDProc, hWnd, uMsg, wParam, lParam);
}
