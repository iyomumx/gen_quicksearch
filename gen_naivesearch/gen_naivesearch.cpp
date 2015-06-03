// gen_naivesearch.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "SearchWindow.h"

int init();
void config();
void quit();

winampGeneralPurposePlugin plugin = { 0x10, "naive search plugin", init, config, quit, 0, 0 };

extern "C" __declspec(dllexport) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin() { return &plugin; }
extern "C" __declspec(dllexport) int winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param) { return 0x0; }

DWORD tid;
HANDLE hWindowThread = NULL;
SearchWindowStartUpParam * param;

#pragma region Winamp Main Window SubClassing

BOOL IsWinampUnicode = FALSE;
WNDPROC oldWndProc;
genHotkeysAddStruct hotkey("Open Naive Search", "Naive Search");
typedef LRESULT(WINAPI * CWP)(WNDPROC, HWND, UINT, WPARAM, LPARAM);
CWP cwp = 0;

LRESULT WINAPI WndProc
(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_WA_IPC && (lParam == IPC_PLAYLIST_MODIFIED || lParam == IPC_FILE_TAG_MAY_HAVE_UPDATEDW))
    {
        if (param->SearchPlugin)
            param->SearchPlugin->OnListRefresh();
    }
    else if (msg == hotkey.uMsg)
    {
        if (param->SearchPlugin)
            param->SearchPlugin->OnHotkeyPressed();
    }
    return cwp(oldWndProc, hwnd, msg, wParam, lParam);
}

void InitSubClassing(HWND wahwnd)
{
    IsWinampUnicode = IsWindowUnicode(wahwnd);
    cwp = IsWinampUnicode ? &CallWindowProcW : &CallWindowProcA;
    oldWndProc = reinterpret_cast<WNDPROC>(((IsWinampUnicode) ?
        SetWindowLongPtrW(wahwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)) :
        SetWindowLongPtrA(wahwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc))));
    hotkey.uMsg = static_cast<UINT>(
        SendMessage(wahwnd, WM_WA_IPC, reinterpret_cast<WPARAM>(hotkey.id), IPC_REGISTER_WINAMP_IPCMESSAGE));
    LRESULT genhotkeys_add_ipc =
        SendMessage(wahwnd, WM_WA_IPC, reinterpret_cast<WPARAM>(&"GenHotkeysAdd"), IPC_REGISTER_WINAMP_IPCMESSAGE);
    PostMessage(wahwnd, WM_WA_IPC, reinterpret_cast<WPARAM>(&hotkey), genhotkeys_add_ipc);
}

#pragma endregion Winamp Main Window SubClassing

#pragma region Plugin Standard Methods

int init()
{
    HANDLE heap = GetProcessHeap();
    if (heap == NULL)
    {
        return 1;
    }
    param = reinterpret_cast<SearchWindowStartUpParam*>(
        HeapAlloc(heap, HEAP_ZERO_MEMORY, sizeof(SearchWindowStartUpParam)));
    if (param == NULL)
    {
        return 1;
    }
    param->Plugin = &plugin;
    param->ProcessHeap = heap;
    InitSubClassing(plugin.hwndParent);
    hWindowThread = CreateThread(NULL, 0, &SearchWindowStart, param, 0, &tid);
    return 0;
}

void config()
{
    param->SearchPlugin->OnConfig();
}

void quit()
{
    param->SearchPlugin->OnExit();
    if (hWindowThread != NULL)
    {
        CloseHandle(hWindowThread);
    }
    if (param != NULL)
    {
        HeapFree(param->ProcessHeap, 0, param);
    }
}

#pragma endregion Plugin Standard Methods