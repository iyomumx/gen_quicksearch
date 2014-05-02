#include "stdafx.h"

using namespace System;

extern "C" __declspec(dllexport) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin() { return &plugin; }
extern "C" __declspec(dllexport) int winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param) { return 0x0; }

int init()
{
	//�����߳��н���WPF��ʼ��
	auto t = gcnew System::Threading::Thread(gcnew System::Threading::ThreadStart(&PluginWindow::Init));
	t->ApartmentState = System::Threading::ApartmentState::STA;
	t->Start();
	//����Windows��Ϣ
	fUnicode = IsWindowUnicode(plugin.hwndParent);
	oldWndProc = (WNDPROC)((fUnicode) ?
		SetWindowLongPtrW(plugin.hwndParent, GWLP_WNDPROC, (LONG_PTR)WndProc) :
		SetWindowLongPtrA(plugin.hwndParent, GWLP_WNDPROC, (LONG_PTR)WndProc));
	//Hotkeyע��
	hotkey = { 0 };
	LRESULT genhotkeys_add_ipc = 0;
	hotkey.wnd = 0;
	hotkey.flags = 0;
	hotkey.name = "���棺�򿪿�������";
	hotkey.id = PLUGIN_SHORT_NAME;
	hotkey.uMsg = (UINT)SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)hotkey.id, IPC_REGISTER_WINAMP_IPCMESSAGE);
	genhotkeys_add_ipc = SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&"GenHotkeysAdd", IPC_REGISTER_WINAMP_IPCMESSAGE);
	PostMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&hotkey, genhotkeys_add_ipc);
	//For more info, google "wa_hotkeys.h" :)
	//API Service
	WASABI_API_SVC = (api_service*)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	return 0;
}

void quit()
{
	PluginWindow::MainWindow->IsClosing = true;
	PluginWindow::MainWindow->Invoke(gcnew Action(PluginWindow::MainWindow, &PluginWindow::Close));
}

void config()
{
	::MessageBox(plugin.hwndParent, _T("Ŀǰ��û�п����õ�ѡ��"), _T("ʩ����"), MB_OK);
}

LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == hotkey.uMsg)
	{
		//Hotkey is Pressed
		PluginWindow::MainWindow->AsyncInvoke(PluginWindow::MainWindow->SAFcallback);
	}
	else if (msg == WM_WA_IPC)
	{
		if (lParam == IPC_PLAYLIST_MODIFIED)
		{
			PluginWindow::MainWindow->AsyncInvoke(PluginWindow::MainWindow->RLcallback);
		}
	}
	return (fUnicode) ? CallWindowProcW(oldWndProc, hwnd, msg, wParam, lParam) : CallWindowProcA(oldWndProc, hwnd, msg, wParam, lParam);
}