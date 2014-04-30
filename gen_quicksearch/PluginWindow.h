#pragma once
#include <Windows.h>
#include <Winamp\wa_ipc.h>
#include <gen_tray\WINAMPCMD.H>

using namespace System;
using namespace System::Windows;

ref class PluginWindow :
public Window
{
public:
	PluginWindow();
	static PluginWindow^ MainWindow;
	static void Init(HWND);
private:
	HWND MainHandle;
	String ^ GetPlayListFile(int index)
	{
		return gcnew String((wchar_t*)SendMessage(MainHandle, WM_WA_IPC, index, IPC_GETPLAYLISTFILEW));
	};
	String ^ GetPlayListTitle(int index)
	{
		return gcnew String((wchar_t*)SendMessage(MainHandle, WM_WA_IPC, index, IPC_GETPLAYLISTTITLEW));
	};
	int GetListLength()
	{
		return SendMessage(MainHandle, WM_WA_IPC, 0, IPC_GETLISTLENGTH);
	};
	void PlayIndex(int index)
	{
		SendMessage(MainHandle, WM_WA_IPC, index, IPC_SETPLAYLISTPOS);
		SendMessage(MainHandle, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON2, 0), 0);
	};
	void InitializeComponent();
};

