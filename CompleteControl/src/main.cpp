#pragma region Includes
#include "obse/PluginAPI.h"
#include "obse/CommandTable.h"
#include "obse_common/SafeWrite.cpp"
#if OBLIVION
#include "obse/GameAPI.h"
OBSECommandTableInterface * g_commandTableIntfc = NULL; // assigned in OBSEPlugin_Load
OBSEScriptInterface * g_scriptIntfc = NULL; //For command argument extraction
#define ExtractArgsEx(...) g_scriptIntfc->ExtractArgsEx(__VA_ARGS__)
#define ExtractFormatStringArgs(...) g_scriptIntfc->ExtractFormatStringArgs(__VA_ARGS__)
#else
#include "obse_editor/EditorAPI.h"
#endif
#include "obse/ParamInfos.h"
#include <vector>
#include <set>
#include "Control.h"
#include <string>
#include "TM_CommonCPP/Misc.h"
#include "TM_CommonCPP/Narrate.h"
#include "TM_CommonCPP/StdStringFromFormatString.h"
#include "TM_CommonCPP_NarrateOverloads.h"
#include "obse/Script.h"
#include "obse/Hooks_DirectInput8Create.h"
#include <sstream>
#include "DebugCC.h"
#include "Settings.h"
#include "Globals.h"
#include "Misc.h"
#include "EventHandlers.h"
#pragma endregion
#pragma region CopyPasted
// Copy-pasted from obse's Control_Input
#define CONTROLSMAPPED 29
//Roundabout way of getting a pointer to the array containing control map
//Not sure what CtrlMapBaseAddr points to (no RTTI) so use brute pointer arithmetic
#if OBLIVION_VERSION == OBLIVION_VERSION_1_1
static const UInt32* CtrlMapBaseAddr = (UInt32*)0x00AEAAB8;
static const UInt32	 CtrlMapOffset = 0x00000020;
static const UInt32  CtrlMapOffset2 = 0x00001B7E;
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2
static const UInt32* CtrlMapBaseAddr = (UInt32*)0x00B33398;
static const UInt32	 CtrlMapOffset = 0x00000020;
static const UInt32  CtrlMapOffset2 = 0x00001B7E;
#elif OBLIVION_VERSION == OBLIVION_VERSION_1_2_416
static const UInt32* CtrlMapBaseAddr = (UInt32*)0x00B33398;
static const UInt32	 CtrlMapOffset = 0x00000020;
static const UInt32  CtrlMapOffset2 = 0x00001B7E;
#endif
UInt8*  InputControls = 0;
UInt8*  AltInputControls = 0;
static void GetControlMap()
{
	UInt32 addr = *CtrlMapBaseAddr + CtrlMapOffset;
	addr = *(UInt32*)addr + CtrlMapOffset2;
	InputControls = (UInt8*)addr;
	AltInputControls = InputControls + CONTROLSMAPPED;
}
static bool IsKeycodeValid(UInt32 id) { return id < kMaxMacros - 2; }
#pragma endregion
#pragma region CompleteControlAPI
//### CommandTemplate
bool Cmd_CommandTemplate_Execute(COMMAND_ARGS)
{
	DebugCC(5,"CommandTemplate`Open");
	DebugCC(5,"CommandTemplate`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(CommandTemplate, "CommandTemplate command", 0, 0, NULL)
//### HandleOblivionLoaded
bool Cmd_HandleOblivionLoaded_Execute(COMMAND_ARGS)
{
	DebugCC(5,"HandleOblivionLoaded`Open");
	bOblivionLoaded = true;
	//Controls = InitializeControls();
	DebugCC(5,"HandleOblivionLoaded`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(HandleOblivionLoaded, "HandleOblivionLoaded command", 0, 0, NULL)
//### DisableKey_Replacing
bool Cmd_DisableKey_Replacing_Execute(ParamInfo * paramInfo, void * arg1, TESObjectREFR * thisObj, UInt32 arg3, Script * scriptObj, ScriptEventList * eventList, double * result, UInt32 * opcodeOffsetPtr)
{
	DebugCC(5,"Cmd_DisableKey_Replacing_Execute`Open");
	UInt32	dxScancode = 0;
	UInt8	iModIndex = 0;
	//---Register in Controls
	//-Get dxScancode
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &dxScancode)) {
		DebugCC(5,"Cmd_DisableKey_Replacing_Execute`Failed arg extraction");
		return false;
	}
	//-Get iModIndex
	iModIndex = (UInt8)(scriptObj->refID >> 24);
	//-Register iModIndex in vControl.cModIndices
	for (Control &vControl : Controls)
	{
		if (vControl.dxScancode == dxScancode)
		{
			vControl.cModIndices_Disables.insert(iModIndex);
			break;
		}
	}
	//---DisableKey
	//-Execute Original DisableKey
	DisableKey_OriginalExecute(PASS_COMMAND_ARGS);
	DebugCC(5,"Cmd_DisableKey_Replacing_Execute`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(DisableKey_Replacing, "Disables a key and registers a disable", 0, 1, kParams_OneInt)
//### EnableKey_Replacing
bool Cmd_EnableKey_Replacing_Execute(ParamInfo * paramInfo, void * arg1, TESObjectREFR * thisObj, UInt32 arg3, Script * scriptObj, ScriptEventList * eventList, double * result, UInt32 * opcodeOffsetPtr)
{
	DebugCC(5,"Cmd_EnableKey_Replacing_Execute`Open");
	UInt32	dxScancode = 0;
	UInt8	iModIndex = 0;
	//---Register in Controls
	//-Get dxScancode
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &dxScancode)) {
		DebugCC(5,"Cmd_EnableKey_Replacing_Execute`Failed arg extraction");
		return false;
	}
	//-Get iModIndex
	iModIndex = (UInt8)(scriptObj->refID >> 24);
	//-Unregister disable. Determine bDoEnableKey by checking if any disables are registered for our dxScancode
	bool bDoEnableKey = true;
	for (Control &vControl : Controls)
	{
		if (vControl.dxScancode == dxScancode)
		{
			vControl.cModIndices_Disables.erase(iModIndex);
			if (!(vControl.cModIndices_Disables.empty()))
			{
				bDoEnableKey = false;
			}
			break;
		}
	}
	//---Execute original EnableKey
	if (bDoEnableKey) {
		DebugCC(5, "Cmd_EnableKey_Replacing_Execute`EnablingKey");
		EnableKey_OriginalExecute(PASS_COMMAND_ARGS);
	}
	else
	{
		DebugCC(5, "Cmd_EnableKey_Replacing_Execute`Neglecting to enable key.");
	}
	DebugCC(5,"Cmd_EnableKey_Replacing_Execute`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(EnableKey_Replacing, "Unregisters disable of this mod. Enables key if there are no disables registered.", 0, 1, kParams_OneInt)
#pragma endregion
#pragma region Tests
//### TestControlsFromString
bool Cmd_TestControlsFromString_Execute(COMMAND_ARGS)
{
	DebugCC(4, "TestControlsFromString`Open");
	DebugCC(4, "Controls:" + TMC::Narrate(Controls));
	std::string sControls = StringizeControls(Controls);
	DebugCC(4, "sControls:" + sControls);
	std::vector<Control> cReturningControls = ControlsFromString(sControls);
	DebugCC(4, "cReturningControls:" + TMC::Narrate(cReturningControls));
	DebugCC(4, "TestControlsFromString`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(TestControlsFromString, "TestControlsFromString command", 0, 0, NULL)
//### TestControlToString
bool Cmd_TestControlToString_Execute(COMMAND_ARGS)
{
	DebugCC(4, "TestControlToString`Open");
	std::string sControl = Controls[0].ToString();
	DebugCC(4, "sControl:" + sControl);
	std::vector<std::string> cStrings = TMC::SplitString(sControl, ",");
	DebugCC(4, "cStrings:" + TMC::Narrate(cStrings));
	Control vControl = Control(sControl);
	DebugCC(4, "vControl:" + vControl.Narrate());
	DebugCC(4, "TestControlToString`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(TestControlToString, "TestControlToString command", 0, 0, NULL)
//### PrintControls
bool Cmd_PrintControls_Execute(COMMAND_ARGS)
{
	DebugCC(5, "PrintControls`Controls:" + TMC::Narrate(Controls));
	return true;
}
DEFINE_COMMAND_PLUGIN(PrintControls, "PrintControls command", 0, 0, NULL)
//### TestCeil
bool Cmd_TestCeil_Execute(ParamInfo * paramInfo, void * arg1, TESObjectREFR * thisObj, UInt32 arg3, Script * scriptObj, ScriptEventList * eventList, double * result, UInt32 * opcodeOffsetPtr)
{
	DebugCC(5, "TestCeil`Open");
	*result = 0;
	UInt8* fArgs = new UInt8[3 + sizeof(double)];
	UInt16* fArgsNumArgs = (UInt16*)fArgs;
	*fArgsNumArgs = 1;
	fArgs[2] = 0x7A; // argument type double
	double* fArgsVal = (double*)&fArgs[3];
	*fArgsVal = 18.42;
	UInt32 opOffsetPtr = 0;
	const CommandInfo* ceil = g_commandTableIntfc->GetByName("Ceil");
	ceil->execute(kParams_OneFloat, fArgs, thisObj, arg3, scriptObj, eventList, result, &opOffsetPtr);
	delete[] fArgs;
	DebugCC(5, "TestCeil`opcode:" + TMC::Narrate(ceil->opcode) + " *result:" + TMC::Narrate(*result) + " result:" + TMC::Narrate(result));
	DebugCC(5, "TestCeil`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(TestCeil, "TestCeil command", 0, 0, NULL)
//### BasicRuntimeTests
bool Cmd_BasicRuntimeTests_Execute(COMMAND_ARGS)
{
	DebugCC(5, "BasicRuntimeTests`Open");
	//*result = 0; //Do I need this?
	int iInt = 5;
	DebugCC(5, "5:" + TMC::Narrate(iInt));
	UInt8 vUInt8 = 3;
	DebugCC(5, "3:" + TMC::Narrate(vUInt8));
	std::set<UInt8> cSet;
	cSet.insert(65);
	cSet.insert(64);
	cSet.insert(63);
	DebugCC(5, "Set:" + TMC::Narrate(cSet));
	DebugCC(5, "ActualControls:" + TMC::Narrate(Controls));
	//static std::vector<Control> Controls_Fake;
	//Controls_Fake.push_back(Control(15,UInt32(4)));
	//for (Control &vControl : Controls_Fake)
	//{
	//	vControl.cModIndices_Disables.insert(222);
	//}
	//DebugCC(5,"FakeControls:" + TMC::Narrate(Controls_Fake));
	DebugCC(5, "BasicRuntimeTests`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(BasicRuntimeTests, "BasicRuntimeTests command", 0, 0, NULL)
//### TestGetControlDirectly
bool Cmd_TestGetControlDirectly_Execute(ParamInfo * paramInfo, void * arg1, TESObjectREFR * thisObj, UInt32 arg3, Script * scriptObj, ScriptEventList * eventList, double * result, UInt32 * opcodeOffsetPtr)
{
	DebugCC(5, "TestGetControlDirectly`Open");
	double endResult;
	endResult = ExecuteCommand(GetControl, 2, PASS_COMMAND_ARGS);
	// Report
	//DebugCC(5,"TestGetControlDirectly`opcode:" + TMC::Narrate(GetControl->opcode) + " *result:" + TMC::Narrate(*result) + " result:" + TMC::Narrate(result));
	DebugCC(5, "TestGetControlDirectly`endResult:" + TMC::Narrate(endResult));
	DebugCC(5, "TestGetControlDirectly`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(TestGetControlDirectly, "TestGetControlDirectly command", 0, 0, NULL)
//### TestGetControlDirectly2
bool Cmd_TestGetControlDirectly2_Execute(ParamInfo * paramInfo, void * arg1, TESObjectREFR * thisObj, UInt32 arg3, Script * scriptObj, ScriptEventList * eventList, double * result, UInt32 * opcodeOffsetPtr)
{
	DebugCC(5, "TestGetControlDirectly2`Open");
	double endResult;
	endResult = ExecuteCommand(GetControl, 2);
	// Report
	//DebugCC(5,"TestGetControlDirectly2`opcode:" + TMC::Narrate(GetControl->opcode) + " *result:" + TMC::Narrate(*result) + " result:" + TMC::Narrate(result));
	DebugCC(5, "TestGetControlDirectly2`endResult:" + TMC::Narrate(endResult));
	DebugCC(5, "TestGetControlDirectly2`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(TestGetControlDirectly2, "TestGetControlDirectly2 command", 0, 0, NULL)
//### TestGetControlCopyPasta
bool Cmd_TestGetControlCopyPasta_Execute(COMMAND_ARGS)
{
	DebugCC(5, "TestGetControlCopyPasta`Open");
	*result = 0xFFFF;
	UInt32	keycode = 0;
	//ExtractArgs
	if (!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &keycode)) return true;
	//
	if (!InputControls) GetControlMap();
	*result = InputControls[keycode];
	DebugCC(5, "TestGetControlCopyPasta`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(TestGetControlCopyPasta, "TestGetControlCopyPasta command", 0, 1, kParams_OneInt)
//### TestDisableKeyCopyPasta
bool Cmd_TestDisableKeyCopyPasta_Execute(COMMAND_ARGS)
{
	//Open
	*result = 0;
	UInt32	keycode = 0;
	DebugCC(5, "TestDisableKeyCopyPasta`Open");
	//
	if (!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &keycode)) return true;
	if (keycode % 256 == 255 && keycode < 2048) keycode = 255 + (keycode + 1) / 256;
	if (IsKeycodeValid(keycode)) DI_data.DisallowStates[keycode] = 0x00;
	DebugCC(5, "TestDisableKeyCopyPasta`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(TestDisableKeyCopyPasta, "TestDisableKeyCopyPasta command", 0, 0, NULL)
//### GenerateEnum
bool Cmd_GenerateEnum_Execute(COMMAND_ARGS)
{
	DebugCC(5, "GenerateEnum`Open");
	//Extract Args
	TESForm* rTemp = NULL;
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &rTemp)) {
		DebugCC(5, "Cmd_DisableKey_Replacing_Execute`Failed arg extraction");
		return false;
	}
	//Report
	DebugCC(5, "rTemp:" + TMC::Narrate(rTemp->refID));
	//Return
	*result = rTemp->refID;
	DebugCC(5, "GenerateEnum`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(GenerateEnum, "GenerateEnum command", 0, 1, kParams_OneObjectRef)
#pragma endregion
#pragma region LoadEvent
extern "C" {
bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info)
{
	DebugCC(5,"Query`Open");
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "CompleteControl";
	info->version = 1;
	// version checks
	if(!obse->isEditor)
	{
#if OBLIVION
		if(obse->oblivionVersion != OBLIVION_VERSION)
		{
			_ERROR("incorrect Oblivion version (got %08X need %08X)", obse->oblivionVersion, OBLIVION_VERSION);
			return false;
		}
#endif	
	}

	if (obse->obseVersion < OBSE_VERSION_INTEGER)
	{
		_MESSAGE("OBSE version too old (got %08X expected at least %08X)", obse->obseVersion, OBSE_VERSION_INTEGER);
		_MESSAGE("OBSE version too old (got %i expected at least %i)", obse->obseVersion, OBSE_VERSION_INTEGER);
		//return false;
	}

	DebugCC(5, "Query`Close");
	return true;
}
bool OBSEPlugin_Load(const OBSEInterface * obse)
{
	DebugCC(4, "Load`Open");
	g_pluginHandle = obse->GetPluginHandle();
	if (!obse->isEditor)
	{
		g_serialization = (OBSESerializationInterface *)obse->QueryInterface(kInterface_Serialization);
		if (!g_serialization)
		{
			_ERROR("serialization interface not found");
			return false;
		}
		if (g_serialization->version < OBSESerializationInterface::kVersion)
		{
			_ERROR("incorrect serialization version found (got %08X need %08X)", g_serialization->version, OBSESerializationInterface::kVersion);
			return false;
		}
		g_serialization->SetSaveCallback(g_pluginHandle, Handler_Save);
		g_serialization->SetLoadCallback(g_pluginHandle, Handler_Load);
		g_serialization->SetNewGameCallback(g_pluginHandle, Handler_NewGame);
	}
	obse->SetOpcodeBase(0x28B0);
	obse->RegisterCommand(&kCommandInfo_BasicRuntimeTests);
	obse->RegisterCommand(&kCommandInfo_TestGetControlDirectly);
	obse->RegisterCommand(&kCommandInfo_TestGetControlDirectly2);
	obse->RegisterCommand(&kCommandInfo_GenerateEnum);
	obse->RegisterCommand(&kCommandInfo_CommandTemplate);
	obse->RegisterCommand(&kCommandInfo_TestGetControlCopyPasta);
	obse->RegisterCommand(&kCommandInfo_TestDisableKeyCopyPasta);
	obse->RegisterCommand(&kCommandInfo_TestCeil);
	obse->RegisterCommand(&kCommandInfo_HandleOblivionLoaded);
	obse->RegisterCommand(&kCommandInfo_PrintControls);
	obse->RegisterCommand(&kCommandInfo_TestControlToString);
	obse->RegisterCommand(&kCommandInfo_TestControlsFromString);

	if (!obse->isEditor)
	{
		// get an OBSEScriptInterface to use for argument extraction
		g_scriptIntfc = (OBSEScriptInterface*)obse->QueryInterface(kInterface_Script);
		g_commandTableIntfc = (OBSECommandTableInterface*)obse->QueryInterface(kInterface_CommandTable);
		// replace DisableKey
		DisableKey_OriginalExecute = g_commandTableIntfc->GetByOpcode(0x1430)->execute; //DisableKey_opcode:00001430
		g_commandTableIntfc->Replace(0x1430, &kCommandInfo_DisableKey_Replacing);
		// replace EnableKey
		EnableKey_OriginalExecute = g_commandTableIntfc->GetByOpcode(0x1431)->execute; //EnableKey_opcode:00001431
		g_commandTableIntfc->Replace(0x1431, &kCommandInfo_EnableKey_Replacing);
		// Get GetControl
		GetControl = g_commandTableIntfc->GetByName("GetControl");
		//GetControl->
		//
		DisableKey_CmdInfo = g_commandTableIntfc->GetByOpcode(0x1430);
		//DisableKey_CmdInfo->execute();
	}

	DebugCC(4, "Load`Close");
	return true;
}
};
#pragma endregion
