#pragma once
#include "Color.h"
#include "Utils.h"
#define _CRT_SECURE_NO_WARNINGS
class ConVar
{
public:
	Color GetColor()
	{
		return Utils::CallVFunc<10 ,Color>(this);
	}

	const char* GetString()
	{
		return Utils::CallVFunc<11 , const char*>(this);
	}

	float GetFloat()
	{
		return Utils::CallVFunc<12 , float>(this);
	}

	int GetInt()
	{
		return Utils::CallVFunc<13 , int>(this);
	}

	void SetValue(const char *value)
	{
		Utils::CallVFunc<14 ,void>(this, value);
	}

	void SetValue(float value)
	{
		Utils::CallVFunc<15 , void>(this, value);
	}

	void SetValue(int value)
	{
		Utils::CallVFunc<16 ,void>(this, value);
	}

	void SetValue(Color value)
	{
		Utils::CallVFunc<17 ,void>(this, value);
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
#define PRINTF_FORMAT_STRING _Printf_format_string_
class ICVar {
public:
	virtual void Func0();
	virtual void Func1();
	virtual void Func2();
	virtual void Func3();
	virtual void Func4();
	virtual void Func5();
	virtual void Func6();
	virtual void Func7();
	virtual void Func8();
	virtual void Func9();
	virtual void RegisterConCommand(ConVar *pCommandBase) = 0;
	virtual void UnregisterConCommand(ConVar *pCommandBase) = 0;
	virtual void Func12();
	virtual void Func13();
	virtual void Func14();
	virtual void Func15();
	virtual ConVar* FindVar(const char* getVar);
	virtual void Func17();
	virtual void Func18();
	virtual void Func19();
	virtual void Func20();
	virtual void Func21();
	virtual void Func22();
	virtual void Func23();
	virtual void Func24();
	virtual void ConsoleColorPrintf(const Color& clr, const char *format, ...);
	virtual void ConsolePrintf(const char *format, ...);
	virtual void ConsoleDPrintf(PRINTF_FORMAT_STRING const char *pFormat, ...);
	virtual void Func28();
	virtual void Func29();
	virtual void Func30();
	virtual void Func31();
	virtual void Func32();
	virtual void Func33();
	virtual void Func34();
	virtual void Func35();
	virtual void Func36();
	virtual void Func37();
};

extern ICVar* g_pCVar;

