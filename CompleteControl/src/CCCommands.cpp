#include "CCCommands.h"

//### CommandTemplate
bool Cmd_CommandTemplate_Execute(COMMAND_ARGS)
{
	DebugCC(5, std::string(__func__) + "`Open");
	DebugCC(5, std::string(__func__) + "`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(CommandTemplate, "CommandTemplate command", 0, 0, NULL)
//### DisableKey_Replacing
bool Cmd_DisableKey_Replacing_Execute(ParamInfo * paramInfo, void * arg1, TESObjectREFR * thisObj, UInt32 arg3, Script * scriptObj, ScriptEventList * eventList, double * result, UInt32 * opcodeOffsetPtr)
{
	DebugCC(5, std::string(__func__) + "`Open");
	UInt32	dxScancode = 0;
	UInt8	iModIndex = 0;
	//---Register in Controls
	//-Get dxScancode
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &dxScancode)) { DebugCC(5, std::string(__func__) + "`Failed arg extraction"); return false; }
	//-Get iModIndex
	iModIndex = (UInt8)(scriptObj->refID >> 24);
	//-Register iModIndex in vControl.cModIndices. SetOutcome.
	auto vControl = GetControlByScancode(dxScancode);
	vControl->cModIndices_Disables.insert(iModIndex);
	vControl->SetOutcome();
	DebugCC(5, std::string(__func__) + "`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(DisableKey_Replacing, "Disables a key and registers a disable", 0, 1, kParams_OneInt)
//### EnableKey_Replacing
bool Cmd_EnableKey_Replacing_Execute(ParamInfo * paramInfo, void * arg1, TESObjectREFR * thisObj, UInt32 arg3, Script * scriptObj, ScriptEventList * eventList, double * result, UInt32 * opcodeOffsetPtr)
{
	DebugCC(5, std::string(__func__) + "`Open");
	UInt32	dxScancode = 0;
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &dxScancode)) { DebugCC(5, std::string(__func__) + "`Failed arg extraction"); return false; }
	auto pControl = GetControlByScancode(dxScancode);
	//-Unregister disable. SetOutcome.
	pControl->cModIndices_Disables.erase((UInt8)(scriptObj->refID >> 24)); //iModIndex:(UInt8)(scriptObj->refID >> 24)
	pControl->SetOutcome();
	DebugCC(5, std::string(__func__) + "`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(EnableKey_Replacing, "Unregisters disable of this mod. Enables key if there are no disables registered.", 0, 1, kParams_OneInt)
//### DisableControl_Replacing
bool Cmd_DisableControl_Replacing_Execute(ParamInfo * paramInfo, void * arg1, TESObjectREFR * thisObj, UInt32 arg3, Script * scriptObj, ScriptEventList * eventList, double * result, UInt32 * opcodeOffsetPtr)
{
	DebugCC(5, std::string(__func__) + "`Open");
	UInt32	vControlID = 0;
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &vControlID)) { DebugCC(5, std::string(__func__) + "`Failed arg extraction"); return false; }
	auto pControl = GetControlByID(vControlID);
	//-Register iModIndex in pControl.cModIndices. SetOutcome.
	pControl->cModIndices_Disables.insert((UInt8)(scriptObj->refID >> 24)); //iModIndex:(UInt8)(scriptObj->refID >> 24)
	pControl->SetOutcome();
	DebugCC(5, std::string(__func__) + "`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(DisableControl_Replacing, "Disables a key and registers a disable", 0, 1, kParams_OneInt)
//### EnableControl_Replacing
bool Cmd_EnableControl_Replacing_Execute(ParamInfo * paramInfo, void * arg1, TESObjectREFR * thisObj, UInt32 arg3, Script * scriptObj, ScriptEventList * eventList, double * result, UInt32 * opcodeOffsetPtr)
{
	DebugCC(5, std::string(__func__) + "`Open");
	UInt32	vControlID = 0;
	UInt8	iModIndex = 0;
	//---Register in Controls
	//-Get vControlID
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &vControlID)) { DebugCC(5, std::string(__func__) + "`Failed arg extraction"); return false; }
	//-Get iModIndex
	iModIndex = (UInt8)(scriptObj->refID >> 24);
	//-Unregister disable. SetOutcome.
	auto vControl = GetControlByID(vControlID);
	vControl->cModIndices_Disables.erase(iModIndex);
	vControl->SetOutcome();
	DebugCC(5, std::string(__func__) + "`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(EnableControl_Replacing, "Unregisters disable of this mod. Enables key if there are no disables registered.", 0, 1, kParams_OneInt)
//### RegisterControl
bool Cmd_RegisterControl_Execute(COMMAND_ARGS)
{
	DebugCC(5, std::string(__func__) + "`Open");
	TESForm* vControlID_Form = 0;
	int dxScancode = 0;
	int iMenuModeType = -1;
	auto eMenuModeType = Control::MenuModeType(0);
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &vControlID_Form, &dxScancode, &iMenuModeType)) { DebugCC(5, std::string(__func__) + "`Failed arg extraction"); return false; }
	//-Defaults for optional params
	if (!dxScancode) { dxScancode = 0xFF; } //0xFF = unassigned
	if (iMenuModeType != -1) { eMenuModeType = Control::MenuModeType(iMenuModeType); }
	//-
	Controls.push_back(Control(vControlID_Form->refID, eMenuModeType, dxScancode));
	DebugCC(4, std::string(__func__) + "`Controls:" + TMC::Narrate(Controls));
	DebugCC(5, std::string(__func__) + "`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(RegisterControl, "Registers a non-vanilla Control. Uses any ref as its ID.", 0, 3, kParams_RegisterControl)
//### IsDisabled
bool Cmd_IsDisabled_Execute(COMMAND_ARGS)
{
	DebugCC(5, std::string(__func__) + "`Open");
	UInt32	vControlID = 0;
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &vControlID)) { DebugCC(5, std::string(__func__) + "`Failed arg extraction"); return false; }
	*result = GetControlByID(vControlID)->IsDisabled();
	DebugCC(5, std::string(__func__) + "`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(IsDisabled, "True if the key -should- be disabled.", 0, 1, kParams_OneInt)
//### GetKey
bool Cmd_GetKey_Execute(COMMAND_ARGS)
{
	DebugCC(5, std::string(__func__) + "`Open");
	UInt32	vControlID = 0;
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &vControlID)) { DebugCC(5, std::string(__func__) + "`Failed arg extraction"); return false; }
	*result = GetControlByID(vControlID)->GetDXScancode();
	DebugCC(5, std::string(__func__) + "`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(GetKey, "GetKey command", 0, 1, kParams_OneInt)
//### UnreportedDisable
bool Cmd_UnreportedDisable_Execute(ParamInfo * paramInfo, void * arg1, TESObjectREFR * thisObj, UInt32 arg3, Script * scriptObj, ScriptEventList * eventList, double * result, UInt32 * opcodeOffsetPtr)
{
	DebugCC(5, std::string(__func__) + "`Open");
	UInt32	vControlID = 0;
	UInt8	iModIndex = 0;
	//---Register in Controls
	//-Get vControlID
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &vControlID)) { DebugCC(5, std::string(__func__) + "`Failed arg extraction"); return false; }
	//-Get iModIndex
	iModIndex = (UInt8)(scriptObj->refID >> 24);
	//-Register iModIndex in vControl.cModIndices. SetOutcome.
	auto pControl = GetControlByID(vControlID);
	pControl->cModIndices_UnreportedDisables.insert(iModIndex);
	pControl->SetOutcome();
	DebugCC(5, std::string(__func__) + "`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(UnreportedDisable, "Disables a key and registers a disable", 0, 1, kParams_OneInt)
//### UnreportedEnable
bool Cmd_UnreportedEnable_Execute(ParamInfo * paramInfo, void * arg1, TESObjectREFR * thisObj, UInt32 arg3, Script * scriptObj, ScriptEventList * eventList, double * result, UInt32 * opcodeOffsetPtr)
{
	DebugCC(5, std::string(__func__) + "`Open");
	UInt32	vControlID = 0;
	UInt8	iModIndex = 0;
	//---Register in Controls
	//-Get dxScancode
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &vControlID)) { DebugCC(5, std::string(__func__) + "`Failed arg extraction"); return false; }
	//-Get iModIndex
	iModIndex = (UInt8)(scriptObj->refID >> 24);
	//-Unregister disable. SetOutcome.
	auto pControl = GetControlByID(vControlID);
	pControl->cModIndices_UnreportedDisables.erase(iModIndex);
	pControl->SetOutcome();
	DebugCC(5, std::string(__func__) + "`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(UnreportedEnable, "Unregisters disable of this mod. Enables key if there are no disables registered.", 0, 1, kParams_OneInt)
//### IsEngaged
bool Cmd_IsEngaged_Execute(COMMAND_ARGS)
{
	DebugCC(5, std::string(__func__) + "`Open");
	UInt32	vControlID = -1;
	Control* pControl;
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &vControlID)) { DebugCC(5, std::string(__func__) + "`Failed arg extraction"); return false; }
	if (!(pControl = GetControlByID(vControlID))) { DebugCC(5, std::string(__func__) + "`Received unregistered ControlID:" +TMC::Narrate(vControlID)); return true; }
	*result = pControl->IsEngaged();
	DebugCC(5, std::string(__func__) + "`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(IsEngaged, "Is the control pressed and not disabled?", 0, 1, kParams_OneControlID)
//### IsEngaged_ByRef
bool Cmd_IsEngaged_ByRef_Execute(COMMAND_ARGS)
{
	DebugCC(5, std::string(__func__) + "`Open");
	TESForm* vControlID_Form = 0;
	Control* pControl;
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &vControlID_Form)) { DebugCC(5, std::string(__func__) + "`Failed arg extraction"); return false; }
	if (!(pControl = GetControlByID(vControlID_Form->refID))) { DebugCC(5, std::string(__func__) + "`Received unregistered ControlID:" + TMC::Narrate(vControlID_Form->refID)); return true; }
	*result = pControl->IsEngaged();
	DebugCC(5, std::string(__func__) + "`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(IsEngaged_ByRef, "Is the control pressed and not disabled?", 0, 1, kParams_OneControlRef)
//### OnControlDown2
bool Cmd_OnControlDown2_Execute(COMMAND_ARGS)
{
	DebugCC(8, std::string(__func__) + "`Open");
	UInt32	vControlID = 0;
	Control* pControl;
	*result = 0;
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &vControlID)) { DebugCC(5, std::string(__func__) + "`Failed arg extraction"); return false; }
	if (!(pControl = GetControlByID(vControlID))) { DebugCC(5, std::string(__func__) + "`Received unregistered ControlID:" + TMC::Narrate(vControlID)); return true; }
	if (pControl->IsPressed())
	{
		if (!Contains(pControl->cScriptRefs_ReceivedOnControlDown, scriptObj->refID))
		{
			pControl->cScriptRefs_ReceivedOnControlDown.insert(scriptObj->refID);
			*result = 1;
		}
	}
	else if (Contains(pControl->cScriptRefs_ReceivedOnControlDown, scriptObj->refID))
	{
		pControl->cScriptRefs_ReceivedOnControlDown.erase(scriptObj->refID);
	}
	DebugCC(8, std::string(__func__) + "`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(OnControlDown2, "OnControlDown2 command", 0, 1, kParams_OneControlID)
//### OnControlDown2_ByRef
bool Cmd_OnControlDown2_ByRef_Execute(COMMAND_ARGS)
{
	DebugCC(7, std::string(__func__) + "`Open");
	TESForm* vControlID_Form = 0;
	Control* pControl;
	*result = 0;
	if (!ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &vControlID_Form)) { DebugCC(5, std::string(__func__) + "`Failed arg extraction"); return false; }
	if (!(pControl = GetControlByID(vControlID_Form->refID))) { DebugCC(5, std::string(__func__) + "`Received unregistered ControlID:" + TMC::Narrate(vControlID_Form->refID)); return true; }
	if (pControl->IsPressed())
	{
		if (!Contains(pControl->cScriptRefs_ReceivedOnControlDown, scriptObj->refID))
		{
			pControl->cScriptRefs_ReceivedOnControlDown.insert(scriptObj->refID);
			*result = 1;
		}
	}
	else if (Contains(pControl->cScriptRefs_ReceivedOnControlDown, scriptObj->refID))
	{
		pControl->cScriptRefs_ReceivedOnControlDown.erase(scriptObj->refID);
	}
	DebugCC(7, std::string(__func__) + "`Close");
	return true;
}
DEFINE_COMMAND_PLUGIN(OnControlDown2_ByRef, "OnControlDown2_ByRef command", 0, 1, kParams_OneControlRef)
