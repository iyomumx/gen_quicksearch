#include "stdafx.h"

using namespace System;

#pragma unmanaged

extern "C" __declspec(dllexport) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin() { return &plugin; }
extern "C" __declspec(dllexport) int winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param) { return 0x0; }

#pragma managed

void mInit()
{
	auto t = gcnew System::Threading::Thread(gcnew System::Threading::ThreadStart(&PluginWindow::Init));
	t->ApartmentState = System::Threading::ApartmentState::STA;
	t->Start();
}

#pragma unmanaged

//非托管代码较多而托管代码只涉及新线程，故将此函数编译为非托管代码
int init()
{
	//在新线程中进行WPF初始化
	mInit();
	//处理Windows消息
	fUnicode = IsWindowUnicode(plugin.hwndParent);
	oldWndProc = (WNDPROC)((fUnicode) ?
		SetWindowLongPtrW(plugin.hwndParent, GWLP_WNDPROC, (LONG_PTR)WndProc) :
		SetWindowLongPtrA(plugin.hwndParent, GWLP_WNDPROC, (LONG_PTR)WndProc));
	//Hotkey注册
	hotkey = { 0 };
	LRESULT genhotkeys_add_ipc = 0;
	hotkey.wnd = 0;
	hotkey.flags = 0;
	hotkey.name = "常规：打开快速搜索";
	hotkey.id = PLUGIN_SHORT_NAME;
	hotkey.uMsg = (UINT)SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)hotkey.id, IPC_REGISTER_WINAMP_IPCMESSAGE);
	genhotkeys_add_ipc = SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&"GenHotkeysAdd", IPC_REGISTER_WINAMP_IPCMESSAGE);
	PostMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&hotkey, genhotkeys_add_ipc);
	//For more info, google "wa_hotkeys.h" :)
	//API Service
	WASABI_API_SVC = (api_service*)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	return 0;
}

#pragma managed

void quit()
{
	PluginWindow::MainWindow->IsClosing = true;
	PluginWindow::MainWindow->Invoke(gcnew Action(PluginWindow::MainWindow, &PluginWindow::Close));
}

inline void mShowAndFocus()
{
	PluginWindow::MainWindow->AsyncInvoke(PluginWindow::MainWindow->SAFcallback);
}

void ShowConfig()
{
	PluginWindow::MainWindow->Invoke(gcnew Action(PluginWindow::MainWindow, &PluginWindow::ShowSetting));
}

void mRefreshList()
{
	PluginWindow::MainWindow->RLcallback->BeginInvoke(nullptr, nullptr);
}

#pragma unmanaged

void config()
{
	//::MessageBox(plugin.hwndParent, _T("目前还没有可设置的选项"), _T("施工中"), MB_OK);
	ShowConfig();
}

//鉴于WndProc在非托管部分运行，而刷新列表和显示窗口相对于其他Windows消息出现机会更少
//故将此函数以非托管代码部分编译，保证程序效率，避免对CallWindowProc的C++ Interop
LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == hotkey.uMsg)
	{
		//Hotkey is Pressed
		mShowAndFocus();
	}
	else if (msg == WM_WA_IPC)
	{
		if (lParam == IPC_PLAYLIST_MODIFIED)
		{
			mRefreshList();
		}
	}
	return (fUnicode) ? CallWindowProcW(oldWndProc, hwnd, msg, wParam, lParam) : CallWindowProcA(oldWndProc, hwnd, msg, wParam, lParam);
}