#pragma once
#include "Color.h"
#include "Utils.h"
#define _CRT_SECURE_NO_WARNINGS

// The default, no flags at all
#define FCVAR_NONE                0 

class ConCommandBase;
class ConCommand;
class ConVar;

class ConVar
{
public:
	Color GetColor()
	{
		return Utils::CallVFunc<10, Color>(this);
	}

	const char* GetString()
	{
		return Utils::CallVFunc<11, const char*>(this);
	}

	float GetFloat()
	{
		return Utils::CallVFunc<12, float>(this);
	}

	int GetInt()
	{
		return Utils::CallVFunc<13, int>(this);
	}

	void SetValue(const char *value)
	{
		Utils::CallVFunc<14, void>(this, value);
	}

	void SetValue(float value)
	{
		Utils::CallVFunc<15, void>(this, value);
	}

	void SetValue(int value)
	{
		Utils::CallVFunc<16, void>(this, value);
	}

	void SetValue(Color value)
	{
		Utils::CallVFunc<17, void>(this, value);
	}

	bool GetBool()
	{
		return !!GetInt();
	}

	char pad_0x0000[0x4]; //0x0000
	ConVar *pNext; //0x0004
	int32_t bRegistered; //0x0008
	char *pszName; //0x000C
	char *pszHelpString; //0x0010
	int32_t nFlags; //0x0014
	char pad_0x0018[0x4]; //0x0018
	ConVar *pParent; //0x001C
	char *pszDefaultValue; //0x0020
	char *strString; //0x0024
	int32_t StringLength; //0x0028
	float fValue; //0x002C
	int32_t nValue; //0x0030
	int32_t bHasMin; //0x0034
	float fMinVal; //0x0038
	int32_t bHasMax; //0x003C
	float fMaxVal; //0x0040
	void *fnChangeCallback; //0x0044
};

typedef void* (*CreateInterfaceFn)(const char *pName, int *pReturnCode);
typedef void* (*InstantiateInterfaceFn)();

class IAppSystem {
public:
	virtual bool                            Connect(CreateInterfaceFn factory) = 0;                                     // 0
	virtual void                            Disconnect() = 0;                                                           // 1
	virtual void*                           QueryInterface(const char *pInterfaceName) = 0;                             // 2
	virtual int /*InitReturnVal_t*/         Init() = 0;                                                                 // 3
	virtual void                            Shutdown() = 0;                                                             // 4
	virtual const void* /*AppSystemInfo_t*/ GetDependencies() = 0;                                                      // 5
	virtual int /*AppSystemTier_t*/         GetTier() = 0;                                                              // 6
	virtual void                            Reconnect(CreateInterfaceFn factory, const char *pInterfaceName) = 0;       // 7
	virtual void                            UnkFunc() = 0;                                                              // 8
};

class IConsoleDisplayFunc
{
public:
	virtual void ColorPrint(const uint8_t* clr, const char *pMessage) = 0;
	virtual void Print(const char *pMessage) = 0;
	virtual void DPrint(const char *pMessage) = 0;
};


#define PRINTF_FORMAT_STRING _Printf_format_string_
class ICVar : public IAppSystem
{
public:
	virtual int        AllocateDLLIdentifier() = 0; // 9
	virtual void                       RegisterConCommand(ConCommandBase *pCommandBase) = 0; //10
	virtual void                       UnregisterConCommand(ConCommandBase *pCommandBase) = 0;
	virtual void                       UnregisterConCommands(int id) = 0;
	virtual const char*                GetCommandLineValue(const char *pVariableName) = 0;
	virtual ConCommandBase*            FindCommandBase(const char *name) = 0;
	virtual const ConCommandBase*      FindCommandBase(const char *name) const = 0;
	virtual ConVar*                    FindVar(const char *var_name) = 0; //16
	virtual const ConVar*              FindVar(const char *var_name) const = 0;
	virtual ConCommand*                FindCommand(const char *name) = 0;
	virtual const ConCommand*          FindCommand(const char *name) const = 0;
	virtual void                       InstallGlobalChangeCallback() = 0;//FnChangeCallback_t callback) = 0;
	virtual void                       RemoveGlobalChangeCallback() = 0; //FnChangeCallback_t callback) = 0;
	virtual void                       CallGlobalChangeCallbacks(ConVar *var, const char *pOldString, float flOldValue) = 0;
	virtual void                       InstallConsoleDisplayFunc(IConsoleDisplayFunc* pDisplayFunc) = 0;
	virtual void                       RemoveConsoleDisplayFunc(IConsoleDisplayFunc* pDisplayFunc) = 0;
	virtual void                       ConsoleColorPrintf(const Color &clr, const char *pFormat, ...) const = 0;
	virtual void                       ConsolePrintf(const char *pFormat, ...) const = 0;
	virtual void                       ConsoleDPrintf(const char *pFormat, ...) const = 0;
	virtual void                       RevertFlaggedConVars(int nFlag) = 0;
};

extern ICVar* g_pCVar;
