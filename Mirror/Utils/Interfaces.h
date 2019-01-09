#pragma once
#include <Windows.h>

//[enc_string_enable /]

#define CREATEINTERFACE_PROCNAME "CreateInterface"
#define VENGINE_CLIENT_INTERFACE_VERSION "VEngineClient014"
#define CLIENT_DLL_INTERFACE_VERSION "VClient018"
#define VCLIENTENTITYLIST_INTERFACE_VERSION	"VClientEntityList003"
#define INTERFACEVERSION_ENGINETRACE_CLIENT	"EngineTraceClient004"
#define VMODELINFO_CLIENT_INTERACE_VERSION "VModelInfoClient004"
#define IENGINESOUND_CLIENT_INTERFACE_VERSION "IEngineSoundClient003"
#define VENGINE_HUDMODEL_INTERFACE_VERSION "VEngineModel016"
#define VENGINE_RENDERVIEW_INTERFACE_VERSION "VEngineRenderView014"
#define MATERIAL_SYSTEM_INTERFACE_VERSION "VMaterialSystem080"
#define VGUI_SURFACE_INTERFACE_VERSION "VGUI_Surface031"
#define GAMEEVENTMANAGER_INTERFACE_VERSION	"GAMEEVENTSMANAGER002"
#define VENGINEVCAR_INTERFACE_VERSION "VEngineCvar007"
#define INPUTSYSTEM_INTERFACE_VERSION "InputSystemVersion001"
#define VENGINEVEFFECTS_INTERFACE_VERSION "VEngineEffects001"
#define STEAMAPI_DLL    "steam_api.dll"
#define ENGINE_DLL "engine.dll"
#define CLIENT_DLL "client_panorama.dll"
#define MATERIAL_DLL "materialsystem.dll"
#define VGUIMT_DLL "vguimatsurface.dll"
#define VSTDLIB_DLL	"vstdlib.dll"
#define INPUTSYSTEM_DLL	"inputsystem.dll"
#define STEAMAPI_DLL    "steam_api.dll"


namespace interfaces
{
    // Used to initialize all the interfaces at one time
    void Init();

};